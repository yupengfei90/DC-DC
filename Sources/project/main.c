/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "common.h"
#include "gpio.h"
/***********************************************
*Global macros
************************************************/
#define LED_ON do{GPIO_PinInit(GPIO_PTB4, GPIO_PinOutput);GPIO_PinSet(GPIO_PTB4);}while(0);

int main(void)
{
	int counter = 0;
	
	
	LED_ON;
	
	for(;;) {	   
	   	counter++;
	}
	
	return 0;
}
