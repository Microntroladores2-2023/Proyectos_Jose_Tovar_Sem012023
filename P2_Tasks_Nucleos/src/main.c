#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void Tarea1(void *pvParameters) //Tarea 1
{

    for (;;) //Loop infinito
    {
        printf("Hola Mundo Tarea1 freeRTOS Estoy corriendo en el nucluo = %d\n\r", xPortGetCoreID());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Tarea2(void *pvParameters) //Tarea 2
{

    for (;;) //Loop infinito
    {
        printf("Hola Mundo Tarea2 freeRTOS Estoy corriendo en el nucluo = %d\n\r", xPortGetCoreID());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Tarea3(void *pvParameters) //Tarea 3
{

    for (;;) //Loop infinito
    {
        printf("Hola Mundo Tarea3 freeRTOS Estoy corriendo en el nucluo = %d\n\r", xPortGetCoreID());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Tarea4(void *pvParameters) //Tarea 4
{

    for (;;) //Loop infinito
    {
        printf("Hola Mundo Tarea4 freeRTOS Estoy corriendo en el nucluo = %d\n\r", xPortGetCoreID());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    //El orden de la prioridad esta  ordenado de forma descendente, es decir
    //La prioridad mas alta coincide con el numero mas alto

    xTaskCreatePinnedToCore(Tarea1, "Task1", 1024 * 2, NULL, 1, NULL, 0); //Se ejecutara en nucleo 0, con prioridad 1
    xTaskCreatePinnedToCore(Tarea2, "Task2", 1024 * 2, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Tarea3, "Task3", 1024 * 2, NULL, 3, NULL, 0); //Esta tiene mayor prioridad que Task 1
    xTaskCreatePinnedToCore(Tarea4, "Task4", 1024 * 2, NULL, 4, NULL, 1);
}