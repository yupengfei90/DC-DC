/*
 * sysinit.c
 *
 *  Created on: Apr 19, 2017
 *      Author: Administrator
 */


#include "sysinit.h"

/*
 * ADCInit()
 * ADC��������
 * ����Ƶ�ʣ� 10MHz
 * FIFO: 4, fifoScan enable, ����ͨ�������Ĵ�
 * �жϣ� enable
 */
void ADCInit()
{
	 ADC_ConfigType  sADC_Config = {{0}};
	
	/* initiaze ADC module */
	//sADC_Config.u8ClockDiv = ADC_ADIV_DIVIDE_4;
	sADC_Config.u8ClockSource = CLOCK_SOURCE_BUS_CLOCK_DIVIDE_2; 
	sADC_Config.u8Mode = ADC_MODE_10BIT;
	sADC_Config.sSetting.bIntEn = 1;
	//sADC_Config.sSetting.bFiFoScanModeEn = 1; //FIFOģʽ��ͨ����������
	sADC_Config.u8FiFoLevel = ADC_FIFO_LEVEL4;
	
	ADC_Init( ADC, &sADC_Config);
}

/*
 * PitInit()
 * ��ʱ1ms��1ms����һ���ж�
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
	GPIO_PinInit(GPIO_PTB4, GPIO_PinOutput); //LED����������Ϊ���
	GPIO_PinInit(GPIO_PTB5, GPIO_PinOutput); //bypass��������Ϊ���
	GPIO_PinInit(GPIO_PTB7, GPIO_PinOutput); //boost��������Ϊ���
}


/*
 * SystemClockSet()
 * ��ϵͳʱ��Ƶ������Ϊ20MHz
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
 * PIT �жϺ������жϴ���ͨ���ص�����������������
 */
extern PIT_CallbackType PIT_Callback[2]; /*!< PIT callback, ������pit.c�� */
void PIT_CH0_IRQHandler()
{
	PIT_ChannelClrFlags(0);  
	
	if(PIT_Callback[0]){
		PIT_Callback[0]();
	}
}


/*
 * ADC�жϺ������жϴ���ͨ���ص�����������������
 */
extern ADC_CallbackType ADC_Callback[1]; 
void ADC0_IRQHandler()
{
	ADC_Callback[0]();
}



