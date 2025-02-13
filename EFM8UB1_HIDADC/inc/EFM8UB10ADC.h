#ifndef EFM8UA10ADC_H
#define EFM8UA10ADC_H

#define ADC_CHANNELS    20

extern xdata uint16_t adcresult[ ADC_CHANNELS ];
extern xdata uint8_t chn_def_P[ ADC_CHANNELS ];
//extern xdata uint8_t chn_def_N[ ADC_CHANNELS ];

extern void VREF_0_enter_DefaultMode_from_RESET(void);
extern void ADC_0_enter_DefaultMode_from_RESET(void);
extern void INTERRUPT_0_enter_DefaultMode_from_RESET(void);
extern void TIMER16_2_enter_DefaultMode_from_RESET(void);

#endif

