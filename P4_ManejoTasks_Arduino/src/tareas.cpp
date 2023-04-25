#include "tareas.h"

//En este archivo esta la mayor parte del programa

extern TaskHandle_t xHandle; //Se usa el atributo extern, usando algo que esta en otro modulo

uint16_t tiempo_Led = 1000; //Variable global para el control del parpadeo

void Boton(void *pvParameters)
{

#define PUSH_BUTTON_PIN (gpio_num_t)0

//enum (estructura) para asignar distintos estados por su nombre

  enum ESTADOS{
    velocidad_lenta,
    velocidad_rapida,
    suspensio_BLINK,
    resumen_BLINK,
    velocidad_media
  };

  uint8_t estado_maquina = velocidad_lenta; //Estado inicial de la maquina

  gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);

  while (1)
  {
    // presiona boton y suelta
    while (gpio_get_level(PUSH_BUTTON_PIN) == 0)
      vTaskDelay(10 / portTICK_PERIOD_MS); // boton presionado

    while (gpio_get_level(PUSH_BUTTON_PIN) == 1)
      vTaskDelay(10 / portTICK_PERIOD_MS); // boton sin presionar

    switch (estado_maquina)
    {
    case velocidad_rapida:
      tiempo_Led = 100;
      estado_maquina = velocidad_lenta;
      break;

    case velocidad_lenta:
      tiempo_Led = 2000;
      estado_maquina = suspensio_BLINK;
      break;

    case suspensio_BLINK:
      vTaskSuspend(xHandle);
      estado_maquina = resumen_BLINK;
      break;

    case resumen_BLINK:
      vTaskResume(xHandle);
      tiempo_Led = 50;
      estado_maquina = velocidad_media;
      break;

    case velocidad_media:
      tiempo_Led = 500;
      estado_maquina = velocidad_rapida;
      break;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void Blink(void *pvParameters)
{

#define LED_PIN (gpio_num_t)2

  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  int ON = 0;

  while (true) //Loop infinito
  {
    ON = !ON;
    gpio_set_level(LED_PIN, ON);
    vTaskDelay(tiempo_Led / portTICK_PERIOD_MS);
  }
}
