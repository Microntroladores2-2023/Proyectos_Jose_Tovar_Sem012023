#include "modbus.h"
#include "adcs.h"

extern UINT16_VAL MBHoldingRegister[maxHoldingRegister];
extern UINT16_VAL MBInputRegister[maxInputRegister];
extern UINT16_VAL MBCoils;
extern UINT16_VAL MBDiscreteInputs;

void TareaEntradaDatos(void *Parametro)
{
     // Iteración valores ADCs

    uint32_t promedio1 = 0;
    uint32_t promedio2 = 0;
    uint32_t promedio3 = 0;

    uint32_t adc_value1 = adc1_get_raw(ADC1_CHANNEL_0);
    uint32_t adc_value2 = adc1_get_raw(ADC1_CHANNEL_3);
    uint32_t adc_value3 = adc1_get_raw(ADC1_CHANNEL_6);

        for (int j = 0; j < NumeroMuestras; j++)
        {
            promedio1 += adc_value1 = adc1_get_raw(ADC1_CHANNEL_0);
            promedio2 += adc_value2 = adc1_get_raw(ADC1_CHANNEL_1);
            promedio3 += adc_value3 = adc1_get_raw(ADC1_CHANNEL_2);
        }
        // Calculo Promedio de 100 muestras
        promedio1 /= NumeroMuestras;
        promedio2 /= NumeroMuestras;
        promedio3 /= NumeroMuestras;

        // Establezco límite superior de entrada
        if (promedio1 > 4095)
            promedio1 = 4095;
        if (promedio2 > 4095)
            promedio2 = 4095;
        if (promedio3 > 4095)
            promedio3 = 4095;

    // Escribo en el Input Register
        MBInputRegister[0].Val = promedio1;
        MBInputRegister[1].Val = promedio2;
        MBInputRegister[2].Val = promedio3;

        // printf("valor adc3 crudo: %d\n",adc_value3);
        // printf("valor adc3 suave: %d\n",promedio3);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
}