/*
 * sysinit.h
 *
 *  Created on: Apr 19, 2017
 *      Author: Administrator
 */

#ifndef SYSINIT_H_
#define SYSINIT_H_
#include "common.h"
#include "gpio.h"
#include "pit.h"
#include "adc.h"
#include "kbi.h"

/*
 * External varibles
 */
extern uint8_t LedShineFlag ;
extern uint16_t LedShineTime ;

/*
 * Global Functions
 */
void sysinit();
void LedShine(uint8_t channel);

#if defined(SINGLE_MODULE_TEST)
	void SystemClockSet();
	void led_Pin_init();
	void PitInit();
	void ADCInit();
	void KBIInit();
#endif

/*
 * Global macros
 */
#define LED_ON do{GPIO_PinSet(GPIO_PTB4);}while(0);
#define LED_OFF do{GPIO_PinClear(GPIO_PTB4);}while(0);
#define LED_TOGGLE do{GPIO_PinToggle(GPIO_PTB4);}while(0);

#define BypassOn do{GPIO_PinSet(GPIO_PTB5);}while(0)
#define BypassOff do{GPIO_PinClear(GPIO_PTB5);}while(0)
#define BoostOn do{GPIO_PinSet(GPIO_PTB7);}while(0)
#define BoostOff do{GPIO_PinClear(GPIO_PTB7);}while(0)


#define SetBoost do{BypassOn;BoostOn;}while(0)
#define SetBypass do{BypassOff;BoostOff;}while(0)

#define ABS(x) ((x)>0) ? (x) : (-(x))
#endif /* SYSINIT_H_ */
