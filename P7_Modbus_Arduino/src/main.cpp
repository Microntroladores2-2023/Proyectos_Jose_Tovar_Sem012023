#include <Arduino.h>
#include "uarts.h"
#include "tareas.h"
#include "adcs.h"
#include "init_coils.h"

void setup()
{
  initUART0();
  init_coils();
  init_adc();
  xTaskCreatePinnedToCore(TareaEntradaDatos, "Tarea_para_entrada1", 1024 * 2, NULL, 12, NULL, 1);
}

void loop()
{
}