#include <SI_EFM8UB1_Register_Enums.h>
#include "efm8ub10adc.h"
#include "usb_main.h"

xdata uint16_t adcresult[ ADC_CHANNELS ];
xdata uint8_t chn_def_P[ ADC_CHANNELS ] = { 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
char *padcresult;
char buffer_num = 0;

int TempAdc, tmp;
char num = 0, add = 0, loop;
char getpos = 0;

//================================================================================
// ADC_0_enter_DefaultMode_from_RESET
//================================================================================
extern void ADC_0_enter_DefaultMode_from_RESET(void)
{
    // $[ADC0CN1 - ADC0 Control 1]
    /***********************************************************************
     - Disable the common mode buffer
     ***********************************************************************/
    ADC0CN1 = ADC0CN1_ADCMBE__CM_BUFFER_DISABLED;
    // [ADC0CN1 - ADC0 Control 1]$

    // $[ADC0MX - ADC0 Multiplexer Selection]
    /***********************************************************************
     - Select ADC0.15
     ***********************************************************************/
    ADC0MX = ADC0MX_ADC0MX__ADC0P15;
    // [ADC0MX - ADC0 Multiplexer Selection]$

    // $[ADC0CF - ADC0 Configuration]
    /***********************************************************************
     - SAR Clock Divider = 0x03
     - ADC0 operates in 10-bit or 12-bit mode
     - The on-chip PGA gain is 1
     - Normal Track Mode
     ***********************************************************************/
    ADC0CF = (0x03 << ADC0CF_ADSC__SHIFT) | ADC0CF_AD8BE__NORMAL
            | ADC0CF_ADGN__GAIN_1 | ADC0CF_ADTM__TRACK_DELAYED;
    // [ADC0CF - ADC0 Configuration]$

    // $[ADC0AC - ADC0 Accumulator Configuration]
    /***********************************************************************
     - Right justified. No shifting applied
     - Enable 12-bit mode
     - ADC0H:ADC0L contain the result of the latest conversion when Burst
     Mode is disabled
     - Perform and Accumulate 8 conversions
     ***********************************************************************/
    ADC0AC = ADC0AC_ADSJST__RIGHT_NO_SHIFT | ADC0AC_AD12BE__12_BIT_ENABLED
            | ADC0AC_ADAE__ACC_DISABLED | ADC0AC_ADRPT__ACC_32;
    // [ADC0AC - ADC0 Accumulator Configuration]$

    // $[ADC0TK - ADC0 Burst Mode Track Time]
    /***********************************************************************
     - The ADC will sample the input once at the beginning of each 12-bit
     conversion
     - Burst Mode Tracking Time = 0x1E
     ***********************************************************************/
    ADC0TK = ADC0TK_AD12SM__SAMPLE_ONCE | (0x1E << ADC0TK_ADTK__SHIFT);
    // [ADC0TK - ADC0 Burst Mode Track Time]$

    // $[ADC0PWR - ADC0 Power Control]
    /***********************************************************************
     - Burst Mode Power Up Time = 0x0F
     - Disable low power mode
     - Low power mode disabled
     - Select bias current mode 1
     ***********************************************************************/
    ADC0PWR = (0x0F << ADC0PWR_ADPWR__SHIFT) | ADC0PWR_ADLPM__LP_BUFFER_DISABLED
            | ADC0PWR_ADMXLP__LP_MUX_VREF_DISABLED | ADC0PWR_ADBIAS__MODE1;
    // [ADC0PWR - ADC0 Power Control]$

    // $[ADC0GTH - ADC0 Greater-Than High Byte]
    // [ADC0GTH - ADC0 Greater-Than High Byte]$

    // $[ADC0GTL - ADC0 Greater-Than Low Byte]
    // [ADC0GTL - ADC0 Greater-Than Low Byte]$

    // $[ADC0LTH - ADC0 Less-Than High Byte]
    // [ADC0LTH - ADC0 Less-Than High Byte]$

    // $[ADC0LTL - ADC0 Less-Than Low Byte]
    // [ADC0LTL - ADC0 Less-Than Low Byte]$

    // $[ADC0CN0 - ADC0 Control 0]
    /***********************************************************************
     - Enable ADC0
     - Enable ADC0 burst mode
     - ADC0 conversion initiated on overflow of Timer 2
     ***********************************************************************/
    ADC0CN0 &= ~ADC0CN0_ADCM__FMASK;
    ADC0CN0 |= ADC0CN0_ADEN__ENABLED | ADC0CN0_ADBMEN__BURST_ENABLED
            | ADC0CN0_ADCM__TIMER2;
    // [ADC0CN0 - ADC0 Control 0]$
  }

//================================================================================
// VREF_0_enter_DefaultMode_from_RESET
//================================================================================
extern void VREF_0_enter_DefaultMode_from_RESET(void) {
    // $[REF0CN - Voltage Reference Control]
    /***********************************************************************
     - Enable the Temperature Sensor
     - The ADC0 ground reference is the GND pin
     - The internal reference operates at 1.65 V nominal
     - The ADC0 voltage reference is the VDD pin
     ***********************************************************************/
    REF0CN = REF0CN_TEMPE__TEMP_ENABLED | REF0CN_GNDSL__GND_PIN
            | REF0CN_IREFLVL__2P4 | REF0CN_REFSL__VDD_PIN;
    // [REF0CN - Voltage Reference Control]$
}

//================================================================================
// INTERRUPT_0_enter_DefaultMode_from_RESET
//================================================================================
extern void INTERRUPT_0_enter_DefaultMode_from_RESET(void) {
    // $[EIE1 - Extended Interrupt Enable 1]
    /*
    // EADC0 (ADC0 Conversion Complete Interrupt Enable) = ENABLED (Enable
    //     interrupt requests generated by the ADINT flag.)
    // EWADC0 (ADC0 Window Comparison Interrupt Enable) = DISABLED (Disable
    //     ADC0 Window Comparison interrupt.)
    // ECP0 (Comparator0 (CP0) Interrupt Enable) = DISABLED (Disable CP0
    //     interrupts.)
    // ECP1 (Comparator1 (CP1) Interrupt Enable) = DISABLED (Disable CP1
    //     interrupts.)
    // EMAT (Port Match Interrupts Enable) = DISABLED (Disable all Port Match
    //     interrupts.)
    // EPCA0 (Programmable Counter Array (PCA0) Interrupt Enable) = DISABLED
    //     (Disable all PCA0 interrupts.)
    // ESMB0 (SMBus (SMB0) Interrupt Enable) = DISABLED (Disable all SMB0
    //     interrupts.)
    // ET3 (Timer 3 Interrupt Enable) = DISABLED (Disable Timer 3
    //     interrupts.)
    */
    EIE1 = EIE1_EADC0__ENABLED | EIE1_EWADC0__DISABLED | EIE1_ECP0__DISABLED
         | EIE1_ECP1__DISABLED | EIE1_EMAT__DISABLED | EIE1_EPCA0__DISABLED
         | EIE1_ESMB0__DISABLED | EIE1_ET3__DISABLED;
    // [EIE1 - Extended Interrupt Enable 1]$

    // $[EIE2 - Extended Interrupt Enable 2]
    // [EIE2 - Extended Interrupt Enable 2]$

    // $[EIP1H - Extended Interrupt Priority 1 High]
    // [EIP1H - Extended Interrupt Priority 1 High]$

    // $[EIP1 - Extended Interrupt Priority 1 Low]
    // [EIP1 - Extended Interrupt Priority 1 Low]$

    // $[EIP2 - Extended Interrupt Priority 2]
    // [EIP2 - Extended Interrupt Priority 2]$

    // $[EIP2H - Extended Interrupt Priority 2 High]
    // [EIP2H - Extended Interrupt Priority 2 High]$

    // $[IE - Interrupt Enable]
    /*
    // EA (All Interrupts Enable) = ENABLED (Enable each interrupt according
    //     to its individual mask setting.)
    // EX0 (External Interrupt 0 Enable) = DISABLED (Disable external
    //     interrupt 0.)
    // EX1 (External Interrupt 1 Enable) = DISABLED (Disable external
    //     interrupt 1.)
    // ESPI0 (SPI0 Interrupt Enable) = DISABLED (Disable all SPI0
    //     interrupts.)
    // ET0 (Timer 0 Interrupt Enable) = DISABLED (Disable all Timer 0
    //     interrupt.)
    // ET1 (Timer 1 Interrupt Enable) = DISABLED (Disable all Timer 1
    //     interrupt.)
    // ET2 (Timer 2 Interrupt Enable) = DISABLED (Disable Timer 2 interrupt.)
    // ES0 (UART0 Interrupt Enable) = DISABLED (Disable UART0 interrupt.)
    */
    IE = IE_EA__ENABLED | IE_EX0__DISABLED | IE_EX1__DISABLED | IE_ESPI0__DISABLED
         | IE_ET0__DISABLED | IE_ET1__DISABLED | IE_ET2__DISABLED | IE_ES0__DISABLED;
    // [IE - Interrupt Enable]$

    // $[IP - Interrupt Priority]
    // [IP - Interrupt Priority]$

    // $[IPH - Interrupt Priority High]
    // [IPH - Interrupt Priority High]$


}

//================================================================================
// TIMER16_2_enter_DefaultMode_from_RESET
//================================================================================
extern void TIMER16_2_enter_DefaultMode_from_RESET(void) {
    // $[Timer Initialization]
    // Save Timer Configuration
    uint8_t TMR2CN0_TR2_save;
    TMR2CN0_TR2_save = TMR2CN0 & TMR2CN0_TR2__BMASK;
    // Stop Timer
    TMR2CN0 &= ~(TMR2CN0_TR2__BMASK);
    // [Timer Initialization]$

    // $[TMR2CN1 - Timer 2 Control 1]
    // [TMR2CN1 - Timer 2 Control 1]$

    // $[TMR2CN0 - Timer 2 Control]
    // [TMR2CN0 - Timer 2 Control]$

    // $[TMR2H - Timer 2 High Byte]
    // [TMR2H - Timer 2 High Byte]$

    // $[TMR2L - Timer 2 Low Byte]
    /***********************************************************************
     - Timer 2 Low Byte = 0x0F
     ***********************************************************************/
    TMR2L = (0x0F << TMR2L_TMR2L__SHIFT);
    // [TMR2L - Timer 2 Low Byte]$

    // $[TMR2RLH - Timer 2 Reload High Byte]
    /***********************************************************************
     - Timer 2 Reload High Byte = 0xFF
     ***********************************************************************/
    TMR2RLH = (0xFF << TMR2RLH_TMR2RLH__SHIFT);
    // [TMR2RLH - Timer 2 Reload High Byte]$

    // $[TMR2RLL - Timer 2 Reload Low Byte]
    /***********************************************************************
     - Timer 2 Reload Low Byte = 0x6A
     ***********************************************************************/
    TMR2RLL = (0x6A << TMR2RLL_TMR2RLL__SHIFT);
    // [TMR2RLL - Timer 2 Reload Low Byte]$

    // $[TMR2CN0]
    /***********************************************************************
     - Start Timer 2 running
     ***********************************************************************/
    TMR2CN0 |= TMR2CN0_TR2__RUN;
    // [TMR2CN0]$

    // $[Timer Restoration]
    // Restore Timer Configuration
    TMR2CN0 |= TMR2CN0_TR2_save;
    // [Timer Restoration]$
}
SI_SBIT(LED, SFR_P3, 0);               // P1.4 LED
SI_INTERRUPT(ADC0EOC_ISR, ADC0EOC_IRQn)
{
   static uint16_t *padcresult = (uint16_t *)&usb_txBuf[1];
   static uint8_t measurementsch = 0;
   static uint16_t delay = 0;
   //uint8_t SFRPAGE_save;

   ADC0CN0_ADINT = 0;              // Clear ADC0 conv. complete flag

   padcresult[measurementsch] = ADC0;
   //SFRPAGE_save = SFRPAGE;
   //   SFRPAGE = LEGACY_PAGE;
   //   SFRPAGE = SFRPAGE_save;
   measurementsch++;
   if(measurementsch > ADC_CHANNELS)
     measurementsch = 0;

   ADC0MX = chn_def_P[measurementsch];

   delay++;
   if( delay > 200 )
   {
       delay = 0;
       LED = 1;
   }
}
