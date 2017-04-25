/*
 * main implementation: use this 'C' sample to create your own application
 *
 */





#include "common.h"
#include "sysinit.h"
/***********************************************
*Global macros
************************************************/ 
//#define FORCE_STABLIZATION
/*bypass & boost 2S切换一次*/
#define ModeToggleTimes 2000 
/*模式变化，延迟等待稳定时间*/
#define ModeChangeSampleDelayTimes 100
/*采样间隔10ms*/
#define SampleInterval 1
#define Temperature_ADC_Diff 50
/*约1v的差别*/
#define InputVol_ADC_Diff 50
#define OutputVol_ADC_Diff 50

/*
 * Local functions
 */
void PIT_Task(void);
void ADC_Task(void);
void KBI_Task(void);

/******************************************************************************
* Global variables
******************************************************************************/
union Errors uErrors;
uint8_t IsBoostOnFlag = 0;
uint8_t CurrentModeFlag = 0; /* 0 = bypass; 1 = boost*/
uint8_t BackupModeFlag = 0;
uint8_t FirstSampleFlag = 1; /*1：是; 0: 否*/
uint8_t TestValue;
uint8_t ModeChangeDelay = ModeChangeSampleDelayTimes/10; //模式发生切换时，需要一个参数稳定时间(100ms)
uint8_t LedShineFlag = 0;
uint16_t LedShineTime = 500;
  
/******************************************************************************
* Local variables
******************************************************************************/
volatile uint16_t u16ADC_ConversionBuff[4];

uint16_t Temperature_ADC, InputVol_ADC, OutputVol_ADC, CRANK_ADC;
uint16_t Temperature_ADC_Basic, InputVol_ADC_Basic, OutputVol_ADC_Basic;
volatile uint16_t ADC_DiffValue;
volatile uint16_t u16ADC_ConversionCount = 0;
volatile uint8_t  u8ADC_ConversionFlag = 0;

int main(void)
{
	uint32_t counter = 0;

	sysinit(); /* 初始化bus clock, PIT,ADC等*/
	
	PIT_SetCallback(PIT_CHANNEL0, PIT_Task);
	ADC_SetCallBack(ADC_Task);
	
	for(;;) {	   
	   	counter++;
	   	

	    /* set channel to start a new conversion */
	   	u8ADC_ConversionFlag = 0;
	   	ADC_SetChannel(ADC,ADC_CHANNEL_AD4); 	/*温度采集通道*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD7);	/*输入电压采集通道*/	
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD6);	/*输出电压采集通道*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD1);	/*CRANK信号采集通道*/

        /* wait conversion complete */
        while( !u8ADC_ConversionFlag);

#if defined(FORCE_STABLIZATION)
        if(InputVol_ADC > 350){
        	SetBoost;
        } 
        else{
        	SetBypass;
        }
#endif       

#ifndef PERIOD_MODE_CHANGE

        if(CRANK_ADC > 100){
			CurrentModeFlag = 1; 				
			SetBoost; /*打开boost稳压电路*/
        }
        else {
			CurrentModeFlag = 0;	
			SetBypass; /*打开bypass电路*/
        }
#endif
        
        u16ADC_ConversionCount = 0;
	}
	
	return 0;
}

/*
 * PIT ISR
 * 1ms进入一次
 */
void PIT_Task(void)
{
#ifndef FORCE_STABLIZATION
	
#ifdef PERIOD_MODE_CHANGE
	static uint32_t mode_cnt = 0; 
#endif
	static uint8_t sample_cnt = 0;
	
#ifdef PERIOD_MODE_CHANGE
	/*
	 * 2s在bypass 和 boost间切换一次
	 */
	if(mode_cnt++ >=  ModeToggleTimes) /*2秒执行一次*/
	{
		mode_cnt = 0;
		if(IsBoostOnFlag){
			IsBoostOnFlag = 0; /*TOGGLE mode*/
			CurrentModeFlag = 1; 
			
			SetBoost; /*打开boost稳压电路*/
		}
		else{
			IsBoostOnFlag = 1; /*TOGGLE mode*/
			CurrentModeFlag = 0;
			
			SetBypass; /*打开bypass电路*/
		}
	}	

#endif	
	/*
	 * 10ms 采样一次
	 */
	if(SampleInterval== sample_cnt++){
		sample_cnt = 0;
		if(BackupModeFlag == CurrentModeFlag){ /*模式没有发生突变，处于平稳状态，采样值不会突然变化*/
			if(FirstSampleFlag == 1){ /*第一次采样备份基底值*/
				FirstSampleFlag = 0;
				while(!InputVol_ADC); /*确保已开始采样*/ 
				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
			
			} 
			else{ /*非第一次采样，已有基底值作为比较*/
				
				ADC_DiffValue = ABS(Temperature_ADC - Temperature_ADC_Basic);
				if( ADC_DiffValue > Temperature_ADC_Diff){
					uErrors.sBits.TemperatureDisturbed = 1;
				} else {
					uErrors.sBits.TemperatureDisturbed = 0;
				}
				
				ADC_DiffValue = ABS(InputVol_ADC - InputVol_ADC_Basic);
				if( ADC_DiffValue > InputVol_ADC_Diff){
					uErrors.sBits.InputVolDisturbed = 1;
				} else {
					uErrors.sBits.InputVolDisturbed = 0;
				}
				
				ADC_DiffValue = ABS(OutputVol_ADC - OutputVol_ADC_Basic);
				if(ADC_DiffValue > OutputVol_ADC_Diff){
					uErrors.sBits.OutputVolDisturbed = 1;
				} else {
					uErrors.sBits.OutputVolDisturbed = 0;
				}
				
				switch(uErrors.AllErrors)
				{
				case 0: //运行正常，没有受到电磁干扰
					LedShineFlag = 0; 
					LED_OFF;
					break;
				case 1:	//Temperatures Disturbed
					LedShineFlag = 1;
					LedShineTime = 1000; //1S闪1次
					break;
				case 2: //InputVol Disturbed
					LedShineFlag = 2; 
					LedShineTime = 500; //1S闪2次
					break;
				case 3: //InputVol and Temperature Disturbed
					LedShineFlag = 3;
					LedShineTime = 250; //1S闪4次
					break;
				case 4:	//OutputVol Disturbed
					LedShineFlag = 4;
					LedShineTime = 125; //1S闪8次
					break;
				case 5:	//OutputVol and Temperature Disturbed
					LedShineFlag = 5;
					LedShineTime = 100; //1S闪10次
					break;
				case 6:	//OutputVol and InputVol Disturbed
					LedShineFlag = 6;
					LedShineTime = 50; //1S闪20次
					break;
				case 7:	//OutputVol, InputVol, Temperature all Disturbed
					LedShineFlag = 7;
					LedShineTime = 25; //1S闪40次
					break;
				default:
					break;
				}
			}			
		}
		else{ /*模式发生了转化，需要重新备份基底值*/
			if(ModeChangeDelay--); //延时等待参数稳定
			else{ 
				ModeChangeDelay = (ModeChangeSampleDelayTimes / 10);
				ADC_DiffValue = ABS(OutputVol_ADC - OutputVol_ADC_Basic);

				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
				
				BackupModeFlag = CurrentModeFlag; /*备份过基底值后，切回模式平稳状态*/
			}

		}
			
	}
	
	/*
	 * 参数ADC值发生大幅变化时，闪灯程序开始工作
	 */
	if(LedShineFlag){ 
		static uint16_t tmr;
		if(tmr++ == LedShineTime){
			tmr = 0;
			LED_TOGGLE;
		}
	}
#endif
}

/*
 * ADC ISR
 */
void ADC_Task(void)
{
    while( !ADC_IsFIFOEmptyFlag(ADC) ) //ADC_ReadResultReg会清除中断标志
    {
        for(u16ADC_ConversionCount = 0; u16ADC_ConversionCount < 4; u16ADC_ConversionCount++ )
        {
        	switch(u16ADC_ConversionCount)
			{
        		case 0:
        			Temperature_ADC = ADC_ReadResultReg(ADC);
        			break;
        		case 1:
        			InputVol_ADC = ADC_ReadResultReg(ADC);
        			break;
        		case 2:
        			OutputVol_ADC = ADC_ReadResultReg(ADC);
        		    break;
        		case 3:
        			CRANK_ADC = ADC_ReadResultReg(ADC);
        		    break;
        		default:
        			break;
			}
        }
    }
    
    u8ADC_ConversionFlag = 1;	
}

