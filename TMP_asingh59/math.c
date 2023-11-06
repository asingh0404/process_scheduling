#include <stdio.h>
#include <math.h>
#include <kernel.h>

/* Calculate x to the power y */
double pow(double x, int y)
{
	double result = 1;
	
	int exp = y;

	while ( exp > 0 )
	{
		result = result * x;
		exp--;
	}

	return result;
}

/* Log function using the Taylor Series */
double log(double x)
{
	if(x <= 0)
	{
		return SYSERR;	
	}
	
	double log_ans = 0;
	
	int i;
	
	for(i = 1; i<=20; i++)
	{
		log_ans = log_ans + (pow(-1, i-1) * (pow(x-1, i)/i));
	}

	return log_ans;
	
}

/* Function to generate random number based on exponential distribution */
double expdev(double lambda)
{
	
	double dummy;
	do
		dummy = (double) rand()/RAND_MAX;
	while (dummy == 0.0);
	
	return -log(dummy) / lambda;
}