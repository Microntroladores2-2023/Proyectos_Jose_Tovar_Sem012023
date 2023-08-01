#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "wifi_credentials.h"
#include "adcs.h"


//Se definen las clases necesarias
WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

//Parametros del Broker
char *server = "broker.emqx.io";
int port = 1883;

//Definicion de pines a utilizar
int ledpin= 26;
int sensor=33;

//Numero de muestras del ADC
#define SAMPLES_PERIOD 250
#define SAMPLES_NUMBER 64

//Parametros de la Cola
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(Datos)
#define SENSOR_GPIO GPIO_NUM_33

//Handle de la Cola
QueueHandle_t xQueue;
QueueHandle_t adcQueue;


//Handles de tareas
TaskHandle_t xHandle_mqtt_task = NULL;
TaskHandle_t xHandle_adc_task = NULL;

//Variable globales
int var = 0;
int ledval = 0;
int fotoval = 0;
char datos[40];
String resultS = "";
long lastMsg = 0;

void wifiInit() {
    Serial.print("Conectándose a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
        vTaskDelay(500 / portTICK_PERIOD_MS);  
    }
    Serial.println("");
    Serial.println("Conectado a WiFi");
    Serial.println("Dirección IP: ");
    Serial.println(WiFi.localIP());
  }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  char payload_string[length + 1];
  
  int resultI;

  memcpy(payload_string, payload, length);
  payload_string[length] = '\0';
  resultI = atoi(payload_string); //De enteros a flotante
  
  var = resultI;

  resultS = "";
  
  for (int i=0;i<length;i++) {
    resultS= resultS + (char)payload[i];
  }
  Serial.println();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conectarse a broker MQTT...");

    if (mqttClient.connect("arduinoClient")) {
      Serial.println("Conectado");

      //Se suscribe el topico al broker para poder recibir datos del mismo
      mqttClient.subscribe("EntradaJT/01");
      
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Intentar de nuevo en 5 segundos");
      // Wait 5 seconds before retrying
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

//Funcion para lectura y promedio del ADC, 3 canales
void readAdcTask(void *parameter) {
  // Initialize the variables
  int adcValue;
  int samplesNumber = 10;

  // Create a queue to store the mean values
  //adcQueue = xQueueCreate(samplesNumber, sizeof(float));
  while(true){
    float mean = 0;
    // Read the ADC value and calculate the mean
    for (int i = 0; i < samplesNumber; i++) {
      adcValue = analogRead(33);
      mean += adcValue;
    }
    mean /= samplesNumber;

    // Send the mean value to the queue
    xQueueSend(adcQueue, &mean, portMAX_DELAY);
  }
}

// void readAdcTask(void *pvParameters) {
//   init_adc();

//   uint32_t muestras[SAMPLES_NUMBER], promedio;
//   int i = 0;
//   int adcValue = adc1_get_raw(ADC1_CHANNEL_5); // Leer el valor del ADC1 del canal 0

//   for (int j = 0; j < SAMPLES_NUMBER; j++)
//         muestras[j] = adcValue;

//   while (true) {
//     adcValue = adc1_get_raw(ADC1_CHANNEL_5); // Leer el valor del ADC1 del canal 0
//     //printf("ADC Value raw: %d \n", adcValue); //Se imprime valor obtenido
//     muestras[i++] = adcValue;
//     promedio = 0;
//     for (int j = 0; j < SAMPLES_NUMBER; j++)
//       promedio = promedio + muestras[j] / SAMPLES_NUMBER;
//     xQueueSend(adcQueue, &promedio, portMAX_DELAY);
//     if (i >= SAMPLES_NUMBER)
//       i = 0;
    
//     //vTaskDelay(10000 / portTICK_PERIOD_MS);
//   }
// }

void mqtt_task(void *pvParameters){
  while(true){
    if (!mqttClient.connected()) {
    reconnect();
    }
    mqttClient.loop();

    long now = millis();
    uint32_t data;
    if (now - lastMsg > 1000 && uxQueueMessagesWaiting(adcQueue) > 0) {

      xQueueReceive(adcQueue, &data, portMAX_DELAY); //Se recibe cola del ADC
      sprintf(datos, "Valor sensor: %d ", data);
      mqttClient.publish("SensorJT/01", datos); //Se publican los datos al broker
    }

    Serial.print("Sensor: ");
    Serial.println(data);

    Serial.print("String: ");
    Serial.println(resultS);

    if(var == 0)
    {
    digitalWrite(ledpin,LOW);
    }
    else if (var == 1)
    {
    digitalWrite(ledpin,HIGH);
    }


    // fotoval = analogRead(fotopin);
    // //Imprimo el valor en puerto serial
    // Serial.print("Sensor: ");
    // Serial.println(fotoval);


    // sprintf(datos, "Valor sensor: %d ", fotoval);
    // mqttClient.publish("SensorJT/01", datos); //Se publican los datos al broker
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  
}

void setup()
{
  init_adc();

  pinMode(ledpin,OUTPUT);
  //se configura puerto serial
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  //Se inicializa WiFi
  wifiInit();

  //Se inicializa comunicacion MQTT y se defina callback
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);

  adcQueue = xQueueCreate(10, sizeof(uint32_t));

  while (adcQueue == NULL) {
    vTaskDelay(pdMS_TO_TICKS(200));
    Serial.println("esperando adc Queue");
  }

  //Creacion de tareas
  
  xTaskCreatePinnedToCore(readAdcTask, "Adc Reader", configMINIMAL_STACK_SIZE * 8, NULL, 3, &xHandle_adc_task, 1);
  xTaskCreatePinnedToCore(mqtt_task, "taskMQTT", 1024*2, NULL, 2, &xHandle_mqtt_task, 1);
}

void loop(){
}