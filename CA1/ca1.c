#include <stdio.h>  // for standard input output
#include <stdlib.h> // to access system to clear screen
#include <math.h>   // to utilise math functions
#include <ctype.h>  // for fgets()
#include <string.h> // to use strlen
#include <stdbool.h> // to use true false
#include <time.h>   // for time delay


#define BUFFER_SIZE 4096

bool check_int(char *string, int *integer);
void input_coeff(int* a, int* b, int* c);
void input_display(int a, int b, int c);
void clrscr();  
void delay(int number_of_seconds);

int main()
{
    int a; // cant be zero otherwise it will be a linear eqn
    int b;
    int c;
    float x1, x2 = 0;  // The two roots
    float D, denom;    // D = Discriminant, denom = Denominator
    char rerun; // to check if user wants to rerun the program

    do{
        input_coeff(&a, &b, &c);
        printf("\n");
        printf("Quadratic equation\n");
        printf("%d(x^2) + %d(x) + %d = 0\n", a, b, c);
        printf("\n\n");

        // delay by 2 sec before clearing terminal
        printf("DISPLAYING RESULT IN 2 SECONDS...");
        delay(2); 
        clrscr();
        
        D = b*b - 4*a*c;
        denom = 2 * a;

        printf("INPUT DISPLAY:\n");
        input_display(a, b, c);
        printf("OUTPUT DISPLAY:\n");


        if (D > 0)
        {   
            x1 = (-b + sqrt(D)) / denom;  // D and denom are float, dont need to typecast
            x2 = (-b - sqrt(D)) / denom;    

            printf("------------------------------------------------------\n");
            printf("| Discriminant |   Type of roots  | Root 1 |  Root 2 |\n");
            printf("|--------------|------------------|--------|---------|\n");
            printf("|     %-9.1f| REAL & DIFFERENT | %-7.3f| %7.3f | \n", D, x1, x2);
            printf("------------------------------------------------------\n");
        }
        
        else if (D == 0)
        {   
            x1 = x2 = (float) -b / (2*a);      // typecast required as all variables here are in int

            printf("-----------------------------------------------------\n");
            printf("| Discriminant |   Type of roots  | Root 1 | Root 2 |\n");
            printf("|--------------|------------------|--------|--------|\n");
            printf("|     %-9.1f|   REAL & EQUAL   | %-7.3f| %6.3f | \n", D, x1, x2);
            printf("-----------------------------------------------------\n");
        }
    
    else
    {
            float real, img;
            real = (float) -b / (2*a);
            img = (float) sqrt(-D) / (2*a);

            printf("---------------------------------------------------------------------\n");
            printf("| Discriminant |   Type of roots   |     Root 1     |     Root 2    |\n");
            printf("|--------------|-------------------|----------------|---------------|\n");
            printf("|    %-9.1f | COMPLEX conjugate | %5.2f + %4.2fi  | %5.2f - %4.2fi | \n", D, real, img, real, img);
            printf("---------------------------------------------------------------------\n");
        }

    printf("\n");
    printf("Do you want to solve another Quadratic Equation?\nType 'y' to continue or any other keys to exit: ");
    scanf("%c",&rerun);
    getchar(); //  newline buffer
    clrscr();   // reset terminal
    }while(rerun == 'y' || rerun == 'Y'); 

    // If user terminates program // 
    clrscr(); 
    printf("Thank you for using our Quadratic Equation Solver Program!");
    printf("\n");
    return 0;
}

///////////////////// Function 1: If string format is able to return integer, returns true /////////////////////
bool check_int(char *string, int *integer)
{
    /* ***This program only works for INT_MIN <= integer <= INT_MAX*** */
    int i = 0;

    while (isspace(string[i]))
    {
        i++; // as long it is whitespace, go to next character in the string
    }

    int length = strlen(string); 

    if (length == i) return false; // edge case where the whole string is just whitespace 
    
    char integer_buffer[BUFFER_SIZE]; // create an array of numeric characters to track the index of each digit characters include '-' negative symbol
    int integer_chars = 0;  // create index for valid integers character

    if (string[i] == '-')
    {
        integer_buffer[integer_chars] = '-';  // set '-' at index 0
        integer_chars++;                     // moves to index 1
        i++;                                // moves to the next char in the string

        if (!isdigit(string[i])) return false; // if next char after '-' is not int, return false
    }

    while (i < length && !isspace(string[i]))
    {
        if (!isdigit(string[i])) return false;  // if after '-' is a non numeric character, returns false
        
        else
        {
            integer_buffer[integer_chars] = string[i]; // store the index of next digit char from string i
            integer_chars++;
            i++;
        }
    }
    integer_buffer[integer_chars] = '\0';       // terminate the string of numeric characters

    while (isspace(string[i])) i++;  // loop through every whitespace after the numeric characters if theres any
    if (string[i] != '\0') return false; // return false if after those whitespaces, there are still other characters: improper format

    *integer = atoi(integer_buffer); // convert the strings of numeric characters into int and store into the dereferenced pointer value integer
    return true;
}

///////////////////// Function 2: Retrieve input coefficients of the quadratic equations from user /////////////////////
void input_coeff(int* a, int* b, int* c)
{
    bool valid_integer_a = true;
    bool valid_integer_b = true;
    bool valid_integer_c = true;
    
    do
    {
        char buffer[BUFFER_SIZE];

        printf("\n");
        printf("TERMINAL INPUT:\n");

        printf("Enter integer a: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        valid_integer_a = check_int(buffer, a);

        printf("Enter integer b: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        valid_integer_b = check_int(buffer, b);

        printf("Enter integer c: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        valid_integer_c = check_int(buffer, c);

        if (!valid_integer_a)
        {
            printf("a must be an integer value!\n");
        }

        if (!valid_integer_b)
        {
            printf("b must be an integer value!\n");
        }

        if (!valid_integer_c)
        {
            printf("c must be an integer value!\n");
        }
    } while (!valid_integer_a || !valid_integer_b || !valid_integer_c); // checks if all a,b,c are valid integers
}

// Function 3: To display the input table // 
void input_display(int a, int b, int c)
{
    printf("------------------------------------------------\n");
    printf("|  a  |  b  |  c  |     Quadratic Equation     |\n");
    printf("|-----|-----|-----|----------------------------|\n");

    // conditional statement to check for the signs display of quadratic equation
    if(b > 0 && c > 0)
    {
        printf("| %-3d | %-3d | %3d | %3d (x^2)  + %3d (x) + %3d |\n", a, b, c, a, b, c);
    }

    else if(b < 0 && c > 0)
    {
        int bb = fabs(b); // returning absolute value of b
        printf("| %-3d | %-3d | %3d | %3d (x^2)  - %3d (x) + %3d |\n", a, b, c, a, bb, c);
    }

    else if(b > 0 && c < 0)
    {
        int cc = fabs(c);
        printf("| %-3d | %-3d | %3d | %3d (x^2)  + %3d (x) - %3d |\n", a, b, c, a, b, cc);
    }
    
    else
    {
        int bb = fabs(b);
        int cc = fabs(c);
        printf("| %-3d | %-3d | %3d | %3d (x^2)  - %3d (x) - %3d |\n", a, b, c, a, bb, cc);
    }

    printf("------------------------------------------------\n");
    printf("\n");

}

// Function 4:  To clear terminal //
void clrscr()
{
    system("@cls||clear"); // cls for windows command prompt, clear is for unix based bash
}

// Function 5: to delay by indicated number of seconds //
void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}