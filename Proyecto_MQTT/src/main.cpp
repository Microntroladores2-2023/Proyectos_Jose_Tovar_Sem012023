#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "wifi_credentials.h"

//Se definen las clases necesarias
WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

//Parametros del Broker
char *server = "broker.emqx.io";
int port = 1883;

//Definicion de pines a utilizar
int ledpin= 26;
int fotopin=33;

//Parametros de la Cola
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(Datos)
#define SENSOR_GPIO GPIO_NUM_33

//Handle de la Cola
QueueHandle_t xQueue;

//Handles de tareas
TaskHandle_t xHandle_mqtt_task = NULL;
//TaskHandle_t xHandle_adc_task = NULL;

//Variable globales
int var = 0;
int ledval = 0;
int fotoval = 0;
char datos[40];
String resultS = "";

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
    Serial.print("Intentando conectarse MQTT...");

    if (mqttClient.connect("arduinoClient")) {
      Serial.println("Conectado");

      mqttClient.subscribe("Entrada/01");
      
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
  while(true){
    if (!mqttClient.connected()) {
    reconnect();
    }
    mqttClient.loop();

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

    fotoval = analogRead(fotopin);
    //Imprimo el valor en puerto serial
    Serial.print("Sensor: ");
    Serial.println(fotoval);


    sprintf(datos, "Valor sensor: %d ", fotoval);
    mqttClient.publish("Salida/02",datos); //Se envian los datos al broker
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
  
}

void setup()
{
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

  //Creacion de tareas
  xTaskCreatePinnedToCore(mqtt_task, "taskMQTT", 1024*2, NULL, 2, &xHandle_mqtt_task, 1);
  //xTaskCreatePinnedToCore(SendHTTP, "EnvioHTTP", 1024*2, NULL, 6, &xHandle_http_task, 1);


}

void loop(){
}


/*
void loop()
{
   if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

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

  fotoval = analogRead(fotopin);
  Serial.print("Foto: ");
  Serial.println(fotoval);

  sprintf(datos, "Valor fotoresistencia: %d ", fotoval);
  mqttClient.publish("Salida/01",datos);
  delay(5000);
}
*/