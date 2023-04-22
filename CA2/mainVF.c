// standard headers
#include <stdio.h>
#include <stdlib.h>
// #include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>


// headers for the pci and DAQ 
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <math.h>

// headers for pthreads 
#include <pthread.h>
#include <process.h>
																
#define	INTERRUPT		iobase[1] + 0				// Badr1 + 0 : also ADC register
#define	MUXCHAN			iobase[1] + 2				// Badr1 + 2
#define	TRIGGER			iobase[1] + 4				// Badr1 + 4
#define	AUTOCAL			iobase[1] + 6				// Badr1 + 6
#define DA_CTLREG		iobase[1] + 8				// Badr1 + 8

#define	AD_DATA 		iobase[2] + 0				// Badr2 + 0
#define	AD_FIFOCLR		iobase[2] + 2				// Badr2 + 2

#define	TIMER0			iobase[3] + 0				// Badr3 + 0
#define	TIMER1			iobase[3] + 1				// Badr3 + 1
#define	TIMER2			iobase[3] + 2				// Badr3 + 2
#define	COUNTCTL		iobase[3] + 3				// Badr3 + 3
#define	DIO_PORTA		iobase[3] + 4				// Badr3 + 4
#define	DIO_PORTB		iobase[3] + 5				// Badr3 + 5
#define	DIO_PORTC		iobase[3] + 6				// Badr3 + 6
#define	DIO_CTLREG		iobase[3] + 7				// Badr3 + 7
#define	PACER1			iobase[3] + 8				// Badr3 + 8
#define	PACER2			iobase[3] + 9				// Badr3 + 9
#define	PACER3			iobase[3] + a				// Badr3 + a
#define	PACERCTL		iobase[3] + b				// Badr3 + b

#define DA_Data			iobase[4] + 0				// Badr4 + 0
#define	DA_FIFOCLR		iobase[4] + 2				// Badr4 + 2
	
int badr[5];															// PCI 2.2 assigns 6 IO base addresses


#define BILLION		1000000000L
#define MILLION		1000000L
#define THOUSAND	1000L


struct timespec start, temp;
double accum =0.0;
double period = 0.01;
int ii = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
// Declare functions

void cmdArg(int argc, char **argv, int *ptr_wavetype, float *ptr_amplitude, int *ptr_frequency);
void waveGeneration(int *ptr_wavetype, float *ptr_amplitude);
void signalHandler(int signum);
void *adcIn(void *arg);
void *event_loop(void *arg);
void startmsg();

void updateValues();

// Global Variables

int wavetype, frequency;
float amplitude; 
unsigned int data[100];
uint16_t adc_in;
uintptr_t iobase[6];
uintptr_t dio_in;
int mode = 0;
int old_mode=0;
pthread_t thKey, thdaq;

FILE *fp;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {
    struct pci_dev_info info;
    void *hdl;  

    unsigned int i;
    
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
    

	signal(SIGINT, signalHandler);

    memset(&info,0,sizeof(info));
    if(pci_attach(0)<0) {
    perror("pci_attach");
    exit(EXIT_FAILURE);
    }
                                                                            /* Vendor and Device ID */
    info.VendorId=0x1307;
    info.DeviceId=0x01;

    if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
    perror("pci_attach_device");
    exit(EXIT_FAILURE);
    }
                                                                            // Determine assigned BADRn IO addresses for PCI-DAS1602			

    //printw("\nDAS 1602 Base addresses:\n\n");
    for(i=0;i<5;i++) {
    badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
    //printw("Badr[%d] : %x\n", i, badr[i]);
    }
    
    //printw("\nReconfirm Iobase:\n");  						// map I/O base address to user space						
    for(i=0;i<5;i++) {												// expect CpuBaseAddress to be the same as iobase for PC
    iobase[i]=mmap_device_io(0x0f,badr[i]);	
   // printw("Index %d : Address : %x ", i,badr[i]);
   // printw("IOBASE  : %x \n",iobase[i]);
    }													
                                                                            // Modify thread control privity
    if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
    perror("Thread Control");
    exit(1);
    }																											

    //******************************************************************************
    // ADC Port Functions
    //******************************************************************************
    // Initialise Board								
    out16(INTERRUPT,0x60c0);				// sets interrupts	 - Clears			
    out16(TRIGGER,0x2081);					// sets trigger control: 10MHz, clear, Burst off,SW trig. default:20a0
    out16(AUTOCAL,0x007f);					// sets automatic calibration : default

    out16(AD_FIFOCLR,0); 					// clear ADC buffer
    out16(MUXCHAN,0x0D00);				    // Write to MUX register - SW trigger, UP, SE, 5v, ch 0-0 	
											// x x 0 0 | 1  0  0 1  | 0x 7   0 | Diff - 8 channels
											// SW trig |Diff-Uni 5v| scan 0-7| Single - 16 channels
											
	out8(DIO_CTLREG,0x90);		// Port A : Input,  Port B : Output,  Port C (upper | lower) : Output | Output
	if(in8(DIO_PORTA)!=255){
		printw("Please turn all toggle switches on to start the program"); 
		refresh();
		while(in8(DIO_PORTA)!=255){delay(1);}
	}

    // Reading and assigning cmd line argument 
    cmdArg(argc, argv, &wavetype, &amplitude, &frequency); 

    // Wave Generation based on cmd line arguments
    waveGeneration(&wavetype, &amplitude);

	startmsg();

    pthread_create(&thdaq,NULL, &adcIn, NULL);
    
    pthread_create(&thKey, NULL, &event_loop, NULL);
    

    while(1) {
    	
ii = 0;
    	accum = 0;
    
    	if(clock_gettime(CLOCK_REALTIME,&start)==-1)
		{
			perror("clock gettime");
			exit(EXIT_FAILURE);
  		}
  		
    	while(ii<50){
    		if(accum > period){
				out16(DA_CTLREG,0x0a23);			// DA Enable, #0, #1, SW 5V unipolar		2/6
        		out16(DA_FIFOCLR, 0);					// Clear DA FIFO  buffer
        		out16(DA_Data,(short) data[ii]);																																		
        		out16(DA_CTLREG,0x0a43);			// DA Enable, #1, #1, SW 5V unipolar		2/6
        		out16(DA_FIFOCLR, 0);					// Clear DA FIFO  buffer
        		out16(DA_Data,(short) data[ii]);	
				start = temp;
				ii++;
			}
		
			if(clock_gettime(CLOCK_REALTIME,&temp)==-1)
{
				perror("clock gettime");
				exit(EXIT_FAILURE);
 			}
   			accum = (double)(temp.tv_sec-start.tv_sec)+(double)(temp.tv_nsec- start.tv_nsec)/BILLION;
    	}
    	
    }
                                                            // Unreachable code
                                                            // Reset DAC to 5v
    out16(DA_CTLREG,(short)0x0a23);	
    out16(DA_FIFOCLR,(short) 0);			
    out16(DA_Data, 0x8fff);						// Mid range - Unipolar																											
    
    out16(DA_CTLREG,(short)0x0a43);	
    out16(DA_FIFOCLR,(short) 0);			
    out16(DA_Data, 0x8fff);				
                                                                                                                                                            
    printw("\n\nExit Demo Program\n");
    pci_detach_device(hdl);
    endwin();
 
    return(0);
}

// Function 1: Command line arguments //
void cmdArg(int argc, char **argv, int *ptr_wavetype, float *ptr_amplitude, int *ptr_frequency)           // *ptr_wavetype -> &wavetype
{   
    char arr[4][100] = {"Sine", "Square", "Sawtooth", "Triangular"}; // an array of different waveforms
    char **ptr_argv = &argv[1];     // start first argument after name of file.exe
    bool valid_wave = FALSE;
    unsigned int i;
    // Check if the arguments are passed

    if(argc < 2)  // if no argument was passed, read from waveConfig.txt
    {
        FILE *fp = fopen("waveConfig.txt", "r");
        if (fp == NULL) { 
        printf("Error opening file\n");
        }
        
        // Read the contents of the file
        fscanf(fp, "%d %f %d", &wavetype, &amplitude, &frequency);
        printw("Wave configuration was read from the waveConfig.txt");
        // Close the file
        fclose(fp);
    }   
    else 
    {
        // For Waveform //
        for (i = 0; i < 4; i++)
        {
            if (strcmpi(arr[i], *(ptr_argv)) == 0)     // return 0 if all chars are = (ignore cases)
            {
            // printw("Plotting %s Wave...\n", arr[i]);
            valid_wave = TRUE;
            *ptr_wavetype = i + 1;       // 1st argument = Type of waveform - 1: Sine, 2: Square, 3: Sawtooth, 4: Triangular (call by ref)
            }
        }
        // Amplitude //
        *ptr_amplitude = atoi(*(ptr_argv + 1));      // 2nd argument = Amplitude    
        // Frequency //
        *ptr_frequency = atoi(*(ptr_argv + 2));      // 3rd argument = Frequency
    }
}

void waveGeneration(int *ptr_wavetype, float *ptr_amplitude)
{
    double delta, dummy; 
    unsigned int i;
    double scale; 
    scale = ((*ptr_amplitude)/2.5);  // scale will range from 0 to 1
    
     
    switch (*ptr_wavetype)
    {
    case 1: // Sine 
        delta=(2.0*3.142)/50.0;					// increment 
        for(i=0;i<50;i++) {
            dummy= ((sinf((float)(i*delta))) +1 )* 0.5 * scale * 0xFFFF  ;    // FFFF max value - 5 V, 7FFF mid point value - 2.5 V, 0000 lowest 0V
            data[i]= (unsigned) dummy;			
        }
        break;
    
    case 2: // Square 
        for (i=0; i<50; i++){
            if (i < (50)/2)
            {
                dummy = scale * 0xFFFF;
                data[i] =  (unsigned) dummy;
            }
            else if (i >= 50/2)
            {
                dummy = 0x0000;
                data[i] =  (unsigned) dummy;
            }
        }
        break;
    
    case 3: // Sawtooth 
        delta = 1.0/(50);
 		for (i=0; i<(50); i++){
			if (i <= (50)/2)
			{
				dummy = ( i * delta) * scale  * 0xFFFF;
			}
			if (i > (50)/2 && i <(50))
 			{
				dummy = ( i * delta)  * scale * 0xFFFF;
			}
			data[i] = (unsigned)(dummy); 
 		}
        break;
    
    case 4: // Triangular 
        delta = 2.0/(50);
 		for (i=0; i<50; i++)
 		{
 			if(i <= 50/2)
 	            {
                dummy = i * delta * scale * 0xFFFF;
            }
            if (i > 50/2)
            {
                dummy = (scale * 0xFFFF) - ( (i-25)  * delta * scale *  0xFFFF);
            }
            data[i] = (unsigned)(dummy);
		}
    break;

    default:
        delta=(2.0*3.142)/50.0;					// increment 
        for(i=0;i<50;i++) {
            dummy= ((sinf((float)(i*delta))) +1 )* 0.5 *scale * 0xFFFF  ;    // FFFF max value - 5 V, 7FFF mid point value - 2.5 V, 0000 lowest 0V
            data[i]= (unsigned) dummy;			
        }
        break;
    }
}



// ADC input Thread
void *adcIn(void *arg)
{

	unsigned int count;
    unsigned short chan;
    float amp;
    int freq, wt;	
	
    while(1) {
  
    	count=0x00;
    
    	while (count < 0x02) {
    		chan= ((count & 0x0f)<<4) | (0x0f & count);
    		out16(MUXCHAN,0x0D00|chan);		// Set channel	 - burst mode off.
    		delay(1);											// allow mux to settle
    		out16(AD_DATA,0); 							// start ADC 
    		while(!(in16(MUXCHAN) & 0x4000));
    		adc_in = in16(AD_DATA);   
   		// printw("ADC Chan: %02x Data [%3d]: %4x \n", chan, (int)count, (unsigned int)adc_in);		// print ADC													
   
   			
   			// DIO input for mode switching //
   			

			dio_in=in8(DIO_PORTA); 					// Read Port A	
			//printw("Port A : %02x\n", dio_in);	
			
			if(dio_in - 240 <= 7)
			{
				raise(SIGINT);
			}
			else if(dio_in -248 <= 3)
			{
				mode = 1;						// Hardware Mode
				if(old_mode!=mode){
					updateValues();
					old_mode = mode;
				}
				wt = dio_in - 248 +1;
				if (wavetype != wt){
					pthread_mutex_lock( &mutex );
					wavetype = wt;
					updateValues();
    				waveGeneration(&wavetype, &amplitude);
    				pthread_mutex_unlock( &mutex );
    			}
			}
			else 
			{
				mode = 0;						// Keyboard Mode
				if(old_mode!=mode){
					updateValues();
					old_mode = mode;
				}
			}
   			
   			
   			delay(10);	
   			
   			if(mode){
    								// Write to MUX register - SW trigger, UP, DE, 5v, ch 0-7 	
   
    		if (count == 0x00) {
    			if (adc_in < 0x327c){
    				amp = 0.5;
    			}
    			else if(adc_in >= 0x327c && adc_in < 0x6537){
    				amp = 1.0;
    			}
    			else if(adc_in >= 0x6538 && adc_in < 0x9783){
    				amp = 1.5;
    			}
    			else if(adc_in >= 0x9784 && adc_in < 0xcb6f){
    				amp = 2.0;
    			}
    			else if(adc_in >= 0xcb70 && adc_in < 0xffff){
    				amp = 2.5;
    			}
    
    			if(amplitude!=amp){
    				pthread_mutex_lock( &mutex );
    				amplitude = amp;
    				updateValues();
    				waveGeneration(&wavetype, &amplitude);
    				pthread_mutex_unlock( &mutex );
    			}
    			count = 0x01;
    		}
    		
    		else {
    			freq = (int) (adc_in/66.19 + 10);
    
    			if(frequency!=freq){
    				pthread_mutex_lock( &mutex );
    				frequency = freq;
    				updateValues();
    				period = (float) 1.0/frequency;
    				pthread_mutex_unlock( &mutex );
    			}
    			count = 0x00;
    		}
    		}
    	}
    }
   
}


// Keypress tracking //
void *event_loop(void *arg){
	int ch;
	while ((ch = getch()) != KEY_F(1)){
		if(!mode){
			pthread_mutex_lock( &mutex );
			if (ch == KEY_RIGHT && frequency < 901){
				frequency+= 100;
				period = (float) 1.0/frequency;
				updateValues();
			} 
			else if (ch == KEY_LEFT && frequency > 199){
				frequency -= 100;
				period = (float) 1.0/frequency;
				updateValues();
			}
			else if (ch == KEY_DOWN && amplitude > 0.5){
				amplitude -= 0.5;
				waveGeneration(&wavetype, &amplitude);
				updateValues();
			}
			else if (ch == KEY_UP && amplitude < 2.5){
				amplitude += 0.5;
				waveGeneration(&wavetype, &amplitude);
				updateValues();
			}
            else if (ch == 32){
                if(wavetype!=4){
                    wavetype++;
                }
                else{
                    wavetype = 1;
                }
                waveGeneration(&wavetype, &amplitude);
                updateValues();
            }
         pthread_mutex_unlock( &mutex );
		}
	}
	//pthread_EXIT(NULL);
}

// Exit Signal Handler //
void signalHandler(int signum)
{
    endwin();
    printf("\nYou have either pressed Control C or toggle switch 1 off to exit the program\n");

            // Procedure for writing to file upon exit //
    fp = fopen("waveConfig.txt", "w");

    // Check if the file was opened successfully
    if (fp == NULL) {
        printf("Error opening file\n");
    }
    fprintf(fp, "%d %f %d\n", wavetype, amplitude, frequency);    
    fclose(fp); // Close the file
    pthread_mutex_destroy( &mutex );
    printf("Lastest wave configuration has been saved to waveConfig.txt\n");
    printf("Exit Demo Program\n");
    exit(1);
}

// UI
void startmsg(){

	char *wavetype_str, *mode_str; 

	switch(mode) {
	case 1:
		mode_str = "DAQ     ";
		break;
	case 0:
		mode_str = "Keyboard";
		break;
	}
	
	switch(wavetype) {
	case 1:
		wavetype_str = "Sine      ";
		break;
	case 2:
		wavetype_str = "Square    ";
		break;
	case 3:
		wavetype_str = "Sawtooth  ";
		break;
	case 4:
		wavetype_str = "Triangular";
		break;
	}


  box(stdscr, 0, 0);
  move(0,0);
  addstr("Welcome to our wave generator!\n\n");
  refresh();
  
  // Instructions for Hardware
  attron(A_UNDERLINE);
  addstr("\tDAQ CONTROL INSTRUCTIONS\n");
  attroff(A_UNDERLINE);
  addstr("\tTo enable hardware control, please on toggle switch 2\n");
  addstr("\tUse potentiometer 1 to control amplitude\n");
  addstr("\tUse potentiometer 2 to control frequency\n");
  addstr("\tUse toggle switches 3 and 4 to control the wave type:\n\t00 - Sine\n\t01 - Square\n\t10 - Sawtooth\n\t11 - Triangular\n\n");

  //Instructions for keyboard controls
  attron(A_UNDERLINE);
  addstr("\tKEYBOARD CONTROL INSTRUCTIONS\n");
  attroff(A_UNDERLINE);
  addstr("\tTo enable keyboard control, please off toggle switch 2\n");
  addstr("\tUse up and down arrow keys to control amplitude\n");
  addstr("\tUse left and right arrow keys to control frequency\n");
  addstr("\tUse spacebar to change the wavetype\n\n");

  attron(A_UNDERLINE);
  addstr("\tCurrent Parameters\n");
  attroff(A_UNDERLINE);
  refresh();
  printw("\tControl Mode: %s\t\tWave Type: %s\n", mode_str, wavetype_str); //hardware or keyboard,   use move() to dynamically edit the values when they change respectively
  printw("\tFrequency: %4d\t\t\tAmplitude: %.2f\n", frequency, amplitude);
  curs_set(0);
  refresh();
}

void updateValues(){
	char *wavetype_str, *mode_str; 

	switch(mode) {
	case 1:
		mode_str = "DAQ     ";
		break;
	case 0:
		mode_str = "Keyboard";
		break;
	}
	
	switch(wavetype) {
	case 1:
		wavetype_str = "Sine      ";
		break;
	case 2:
		wavetype_str = "Square    ";
		break;
	case 3:
		wavetype_str = "Sawtooth  ";
		break;
	case 4:
		wavetype_str = "Triangular";
		break;
	}

	move(19,22);  //move cursor to control mode value
	printw("%s", mode_str);
	move(19, 51); //move cursor to wave type value
	printw("%s", wavetype_str);
	move(20,19);  //move cursor to frequency value
	printw("%4d", frequency);
	move(20,51);  //move cursor to amplitude value
	printw("%.2f", amplitude);
	
	//change the values
	refresh();
}

#/** PhEDIT attribute block
#-11:16777215
#0:14018:default:-3:-3:0
#14018:14050:TextFont9:-3:-3:0
#14050:17229:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000174)**/
