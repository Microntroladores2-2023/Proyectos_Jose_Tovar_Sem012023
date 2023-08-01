#include <Adafruit_BME280.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Wire.h>
#include <BH1750.h>
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "wifi_credentials.h"



// BME280 sensor object
Adafruit_BME280 bme;

//BH1750 sensor
BH1750 lightMeter(0x23);

//Se definen las clases necesarias
WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

//Parametros del Broker
char *server = "broker.emqx.io";
int port = 1883;

//Definicion de pines a utilizar
int ledpin= 26;


// Task handle
TaskHandle_t xHandle_readBMETask = NULL;
TaskHandle_t xHandle_receiveDataTask = NULL;
TaskHandle_t xHandle_readBH1750Task = NULL;
TaskHandle_t xHandle_receiveBHDataTask = NULL;

TaskHandle_t xHandle_mqtt_task = NULL;


//Handle de la Cola
QueueHandle_t xQueue;
QueueHandle_t dataQueue;
QueueHandle_t BHdataQueue;

//Variable global
uint16_t lux = 0;

// //Pines para I2C
// #define I2C_SDA  
// #define I2C_SCL  

// Data struct
struct BMEData {
  float temperature;
  float humidity;
  float pressure;
};

//Variable globales
int var = 0;
int ledval = 0;
int fotoval = 0;
char datos1[40];
char datos2[60];
String resultS = "";
long lastMsg = 0;

//Leer BH
void readBH1750Task(void *pvParameters){
  while(true){
    uint16_t lux = lightMeter.readLightLevel();//Realizamos una lectura del sensor

    xQueueSend(BHdataQueue, &lux, portMAX_DELAY);

    vTaskDelay(4800 / portTICK_PERIOD_MS);
  }
  
}

// void receiveBHDataTask(void *pvParameters){
//   while(true){
//     uint16_t dataBH;
//     xQueueReceive(BHdataQueue, &dataBH, portMAX_DELAY);

//     Serial.print("Luz(iluminancia):  ");
//     Serial.print(dataBH);
//     Serial.println(" lux");
//   }
// }

//Leer BME
void readBMETask(void *parameter) {
  while (true) {
    // Create a BMEData struct
    BMEData data1;

    // Read the BME sensor values
    data1.humidity = bme.readHumidity();
    data1.pressure = bme.readPressure();
    data1.temperature = bme.readTemperature();
    

    // Send the data to the queue
    xQueueSend(dataQueue, &data1, portMAX_DELAY);

    // Delay for 1 second
    vTaskDelay(4800 / portTICK_PERIOD_MS);
  }
}

// Task function
void receiveDataTask(void *parameter) {
  while (true) {
    // Receive the data from the queue
    BMEData data;
    xQueueReceive(dataQueue, &data, portMAX_DELAY);

    // Print the data
    Serial.print("Temperature: ");
    Serial.print(data.temperature);
    Serial.print(" *C\t");
    Serial.print("Humidity: ");
    Serial.print(data.humidity);
    Serial.print("%\t");
    Serial.print("Pressure: ");
    Serial.println(data.pressure);
  }
}

//---------------------MQTT----------------------------

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

void mqtt_task(void *pvParameters){
  uint16_t dataBH;
  BMEData dataBME;
  while(true){
    if (!mqttClient.connected()) {
    reconnect();
    }
    mqttClient.loop();

    xQueueReceive(BHdataQueue, &dataBH, portMAX_DELAY);
    Serial.print("Luz(iluminancia):  ");
    Serial.print(dataBH);
    Serial.println(" lux");
  
    xQueueReceive(dataQueue, &dataBME, portMAX_DELAY);
    Serial.print("Temperature: ");
    Serial.print(dataBME.temperature);
    Serial.print(" *C\t");
    Serial.print("Humidity: ");
    Serial.print(dataBME.humidity);
    Serial.print("%\t");
    Serial.print("Pressure: ");
    Serial.println(dataBME.pressure);

    //String recibido para modificar LED
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

    //fotoval = analogRead(fotopin);
    //Imprimo el valor en puerto serial
    // Serial.print("Sensor: ");
    // Serial.println(fotoval);

    sprintf(datos1, "Valor sensor de luminicidad: %d lux", dataBH);
    mqttClient.publish("SensorJT/01", datos1); //Se publican los datos al broker
    sprintf(datos2, "Temperatura: %f C  Presion: %f   Humedad: %f", dataBME.temperature, dataBME.pressure, dataBME.humidity);
    mqttClient.publish("SensorJT/02", datos2); //Se publican los datos al broker
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  
}


void setup() {

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

  // Create the queues
  dataQueue = xQueueCreate(3, sizeof(BMEData));
  BHdataQueue = xQueueCreate(1, sizeof(lux));

  // Initialize the BME280 sensor
  while(!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor!");
    while (1);
  }

  //Inicializacion de sensor
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  //Creacion de tareas
    xTaskCreatePinnedToCore(mqtt_task, "taskMQTT", 1024*2, NULL, 2, &xHandle_mqtt_task, 1);
    xTaskCreatePinnedToCore(readBMETask, "readBMETask", 1024*2, NULL, 2, &xHandle_readBMETask, 1);
    xTaskCreatePinnedToCore(readBH1750Task, "readBH1750Task", 1024*2, NULL, 2, &xHandle_readBH1750Task, 1);
  //xTaskCreatePinnedToCore(receiveDataTask, "receiveDataTask", 1024*2, NULL, 2, &xHandle_readBMETask, 1);
  //xTaskCreatePinnedToCore(receiveBHDataTask, "receiveBHDataTask", 1024*2, NULL, 2, &xHandle_receiveBHDataTask, 1);
}

void loop() {
  // Do nothing
}
