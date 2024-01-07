#include "pbPlots.h"
#include "supportLib.h"
#include <stdio.h>
#include <math.h>

const double PI = 3.141592653589793238;

void sinewave(int n, double delta, double *data, double *xs);
void squarewave(int n, double delta, double *data, double *xs);
void sawtoothwave(int n, double delta, double *data, double *xs);
void triangularwave(int n, double delta, double *data, double *xs);
void waveGenerator(int n, double delta, double *data, double *xs);

int main(){
	const int n = 50;
	_Bool success;

	double delta;
    double data[50]; // y-axis
    double *data_ptr = data;
	double xs[50]; // x-axis
	double *xs_ptr = xs;

	// TYPE OF WAVEFORM //
    waveGenerator(n, delta, data_ptr, xs_ptr);
	
	// PLOTTING // 
	RGBABitmapImageReference *canvasReference = CreateRGBABitmapImageReference();
	StringReference *errorMessage = CreateStringReference(L"", 0);
	success = DrawScatterPlot(canvasReference, 600, 400, xs, n, data, n, errorMessage);

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

	FreeAllocations();

	return success ? 0 : 1;
}

void sinewave(int n, double delta, double *data, double *xs)
{   
    delta = (2.0 * PI)/n;
    
    for (int i=0; i<n; i++)
    {
		// creating y-axis //
        data[i] = sinf(i*delta);

		// creating x-axis //
		xs[i] = delta * i;
    }
}

void squarewave(int n, double delta, double *data, double *xs)
{
	delta = (2.0 * PI)/n;
	
    for (int i=0; i<n; i++)
    {
        if (i < n/2)
        {
            data[i] = 1;
        }
        else if (i >= n/2)
        {
            data[i] = -1;
        }
        
        xs[i] = delta * i;
    }
}

void sawtoothwave(int n, double delta, double *data, double *xs)
{
	double dummy;
	delta = 2.0/n;
	
    for (int i=0; i<n; i++)
    {
		if (i <= n/2)
		{
			dummy = i * delta;
		}

		if (i > n/2 && i <n)
		{
			dummy = (-2 + i * delta);
		}
		data[i] = dummy / 5;
		xs[i] = delta * i;
    }
}

void triangularwave(int n, double delta, double *data, double *xs)
{
	double dummy;
	delta = 4.0/n;
	
    for (int i=0; i<n; i++)
    {
		if (i <= n/4)
		{
			dummy = i * delta;
		}
		
		if (i > n/4 && i <= n * 3/4)
		{
			dummy = (2 - i * delta);
		}

		if (i > n * 3/4 && i <= n)
		{
			dummy = (-4 + i * delta);
		}

		data[i] = dummy / 5;
		xs[i] = delta * i;
    }
}

void waveGenerator(int n, double delta, double *data, double *xs)
{
	int input;
	double dummy;

	printf("WAVEFORM SELECTION\n");
	printf("Key '1' for Sine Wave\n");
	printf("Key '2' for Square Wave\n");
	printf("Key '3' for Sawtooth Wave\n");
	printf("Key '4' for Triangular Wave\n");

	scanf("%d", &input);



	switch(input)
	{
		case 1:
			delta = (2.0 * PI)/n;

			for (int i=0; i<n; i++)
			{
				// creating y-axis //
				data[i] = sinf(i*delta);
				// creating x-axis //
				xs[i] = delta * i;
			}
			break;

		case 2:
			delta = (2.0 * PI)/n;
			for (int i=0; i<n; i++)
			{
				if (i < n/2)
				{
					data[i] = 1;
				}
				else if (i >= n/2)
				{
					data[i] = -1;
				}
				xs[i] = delta * i;
			}
			break;

		case 3:
			delta = 2.0/n;
			for (int i=0; i<n; i++)
			{
				if (i <= n/2)
				{
					dummy = i * delta;
				}

				if (i > n/2 && i <n)
				{
					dummy = (-2 + i * delta);
				}
				data[i] = dummy / 5;
				xs[i] = delta * i;
			}
			break;

		case 4:
			delta = 4.0/n;
			for (int i=0; i<n; i++)
			{
				if (i <= n/4)
				{
					dummy = i * delta;
				}
				
				if (i > n/4 && i <= n * 3/4)
				{
					dummy = (2 - i * delta);
				}

				if (i > n * 3/4 && i <= n)
				{
					dummy = (-4 + i * delta);
				}

				data[i] = dummy / 5;
				xs[i] = delta * i;
			}
			break;

		default:
			printf("Invalid choice");
	}
}