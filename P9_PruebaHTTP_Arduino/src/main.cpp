#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "JET Home";
const char* password = "chemundo300665";
const char* botToken = "5992410997:AAGNhUo8I5K-MnNsfuT3mrWa-RaVl4RFmd0";
const char* chatId = "1467611816";

const int buttonPin = 0;
const int debounceDelay = 50; // Debounce delay in milliseconds

TaskHandle_t telegramTaskHandle;

void telegramTask(void *pvParameters) {
  while (1) {
    vTaskSuspend(NULL); // Suspender la tarea hasta que se reactive

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

  xTaskCreate(telegramTask, "telegramTask", 4096, NULL, 1, &telegramTaskHandle); // Crear tarea de FreeRTOS
}

void loop() {
  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = HIGH;
  int buttonState = digitalRead(buttonPin);

  // Detectar cambios en el estado del botón después del tiempo de debounce
  if (buttonState != lastButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();

    if (buttonState == LOW) {
      Serial.println("Se va a reactivar la tarea.");
      vTaskResume(telegramTaskHandle); // Reanudar la tarea cuando se presione el botón
    }
  }

  lastButtonState = buttonState;
}