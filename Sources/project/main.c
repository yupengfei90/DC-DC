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
#define SampleInterval 1
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
union Errors uErrors;
uint8_t IsBoostOnFlag = 0;
uint8_t CurrentModeFlag = 0; /* 0 = bypass; 1 = boost*/
uint8_t BackupModeFlag = 0;
uint8_t FirstSampleFlag = 1; /*1����; 0: ��*/
uint8_t TestValue;
uint8_t ModeChangeDelay = ModeChangeSampleDelayTimes/10; //ģʽ�����л�ʱ����Ҫһ�������ȶ�ʱ��(100ms)
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

	sysinit(); /* ��ʼ��bus clock, PIT,ADC��*/
	
	PIT_SetCallback(PIT_CHANNEL0, PIT_Task);
	ADC_SetCallBack(ADC_Task);
	
	for(;;) {	   
	   	counter++;
	   	

	    /* set channel to start a new conversion */
	   	u8ADC_ConversionFlag = 0;
	   	ADC_SetChannel(ADC,ADC_CHANNEL_AD4); 	/*�¶Ȳɼ�ͨ��*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD7);	/*�����ѹ�ɼ�ͨ��*/	
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD6);	/*�����ѹ�ɼ�ͨ��*/
	    ADC_SetChannel(ADC,ADC_CHANNEL_AD1);	/*CRANK�źŲɼ�ͨ��*/

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
			SetBoost; /*��boost��ѹ��·*/
        }
        else {
			CurrentModeFlag = 0;	
			SetBypass; /*��bypass��·*/
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
	
#ifdef PERIOD_MODE_CHANGE
	static uint32_t mode_cnt = 0; 
#endif
	static uint8_t sample_cnt = 0;
	
#ifdef PERIOD_MODE_CHANGE
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

#endif	
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
				case 0: //����������û���ܵ���Ÿ���
					LedShineFlag = 0; 
					LED_OFF;
					break;
				case 1:	//Temperatures Disturbed
					LedShineFlag = 1;
					LedShineTime = 1000; //1S��1��
					break;
				case 2: //InputVol Disturbed
					LedShineFlag = 2; 
					LedShineTime = 500; //1S��2��
					break;
				case 3: //InputVol and Temperature Disturbed
					LedShineFlag = 3;
					LedShineTime = 250; //1S��4��
					break;
				case 4:	//OutputVol Disturbed
					LedShineFlag = 4;
					LedShineTime = 125; //1S��8��
					break;
				case 5:	//OutputVol and Temperature Disturbed
					LedShineFlag = 5;
					LedShineTime = 100; //1S��10��
					break;
				case 6:	//OutputVol and InputVol Disturbed
					LedShineFlag = 6;
					LedShineTime = 50; //1S��20��
					break;
				case 7:	//OutputVol, InputVol, Temperature all Disturbed
					LedShineFlag = 7;
					LedShineTime = 25; //1S��40��
					break;
				default:
					break;
				}
			}			
		}
		else{ /*ģʽ������ת������Ҫ���±��ݻ���ֵ*/
			if(ModeChangeDelay--); //��ʱ�ȴ������ȶ�
			else{ 
				ModeChangeDelay = (ModeChangeSampleDelayTimes / 10);
				ADC_DiffValue = ABS(OutputVol_ADC - OutputVol_ADC_Basic);

				Temperature_ADC_Basic = Temperature_ADC;
				InputVol_ADC_Basic = InputVol_ADC;
				OutputVol_ADC_Basic = OutputVol_ADC;
				
				BackupModeFlag = CurrentModeFlag; /*���ݹ�����ֵ���л�ģʽƽ��״̬*/
			}

		}
			
	}
	
	/*
	 * ����ADCֵ��������仯ʱ�����Ƴ���ʼ����
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
    while( !ADC_IsFIFOEmptyFlag(ADC) ) //ADC_ReadResultReg������жϱ�־
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

