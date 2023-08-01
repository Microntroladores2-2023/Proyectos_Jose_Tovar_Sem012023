#include "adcs.h"

//Inicializacion del ADC
void init_adc()
{
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
    adc1_channel_t channel = ADC1_CHANNEL_5;
    adc_bits_width_t adcWidth = ADC_WIDTH_BIT_12;
}