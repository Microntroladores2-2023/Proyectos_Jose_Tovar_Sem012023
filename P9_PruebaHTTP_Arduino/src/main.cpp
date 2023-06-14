#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ssid";
const char* password = "pass";
const char* botToken = "token";
const char* chatId = "cahtid";

const int buttonPin = 0;

void telegramTask(void *pvParameters) {
  while (1) {
    vTaskSuspend(NULL); // Suspender la tarea hasta que se presione el botón

    String message = "Hello, Telegram!";
    String url = "https://api.telegram.org/bot" + String(botToken) + "/sendMessage?chat_id=" + String(chatId) + "&text=" + message;

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP response code: %d\n", httpCode);
      String response = http.getString();
      Serial.println("Server response: " + response);
    } else {
      Serial.println("HTTP request failed.");
    }

    http.end();

    vTaskDelay(pdMS_TO_TICKS(3000)); // Esperar 3 segundos antes de enviar otra solicitud
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");

  xTaskCreate(telegramTask, "telegramTask", 4096, NULL, 1, NULL); // Crear tarea de FreeRTOS
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    vTaskResume(NULL); // Reanudar la tarea cuando se presione el botón
  }
}