#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>


// FOR PLOTTING PURPOSE
#include "pbPlots.h"
#include "supportLib.h"

// Constant Variables //
const float PI = 3.141592653589793238;

// Functions //
void cmdArg(int argc, char **argv, int *ptr_wavetype, int *ptr_amp, float *ptr_freq);
void waveGenerator(int *ptr_wavetype, int *n, double **data_y, double **data_x); 
void currpath_dir(char **retptr);
void fileWriter(char *retptr);

// Main thread //
int main(int argc, char **argv)
{
    int n, wavetype, amplitude;
    float frequency;
	_Bool success;
	double *data_x, *data_y;
    char *retptr;
    
    cmdArg(argc, argv, &wavetype, &amplitude, &frequency);
    waveGenerator(&wavetype, &n, &data_y, &data_x);  
    currpath_dir(&retptr);
    fileWriter(retptr);

    printf("Amplitude = %d\n", amplitude);  
    printf("Frequency = %.2f\n", frequency);  

	/*FOR PLOTTING PURPOSE*/ 

	// TYPE OF WAVEFORM //
    // waveGenerator(n, delta, data_ptr, xs_ptr);
	
	// PLOTTING // 
	RGBABitmapImageReference *canvasReference = CreateRGBABitmapImageReference();
	StringReference *errorMessage = CreateStringReference(L"", 0);

	double arr1[255], arr2[255];

	for(int i=0; i<n; i++)
	{
		arr1[i] = data_x[i];
		arr2[i] = data_y[i];
	}
	free(data_y);
	free(data_x);
	success = DrawScatterPlot(canvasReference, 600, 400, arr1, n, arr2, n, errorMessage);

	if(success){
		size_t length;
		double *pngdata = ConvertToPNG(&length, canvasReference->image);
		WriteToFile(pngdata, length, "example1.png");
		DeleteImage(canvasReference->image);
	}else{
		fprintf(stderr, "Error: ");
		for(int i = 0; i < errorMessage->stringLength; i++){
			fprintf(stderr, "%c", errorMessage->string[i]);
		}
		fprintf(stderr, "\n");
	}

	/*FOR PLOTTING PURPOSE*/

 
    free(retptr);

	FreeAllocations();
	return success ? 0 : 1;
    // return 0;
}

// Function 1: Command line arguments //
void cmdArg(int argc, char **argv, int *ptr_wavetype, int *ptr_amp, float *ptr_freq)           // *ptr_wavetype -> &wavetype
{   
    char arr[4][100] = {"Sine", "Square", "Sawtooth", "Triangular"}; // an array of different waveforms
    char **ptr_argv = &argv[1];     // start first argument after name of file.exe
    bool valid_wave = false;

    // For Waveform //
    for (int i = 0; i < 4; i++)
    {
        if (strcmpi(arr[i], *(ptr_argv)) == 0)     // return 0 if all chars are = (ignore cases)
        {
            // printf("Plotting %s Wave...\n", arr[i]);
            valid_wave = true;
            *ptr_wavetype = i + 1;       // 1st argument = Type of waveform - 1: Sine, 2: Square, 3: Sawtooth, 4: Triangular (call by ref)
        }
    }
    
    // Amplitude //
    *ptr_amp = atoi(*(ptr_argv + 1));      // 2nd argument = Amplitude    
   // Frequency //
    *ptr_freq = atof(*(ptr_argv + 2));      // 3rd argument = Frequency
}

// Function 2: Wave Generator //
void waveGenerator(int *ptr_wavetype, int *n, double **data_y, double **data_x)
{
    double delta, dummy;

    printf("Enter the number of points to be generated for the signal wave: \n");
    scanf("%d", n);
    *data_y  = (double*)malloc((*n)*sizeof(double));
    *data_x  = (double*)malloc((*n)*sizeof(double));
    
    switch(*ptr_wavetype)
    {
        case 1: // SINE  
            delta = (2.0 * PI) / (*n);

            for (int i = 0; i < (*n); i++)
            {
                (*data_y)[i] = sinf(i*delta);
                printf("sample point #%d = %lf\n", i, (*data_y)[i]);
				(*data_x)[i] = delta * i;
            }

            printf("\nGenerating Sine waveform...\n");
            printf("Length of array = %d\n", (*n));
            break;

        case 2: // SQUARE 
			delta = (2.0 * PI)/(*n);
			for (int i=0; i<(*n); i++)
			{
				if (i < (*n)/2)
				{
					(*data_y)[i] = 1;
				}
				else if (i >= (*n)/2)
				{
					(*data_y)[i] = -1;
				}
                printf("sample point #%d = %lf\n", i, (*data_y)[i]);
				(*data_x)[i] = delta * i;
			}
            printf("\nGenerating Square waveform...\n");
            printf("Length of array = %d\n", (*n));
            break;

        case 3: // SAWTOOTH 
            delta = 2.0/(*n);
			for (int i=0; i<(*n); i++)
			{
				if (i <= (*n)/2)
				{
					dummy = i * delta;
				}

				if (i > (*n)/2 && i <(*n))
				{
					dummy = (-2 + i * delta);
				}
				(*data_y)[i] = dummy / 5;
                printf("sample point #%d = %lf\n", i, (*data_y)[i]);
				(*data_x)[i] = delta * i;   
			}
            printf("\nGenerating Sawtooth waveform...\n");
            printf("Length of array = %d\n", (*n));
            break;

        case 4: // Triangular
            delta = 4.0/(*n);
			for (int i=0; i<(*n); i++)
			{
				if (i <= (*n)/4)
				{
					dummy = i * delta;
				}
				
				if (i > (*n)/4 && i <= (*n) * 3/4)
				{
					dummy = (2 - i * delta);
				}

				if (i > (*n) * 3/4 && i <= (*n))
				{
					dummy = (-4 + i * delta);
				}

				(*data_y)[i] = dummy / 5;
                printf("sample point #%d = %lf\n", i, (*data_y)[i]);
				(*data_x)[i] = delta * i;
			}
            printf("\nGenerating Triangular waveform...\n");
            printf("Length of array = %d\n", (*n));
            break;

        default: // Default sine wave
            printf("\nDefault: Generating Sine waveform...\n");
            delta = (2.0 * PI) / (*n);

            for (int i = 0; i < (*n); i++)
            {
                (*data_y)[i] = sinf(i*delta);
                printf("sample point #%d = %lf\n", i, (*data_y)[i]);
				(*data_x)[i] = delta * i;
            }
            printf("Length of array = %d\n", (*n));
    }
}

void currpath_dir(char **retptr)
{
    *retptr = _getcwd(NULL, 1024); // dynamically allocate memory for characters of path_dir even if exceeds 1024

    if (*retptr == NULL)
    {
        printf("Failed to get current working directory.\n");

        if (errno == ERANGE)
            printf("path_dir exceeds max buffer length.\n");
        else if (errno == ENOMEM)
            printf("Memory cannot be allocated for path_dir.\n");
    }

    // printf("(Currpath_dir) Current working directory: %s\n", *retptr);
}

void fileWriter(char *retptr)
{
    char prefix[1024];
    char suffix[50] = "\\output.txt";
    char path_dir[ 2 * sizeof(prefix) ]; //  ensure buffer is big enough! (path_dir is retptr with DOUBLE backslashes)
    
    // This code segment is to change single \ to \\ for windows path_dir... May not to change when working with unix // 
    strcpy(prefix, retptr);
    strcat(prefix, suffix);

    int i, j;
    for (i = j = 0; i < (int)strlen(prefix); i++, j++) {
        path_dir[j] = prefix[i];
        if (prefix[i] == '\\') 
            path_dir[++j] = '\\'; // Insert extra backslash
    }
    path_dir[j] = '\0'; // We need to add nul-terminator!)
    
    printf("output address = %s\n", path_dir);

   FILE *pF = fopen(path_dir, "w");

   fprintf(pF, "POTENTIOMETER DATA...(INCOMPLETE)"); 

   fclose(pF); // pointer as argument
   
   // DELETE A FILE
   /*
   if(remove("test.txt") == 0)
   {
      printf("That file was deleted successfully!");
   }
   else
   {
      printf("That file was NOT deleted!");
   }
   */
}