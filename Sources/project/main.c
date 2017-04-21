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
/*bypass & boost 2S�л�һ��*/
#define ModeToggleTimes 2000 
/*ģʽ�仯���ӳٵȴ��ȶ�ʱ��*/
#define ModeChangeSampleDelayTimes 100
/*�������10ms*/
#define SampleInterval 10
#define Temperature_ADC_Diff 50
/*Լ1v�Ĳ��*/
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
uint8_t FirstSampleFlag = 1; /*1����; 0: ��*/
uint8_t SuddenChangeIndex = 0; /*4���¶�ͻ��; 7: �����ѹͻ��; 6: �����ѹͻ��;*/
uint8_t TestValue;
uint8_t ModeChangeDelay = ModeChangeSampleDelayTimes; //ģʽ�����л�ʱ����Ҫһ�������ȶ�ʱ��(100ms)
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

	sysinit(); /* ��ʼ��bus clock, PIT,ADC��*/
	
	PIT_SetCallback(PIT_CHANNEL0, PIT_Task);
	ADC_SetCallBack(ADC_Task);
	//KBI_SetCallback(KBI0,KBI_Task);  
	
	for(;;) {	   
	   	counter++;
	   	

	    /* set channel to start a new conversion */
	   	u8ADC_ConversionFlag = 0;
	   	ADC_SetChannel(ADC,ADC_CHANNEL_AD4); 	/*�¶Ȳɼ�ͨ��*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD7);	/*�����ѹ�ɼ�ͨ��*/	
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD6);	/*�����ѹ�ɼ�ͨ��*/
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
 * 1ms����һ��
 */
void PIT_Task(void)
{
#ifndef FORCE_STABLIZATION
	static uint32_t mode_cnt = 0; 
	static uint8_t sample_cnt = 0;
	
	/*
	 * 2s��bypass �� boost���л�һ��
	 */
	if(mode_cnt++ >=  ModeToggleTimes) /*2��ִ��һ��*/
	{
		mode_cnt = 0;
		if(IsBoostOnFlag){
			IsBoostOnFlag = 0; /*TOGGLE mode*/
			CurrentModeFlag = 1; 
			
			SetBoost; /*��boost��ѹ��·*/
		}
		else{
			IsBoostOnFlag = 1; /*TOGGLE mode*/
			CurrentModeFlag = 0;
			
			SetBypass; /*��bypass��·*/
		}
	}	

	
	/*
	 * 10ms ����һ��
	 */
	if(SampleInterval== sample_cnt++){
		sample_cnt = 0;
		if(BackupModeFlag == CurrentModeFlag){ /*ģʽû�з���ͻ�䣬����ƽ��״̬������ֵ����ͻȻ�仯*/
			if(FirstSampleFlag == 1){ /*��һ�β������ݻ���ֵ*/
				FirstSampleFlag = 0;
				while(!InputVol_ADC); /*ȷ���ѿ�ʼ����*/ 
				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
			
			} 
			else{ /*�ǵ�һ�β��������л���ֵ��Ϊ�Ƚ�*/
				
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
		else{ /*ģʽ������ת������Ҫ���±��ݻ���ֵ*/
			if(ModeChangeDelay--); //��ʱ�ȴ������ȶ�
			else{ 
				ModeChangeDelay = (ModeChangeSampleDelayTimes / 10);
				//while(!(ABS(OutputVol_ADC - OutputVol_ADC_Basic) > 20)); 
				/*ȷ������boost��ѹ״̬��Ҫ��InputVol
				 	 	 	 	 	 	 	 	 	 	 	 	 	 	��12.5v��0.5v���ϵĲ��*/
				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
				
				BackupModeFlag = CurrentModeFlag; /*���ݹ�����ֵ���л�ģʽƽ��״̬*/
			}

		}
			
	}
	
	/*
	 * ����ADCֵ��������仯ʱLedShine()��LedShineFlag��1�����Ƴ���ʼ����
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
