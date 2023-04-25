#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"

static QueueHandle_t uart0_queue;

TaskHandle_t xHandle; //Handle para manejar la tarea del led

uint16_t tiempo_led = 500; // variable global

#define tamBUFFER 1024 //tamaño del buffer

//Tareas

void TareaEventosUART0(void *Parametro)
{
    uart_event_t evento; //estructura del tipo enum

    uint8_t *datoRX = (uint8_t *)malloc(tamBUFFER); //creacion de buffer dinamico, es decir, adopta el tamaño necesario

   while(1)
    {
        //Si recibo datos por puerto serial
        if (xQueueReceive(uart0_queue, (void *)&evento, (TickType_t)portMAX_DELAY))
        {
            bzero(datoRX, tamBUFFER);
            if (evento.type == UART_DATA)
            {
                uart_read_bytes(UART_NUM_0, datoRX, evento.size, portMAX_DELAY);
                protocolo1Serial(datoRX, evento.size); //Le paso el dato recibido por serial al protocolo

                // uart_write_bytes(UART_NUM_0, (const char*) datoRX, evento.size);

                // vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
    }

    free(datoRX); //Se limpia el buffer
    datoRX = NULL;
    vTaskDelete(NULL); //Se elimina la tarea al terminar su función
}

//Inicializacion de UART

void initUART0()
{
    uart_config_t configUART0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_NUM_0, &configUART0);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, tamBUFFER * 2, tamBUFFER * 2, 20, &uart0_queue, 0);

    xTaskCreatePinnedToCore(TareaEventosUART0, "Tarea_para_UART0", 1024 * 5, NULL, 12, NULL, 1);
}

void blink(void *pvParameters) // Esta es una tarea
{

#define LED_PIN 2

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int ON = 0;

    while (true) // Una tarea nunca regresará ni saldrá
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(tiempo_led / portTICK_PERIOD_MS);
    }
}

//Protocolo Serial creado por usuario

void protocolo1Serial(uint8_t *ByteArray, uint16_t Length)
{

    //uart_write_bytes(UART_NUM_0, (const char*) ByteArray, Length);

    uint8_t estado = ByteArray[0]; //Se va al estado asignado por el mensaje recibido por puerto serial

    switch (estado)
    {
    case 0:
        tiempo_led = 1000; //Se ajusta el tiempo del led
        break;

    case 1:
        tiempo_led = 2000; //Se ajusta el tiempo del led
        break;
    case 2:
        vTaskSuspend(xHandle); //Suspende la tarea de parpadeo de led
        //tiempo_led = 3000; //Se ajusta el tiempo del led
        break;

    case 3:
         vTaskResume(xHandle);
        tiempo_led = 5000; //Se ajusta el tiempo del led
        break;
    }
}

void app_main(void)
{
    initUART0();
    xTaskCreatePinnedToCore(blink, "Blink", 1024 * 2, NULL, 1, &xHandle, 0); //Tarea con manejador
}