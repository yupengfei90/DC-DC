/*
 *    kinetis_sysinit.c - Default init routines for Flycatcher
 *                     		Kinetis ARM systems
 *    Copyright �2012 Freescale semiConductor Inc. All Rights Reserved.
 */
 
#include "kinetis_sysinit.h"
#include "derivative.h"

/**
 **===========================================================================
 **  External declarations
 **===========================================================================
 */
#if __cplusplus
extern "C" {
#endif
extern uint32_t __vector_table[];
extern unsigned long _estack;
extern void __thumb_startup(void);
#if __cplusplus
}
#endif

/**
 **===========================================================================
 **  Default interrupt handler(illege access)
 **===========================================================================
 */
void Default_Handler()
{
	__asm("bkpt");
}

#define NVIC_IP_PRI_6_MASK                       0xFF0000u
#define NVIC_IP_PRI_6_SHIFT                      16
#define NVIC_IP_PRI_6(x)                         (((uint32_t)(((uint32_t)(x))<<NVIC_IP_PRI_6_SHIFT))&NVIC_IP_PRI_6_MASK)

void PE_low_level_init(void)
{
      /* Initialization of the SIM module */
        /* Initialization of the FTMRx_FlashConfig module */
      /* Initialization of the PMC module */
  /* PMC_SPMSC2: LVDV=0,LVWV=0 */
  PMC->SPMSC2 &= (uint8_t)~(uint8_t)(
                 PMC_SPMSC2_LVDV_MASK |
                 PMC_SPMSC2_LVWV(0x03)
                );
  /* PMC->SPMSC1: LVWACK=1,LVWIE=0,LVDRE=1,LVDSE=1,LVDE=1,??=0,BGBE=0 */
  PMC->SPMSC1 = (uint8_t)((PMC->SPMSC1 & (uint8_t)~(uint8_t)(
                PMC_SPMSC1_LVWIE_MASK |
                PMC_SPMSC1_BGBE_MASK |
                0x02U
               )) | (uint8_t)(
                PMC_SPMSC1_LVWACK_MASK |
                PMC_SPMSC1_LVDRE_MASK |
                PMC_SPMSC1_LVDSE_MASK |
                PMC_SPMSC1_LVDE_MASK
               ));
  /* Common initialization of the CPU registers */
  /* SIM->SOPT: CLKOE=0,RSTPE=1,NMIE=0 */
  SIM->SOPT = (uint32_t)((SIM->SOPT & (uint32_t)~(uint32_t)(
              SIM_SOPT_CLKOE_MASK |
              SIM_SOPT_NMIE_MASK
             )) | (uint32_t)(
              SIM_SOPT_RSTPE_MASK
             ));
  /* NVIC_IPR1: PRI_6=0 */
  NVIC->IP[1] &= (uint32_t)~(uint32_t)(NVIC_IP_PRI_6(0xFF));
  //__EI();
}
  /* Flash configuration field */
  __attribute__ ((section (".cfmconfig"))) const uint8_t _cfm[0x10] = {
   /* NV_BACKKEY0: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY1: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY2: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY3: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY4: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY5: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY6: KEY=0xFF */
    0xFFU,
   /* NV_BACKKEY7: KEY=0xFF */
    0xFFU,
    0xFFU,
    0xFFU,
    0xFFU,
    0xFFU,
    0xFFU,
   /* NV_FPROT: FPOPEN=1,??=1,FPHDIS=1,FPHS=3,FPLDIS=1,FPLS=3 */
    0xFFU,
   /* NV_FSEC: KEYEN=3,??=1,??=1,??=1,??=1,SEC=2 */
    0xFEU,
   /* NV_FOPT: ??=1,??=1,??=1,??=1,??=1,??=1,??=1,??=1 */
    0xFFU
  };

/**
 **===========================================================================
 **  Reset handler
 **===========================================================================
 */
void __init_hardware()
{
	SCB->VTOR = (uint32_t)__vector_table; /* Set the interrupt vector table position */
	
	/* Disable the Watchdog because it may reset the core before entering main(). */
	
	WDOG->TOVAL = 0xE803; // setting timeout value
	WDOG->CS2 = WDOG_CS2_CLK_MASK; // setting 1-kHz clock source
	WDOG->CS1 = 0x23; // Watchdog disabled, 
					 // Watchdog interrupts are disabled. Watchdog resets are not delayed, 
					 // Updates allowed. Software can modify the watchdog configuration registers within 128 bus clocks after performing the unlock write sequence,
					 // Watchdog test mode disabled,
					 // Watchdog disabled in chip debug mode,
					 // Watchdog enabled in chip wait mode,
					 // Watchdog enabled in chip stop mode.
	
	PE_low_level_init();
}

/* Weak definitions of handlers point to Default_Handler if not implemented */
void NMI_Handler() __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler() __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler")));

void FTMRE_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PMC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void IRQ_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void UART0_SCI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void ACMP0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTM0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTM2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void ACMP1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PIT_CH0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PIT_CH1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void KBI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void KBI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void ICS_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void WDG_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PWT_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
	
/* The Interrupt Vector Table */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    /* Processor exceptions */
    (void(*)(void)) &_estack,
    __thumb_startup,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* Interrupts */
	Default_Handler, 		/* Reserved interrupt 16/0 */
	Default_Handler, 		/* Reserved interrupt 17/1 */
	Default_Handler, 		/* Reserved interrupt 18/2 */
	Default_Handler, 		/* Reserved interrupt 19/3 */
	Default_Handler, 		/* Reserved interrupt 20/4 */
    FTMRE_IRQHandler, 		/* Command complete and error interrupt */
    PMC_IRQHandler, 		/* Low-voltage detect, low-voltage warning */
    IRQ_IRQHandler,			/* External interrupt */
    I2C0_IRQHandler,		/* Single interrupt vector for all sources */
    Default_Handler, 		/* Reserved interrupt 25/9 */
    SPI0_IRQHandler,		/* Single interrupt vector for all sources */
    Default_Handler,		/* Reserved interrupt 27/11 */
    UART0_SCI0_IRQHandler,  /* UART0 Status and Error interrupt */
    Default_Handler,		/* Reserved interrupt 29/13 */
	Default_Handler,		/* Reserved interrupt 30/14 */
    ADC0_IRQHandler,		/* ADC conversion complete interrupt */
    ACMP0_IRQHandler,		/* Analog comparator 0 interrupt */
    FTM0_IRQHandler,		/* Single interrupt vector for all sources */
	Default_Handler,		/* Reserved interrupt 34/18 */
    FTM2_IRQHandler,		/* Single interrupt vector for all sources */
	RTC_IRQHandler,			/* RTC overflow */
	ACMP1_IRQHandler,		/* Analog comparator 1 interrupt */
	PIT_CH0_IRQHandler,		/* PIT CH0 overflow */
	PIT_CH1_IRQHandler,		/* PIT CH1 overflow */
	KBI0_IRQHandler,		/* Keyboard interrupt0 */
	KBI1_IRQHandler,		/* Keyboard interrupt1 */
	Default_Handler,		/* Reserved interrupt 42/26 */
    ICS_IRQHandler,			/* Clock loss of lock Interrupt */
    WDG_IRQHandler,			/* WDG timeout interrupt */
    PWT_IRQHandler,			/* Single interrupt vector for all sources */
	Default_Handler,		/* Reserved interrupt 46/30 */
	Default_Handler,		/* Reserved interrupt 47/31 */
};
