/**
 * @file ADC.c
 * @brief Implementation of the AVR ADC peripheral driver.
 */
#include "ADC.h"
#include "../../TOOLS.h"
#include "avr/interrupt.h"
/** @brief Application callback invoked after a completed conversion. */
static ADC_Callback_t   adc_interrupt;

/** @brief Handle ADC completion and invoke the registered callback, if any. */
ISR(ADC_vect){
 if(adc_interrupt != 0){
    adc_interrupt();
 }
}
/** @copydoc ADC_init */
void ADC_init(ADC_refrence_t refrence ,
              ADC_INPUT_channel_t input ,
              ADC_result_adj_t adjust ,
              ADC_prescaler_t prescale ,
              ADC_auto_trigger_t trigger
            )
{
    /* Enum values are encoded for direct insertion into ADMUX. */
    ADMUX  = refrence | adjust | input ;
    ADCSRA = prescale;
    if(trigger != ADC_NO_AUTO_TRIGGER){
        /* SFIOR bits 7:5 choose the requested hardware trigger source. */
        SET_BIT(ADCSRA,ADATE);
        SFIOR &=~(0b11100000);
        SFIOR |= trigger;
    }
    SET_BIT(ADCSRA,ADEN);
}


/** @copydoc ADC_set_Channel */
void ADC_set_Channel (ADC_INPUT_channel_t input){
    /* Preserve the reference and alignment fields while replacing MUX. */
    ADMUX &=~(0b00011111);
    ADMUX |= input;
}

/** @copydoc ADC_start_conversion */
void ADC_start_conversion (){
    SET_BIT(ADCSRA,ADSC);
}
/** @copydoc ADC_interrupt_init */
void ADC_interrupt_init (ADC_Callback_t callback){
    adc_interrupt = callback;
}
/** @copydoc ADC_interrupt_enble */
void ADC_interrupt_enble(){
    SET_BIT(ADCSRA,ADIE);
}
/** @copydoc ADC_interrupt_Disable */
void ADC_interrupt_Disable(){
    CLEAR_BIT(ADCSRA,ADIE);
}
/** @copydoc ADC_read_8bit */
uint8_t ADC_read_8bit(){
    /** Poll conversion activity; see the free-running limitation in the API. */
    while(READ_BIT(ADCSRA,ADSC));
    switch(READ_BIT(ADMUX,ADLAR)){
        case 0:
        /*
        for example 
        ADCH  --> XXXX XX11
        ADCL  --> 1010 1010   the last 10 we will discard 
           (ADCL>>2) =-->  0010 1010     (result1)
           we will put the adch in those higher two bits we cleared
            ADCH & 0b0000 0011 --> 0000 0011
            ADCH<<6 -->   1100 0000      (result2)
          (result1) | (result2) --> 1110 1010
        */
        return ((ADCL >> 2)| (( ADCH & 0b00000011)<<6));
        break;
        case 1 :
        return ADCH;
        break;
    }
    return (uint8_t)ADC;
}
/** @copydoc ADC_read_10bit */
uint16_t ADC_read_10bit(){
      /** Poll conversion activity; see the free-running limitation in the API. */
    while(READ_BIT(ADCSRA,ADSC));
    switch(READ_BIT(ADMUX,ADLAR)){
        case 0:
        return ADCL | (( ADCH & 0b00000011)<<8);
        break;
        case 1 :
        return (ADCH<<2)|( ADCL & 0b00000011);
        break;
    }
    return (uint8_t)ADC;
}
