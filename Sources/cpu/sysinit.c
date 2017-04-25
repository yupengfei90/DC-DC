/*
 * sysinit.c
 *
 *  Created on: Apr 19, 2017
 *      Author: Administrator
 */


#include "sysinit.h"

/*
 * ADCInit()
 * ADC配置如下
 * 采样频率： 10MHz
 * FIFO: 4, fifoScan enable, 即单通道采样四次
 * 中断： enable
 */
void ADCInit()
{
	 ADC_ConfigType  sADC_Config = {{0}};
	
	/* initiaze ADC module */
	//sADC_Config.u8ClockDiv = ADC_ADIV_DIVIDE_4;
	sADC_Config.u8ClockSource = CLOCK_SOURCE_BUS_CLOCK_DIVIDE_2; 
	sADC_Config.u8Mode = ADC_MODE_10BIT;
	sADC_Config.sSetting.bIntEn = 1;
	//sADC_Config.sSetting.bFiFoScanModeEn = 1; //FIFO模式单通道连续采样
	sADC_Config.u8FiFoLevel = ADC_FIFO_LEVEL4;
	
	ADC_Init( ADC, &sADC_Config);
}

/*
 * PitInit()
 * 定时1ms，1ms产生一次中断
 */
void PitInit()
{
	uint32_t  u32LoadValue0;
	PIT_ConfigType  sPITConfig0;
	PIT_ConfigType  *pPIT_Config0   =&sPITConfig0;

	/* Use PIT channel 0 only */
	/* PIT clock source is bus clock,20MHz */
	/* PIT channel 0 load value = (20000-1)*/
	u32LoadValue0   = 19999;                  /*!< PIT ch0 timer load value  */ 	
	
    /* configure PIT channel 0, only enable timer */    
    pPIT_Config0->u32LoadValue      = u32LoadValue0;
    pPIT_Config0->bFreeze           = FALSE;    
    pPIT_Config0->bModuleDis        = FALSE;    /*!< enable PIT module */     
    pPIT_Config0->bInterruptEn      = TRUE;		/*!< enavle Interrupt */
    pPIT_Config0->bChainMode        = FALSE; 
    pPIT_Config0->bTimerEn          = TRUE;

    PIT_Init(PIT_CHANNEL0, pPIT_Config0);      
}

/*
 * led_Pin_init()
 */
void led_Pin_init()
{
	GPIO_PinInit(GPIO_PTB4, GPIO_PinOutput); //LED灯引脚配置为输出
	GPIO_PinInit(GPIO_PTB5, GPIO_PinOutput); //bypass引脚配置为输出
	GPIO_PinInit(GPIO_PTB7, GPIO_PinOutput); //boost引脚配置为输出
}


/*
 * SystemClockSet()
 * 将系统时钟频率配置为20MHz
 */
void SystemClockSet()
{
	ICS->C1|=ICS_C1_IRCLKEN_MASK; /* Enable the internal reference clock*/
	ICS->C3= 0x90; /* Reference clock frequency = 31.25 kHz*/
	while(!(ICS->S & ICS_S_LOCK_MASK)); /* Wait for PLL lock, now running at 40 MHz (1280
	 	 	 	 	 	 	 	 	 	 *31.25 kHz) */
	ICS->C2|=ICS_C2_BDIV(1) ; /*BDIV=2, Bus clock = 20 MHz*/
	ICS->S |= ICS_S_LOCK_MASK ; /* Clear Loss of lock sticky bit */
}
/*
 * sysinit.c
 */
void sysinit()
{
	SystemClockSet();
	led_Pin_init();
	PitInit();
	ADCInit();
}

/*
 * PIT 中断函数，中断处理通过回调函数放在主程序中
 */
extern PIT_CallbackType PIT_Callback[2]; /*!< PIT callback, 定义在pit.c中 */
void PIT_CH0_IRQHandler()
{
	PIT_ChannelClrFlags(0);  
	
	if(PIT_Callback[0]){
		PIT_Callback[0]();
	}
}


/*
 * ADC中断函数，中断处理通过回调函数放在主程序中
 */
extern ADC_CallbackType ADC_Callback[1]; 
void ADC0_IRQHandler()
{
	ADC_Callback[0]();
}



