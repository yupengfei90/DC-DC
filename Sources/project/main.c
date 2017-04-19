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
#define SampleInterval 10
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
uint8_t IsBoostOnFlag = 0;
uint8_t CurrentModeFlag = 0; /* 0 = bypass; 1 = boost*/
uint8_t BackupModeFlag = 0;
uint8_t FirstSampleFlag = 1; /*1：是; 0: 否*/
uint8_t SuddenChangeIndex = 0; /*4：温度突变; 7: 输入电压突变; 6: 输出电压突变;*/
uint8_t TestValue;
uint8_t ModeChangeDelay = ModeChangeSampleDelayTimes; //模式发生切换时，需要一个参数稳定时间(100ms)
uint8_t LedShineFlag = 0;
uint16_t LedShineTime = 500;
  
/******************************************************************************
* Local variables
******************************************************************************/
volatile uint16_t u16ADC_ConversionBuff[3];

uint16_t Temperature_ADC, InputVol_ADC, OutputVol_ADC;
uint16_t Temperature_ADC_Basic, InputVol_ADC_Basic, OutputVol_ADC_Basic;
uint16_t ADC_DiffValue;
volatile uint16_t u16ADC_ConversionCount = 0;
volatile uint8_t  u8ADC_ConversionFlag = 0;

int main(void)
{
	uint32_t counter = 0;

	sysinit(); /* 初始化bus clock, PIT,ADC等*/
	
	PIT_SetCallback(PIT_CHANNEL0, PIT_Task);
	ADC_SetCallBack(ADC_Task);
	//KBI_SetCallback(KBI0,KBI_Task);  
	
	for(;;) {	   
	   	counter++;
	   	

	    /* set channel to start a new conversion */
	   	u8ADC_ConversionFlag = 0;
	   	ADC_SetChannel(ADC,ADC_CHANNEL_AD4); 	/*温度采集通道*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD7);	/*输入电压采集通道*/	
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD6);	/*输出电压采集通道*/
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
	static uint32_t mode_cnt = 0; 
	static uint8_t sample_cnt = 0;
	
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
					SuddenChangeIndex = 4;
					LedShine(4);
				}
				
				ADC_DiffValue = ABS(InputVol_ADC - InputVol_ADC_Basic);
				if( ADC_DiffValue > InputVol_ADC_Diff){
					SuddenChangeIndex = 7;
					LedShine(7);
				}
				
				ADC_DiffValue = ABS(OutputVol_ADC - OutputVol_ADC_Basic);
				if(ADC_DiffValue > OutputVol_ADC_Diff){
					SuddenChangeIndex = 6;
					LedShine(6);
				}
			}			
		}
		else{ /*模式发生了转化，需要重新备份基底值*/
			if(ModeChangeDelay--); //延时等待参数稳定
			else{ 
				ModeChangeDelay = (ModeChangeSampleDelayTimes / 10);
				//while(!(ABS(OutputVol_ADC - OutputVol_ADC_Basic) > 20)); 
				/*确保处在boost稳压状态，要求InputVol
				 	 	 	 	 	 	 	 	 	 	 	 	 	 	较12.5v有0.5v以上的差别*/
				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
				
				BackupModeFlag = CurrentModeFlag; /*备份过基底值后，切回模式平稳状态*/
			}

		}
			
	}
	
	/*
	 * 参数ADC值发生大幅变化时LedShine()将LedShineFlag置1，闪灯程序开始工作
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
    while( !ADC_IsFIFOEmptyFlag(ADC) )
    {
        for(u16ADC_ConversionCount = 0; u16ADC_ConversionCount < 3; u16ADC_ConversionCount++ )
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
        		default:
        			break;
			}
        }
    }
    
    u8ADC_ConversionFlag = 1;	
}

/*
 * KBI_Task
 */
void KBI_Task(void)
{
	
}
