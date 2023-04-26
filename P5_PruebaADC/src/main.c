#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t adc1_chars;

void TareaADC(void *pvParameters) // Esta es una tarea
{
#define numeroMuestras 64

    //Se fija la atenuacion y voltaje de referencia
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

    int muestras[numeroMuestras], promedio;
    int i = 0;

    int adc_value = adc1_get_raw(ADC1_CHANNEL_4); //Se toma medicion de valor en canal 4
    for (int j = 0; j < numeroMuestras; j++)
        muestras[j] = adc_value;
    
    printf("Voy a entrar al loop infinito \n");

    uint32_t voltage;

    //Loop infnito de la tarea
    while (1) 
    {
        adc_value = adc1_get_raw(ADC1_CHANNEL_4);

        printf("ADC Value raw: %d \n", adc_value); //Se imprime valor obtenido
        muestras[i++] = adc_value;
        promedio = 0;

        for (int j = 0; j < numeroMuestras; j++)
            promedio = promedio + muestras[j]; //Se calcula promedio con muestras obtenidas

        printf("ADC Value promedio: %d \n", promedio / numeroMuestras);

        if (i >= numeroMuestras)
            i = 0;

        voltage = esp_adc_cal_raw_to_voltage(adc_value, &adc1_chars); //Se convierte a voltaje
        printf("Voltage: %d mV",(int)voltage);
        printf("\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
void app_main() {

 xTaskCreatePinnedToCore(TareaADC, "TareaADC", 1024 * 10, NULL, 1, NULL, 0);

}