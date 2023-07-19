#include <Arduino.h>
#include "adcs.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "freertos/queue.h"
/*
#include <Adafruit_BME280.h>
#include <Wire.h>

#define QUEUE_LENGTH 10
#define QUEUE_ITEM_SIZE sizeof(float)

QueueHandle_t sensorQueue;
*/

//Parametros de la Cola
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(Datos)
#define SENSOR_GPIO GPIO_NUM_33
//Handle de la Cola
QueueHandle_t xQueue;

//Se define estructura para datos del ADC
typedef struct
{
  int id;
  float valuef;
  uint32_t adcValue1;
  uint32_t adcValue2;
  uint32_t adcValue3;
} Datos;

//Definicion de Handles
TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_entrada_datos = NULL;

/*
void leer_sensor(void *pvParameters) {
  Adafruit_BME280 bme;
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    vTaskDelete(NULL);
  }

  float temperature, humidity, pressure;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;

    float sensorData[3] = {temperature, humidity, pressure};
    if (xQueueSend(sensorQueue, (void *)sensorData, (TickType_t)0) != pdTRUE) {
      Serial.println("Failed to send item to queue");
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}
*/

//Funcion para lectura y promedio del ADC, 3 canales
void EntradasADC(void *pvParameters)
{
  uint32_t promedio1 = 0;
  uint32_t promedio2 = 0;
  uint32_t promedio3 = 0;

  uint32_t adc_value1 = adc1_get_raw(CH1);
  uint32_t adc_value2 = adc1_get_raw(CH2);
  uint32_t adc_value3 = adc1_get_raw(CH3);

  while (1)
  {
    // Iteración valores ADCs

    Serial.println("Se comienza lectura de los ADC");

    for (int j = 0; j < NumeroMuestras; j++)
    {

      promedio1 += adc_value1 = adc1_get_raw(CH1);
      promedio2 += adc_value2 = adc1_get_raw(CH2);
      promedio3 += adc_value3 = adc1_get_raw(CH3);
    }
    // Calculo Promedio de 100 muestras
    promedio1 /= NumeroMuestras;
    promedio2 /= NumeroMuestras;
    promedio3 /= NumeroMuestras;

    //Límite superior de entrada
    if (promedio1 > 4095)
      promedio1 = 4095;
    if (promedio2 > 4095)
      promedio2 = 4095;
    if (promedio3 > 4095)
      promedio3 = 4095;

    //Se define estructura de datos a transmitir
    Datos datosTx;
    datosTx.id = 1;
    datosTx.adcValue1 = promedio1;
    datosTx.adcValue2 = promedio2;
    datosTx.adcValue3 = promedio3;

    int hola = xQueueSend(xQueue, &datosTx, portMAX_DELAY);
    if(hola == pdTRUE){
      Serial.println("Se envio la cola de forma exitosa");
    }
    else{
      Serial.println("No envio la cola");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void SendHTTP(void *pvParameters) {

  // int32_t receivedItem;
  //Se define estructura de tipo Datos para la cola a recibir
  Datos datosRx;

  //URL del Scada Vemetris unico
  String ScadaVemetris = "http://137.184.178.17:21486/httpds?__device=JT_micros2";
  
  while (true) {
    if (xQueueReceive(xQueue, &datosRx, portMAX_DELAY) == pdTRUE)
    {
      HTTPClient http;

      String dato1 = String(datosRx.adcValue1);

      String dato2 = String(datosRx.adcValue2);

      String dato3 = String(datosRx.adcValue3);

      String queryString = ScadaVemetris + "&rssi=" + WiFi.RSSI() + "&dato1=" + dato1 + "&dato2=" + dato2 + "&dato3=" + dato3;

      //Iniciar conexion
      http.begin(queryString);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        String payload = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(payload);           
      }
      else
      {
        Serial.println("Error enviando la trama");
      }
      http.end(); // Se libera el cliente
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void initWiFi(void)
{
  const char *network = "JET Home";
  const char *password = "chemundo300665";
  WiFi.begin(network, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Conectando a red wifi...");
    Serial.println(WiFi.status());
  }
}


void setup() 
{
  init_adc();
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  initWiFi();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  //Creacion de la cola
  xQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
  //sensorQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE * 3);

  //Definicion de tareas
  if (xQueue != NULL)
  {
    xTaskCreatePinnedToCore(EntradasADC, "EntradaADC1", 1024*2, NULL, 2, &xHandle_entrada_datos, 1);
    //  xTaskCreate(leer_sensor, "Leer Sensor", 2048, NULL, 1, NULL);
    //xTaskCreatePinnedToCore(EntradasFloat, "Entrada de datos float", 1024*2, NULL, 4, &xHandle_entrada_datos, 1);

    xTaskCreatePinnedToCore(SendHTTP, "EnvioHTTP", 1024*2, NULL, 6, &xHandle_http_task, 1);
  }
  else{
    Serial.println("Tareas no fueron creadas exitosamente");
  }
}

void loop() {
}