#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t xHandle; 

uint16_t tiempo_Led = 1000; //Valor inicial global del tiempo de led

void Boton(void *pvParameters)
{
#define PUSH_BUTTON_PIN 0

    uint8_t estado_maquina = 0;

    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

    while (1)
    {
        //Lectura del estado del boton
        while (gpio_get_level(PUSH_BUTTON_PIN) == 0)
            vTaskDelay(10/portTICK_PERIOD_MS); // Boton presionado

        while (gpio_get_level(PUSH_BUTTON_PIN) == 1)
           vTaskDelay(10/portTICK_PERIOD_MS); // Boton sin presionar

        //--------------MAQUINA DE ESTADOS-----------------------------
        switch (estado_maquina)
        {
        case 0:
            tiempo_Led = 100;
            estado_maquina = 1;
            printf("Estado actual: 1 \n");
            break;

        case 1:
            tiempo_Led = 2000;
            estado_maquina = 2;
            printf("Estado actual: 2 \n");
            break;

        case 2:
            vTaskSuspend(xHandle);
            estado_maquina = 3;
            printf("Estado actual: 3 \n");
            break;

        case 3:
            vTaskResume(xHandle);
            tiempo_Led = 50;
            estado_maquina = 4;
            printf("Estado actual: 4 \n");
            break;

        case 4:
            tiempo_Led = 500;
            estado_maquina = 0;
            printf("Estado actual: 5 \n");
            break;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void Blink(void *pvParameters) // Esta es una tarea
{

#define LED_PIN 2

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int ON = 0; //Estado inicial de la salida

    while (true) //Loop infinito
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(tiempo_Led / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{

    xTaskCreatePinnedToCore(Boton, "Boton", 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Blink, "Blink", 1024 * 2, NULL, 1, &xHandle, 0); //Se le pasa el handle a esta tarea
}
