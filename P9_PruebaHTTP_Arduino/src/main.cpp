#include <WiFi.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>

const char* ssid = "JET Home";
const char* password = "chemundo300665";
const char* botToken = "5992410997:AAGNhUo8I5K-MnNsfuT3mrWa-RaVl4RFmd0";
const char* chatId = "1467611816";
const int buttonPin = 0;
const int debounceDelay = 50;

TaskHandle_t telegramTaskHandle;

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

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
      Serial.println("Button pressed.");
      bot.sendMessage(chatId, "Button pressed.");
    }
  }

  lastButtonState = buttonState;

  // Leer y procesar los comandos del bot de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    Serial.println("New message received.");
    for (int i = 0; i < numNewMessages; i++) {
      String chatId = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      // Procesar el comando "/enable"
      if (text == "/enable") {
        Serial.println("Enabling button.");
        vTaskResume(telegramTaskHandle);
        bot.sendMessage(chatId, "Button enabled.");
      }

      // Procesar el comando "/disable"
      if (text == "/disable") {
        Serial.println("Disabling button.");
        vTaskSuspend(telegramTaskHandle);
        bot.sendMessage(chatId, "Button disabled.");
      }
    }
    // Actualizar el mensaje recibido más reciente
    bot.last_message_received = bot.messages[numNewMessages - 1].update_id + 1;
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}