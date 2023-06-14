// Import the necessary libraries
#include <WiFi.h>
#include <WiFiServer.h>
#include <FreeRTOS.h>
#include <task.h>

// Define the port number
const int port = 8080;

// Create a WiFi server object
WiFiServer server(port);

// The task function that handles the server
void serverTask(void *pvParameters) {
  // Get the WiFi client object
  WiFiClient client = server.available();

  // While there are clients connected
  while (client) {
    // Print the IP address of the client
    Serial.println("New client connected!");
    Serial.println(client.remoteIP());

    // Read data from the client
    String data = client.readStringUntil('\n');

    // Print the data that was received
    Serial.println("Received: ");
    Serial.println(data);

    // Send a response to the client
    client.println("Hello, world!");

    // Close the connection
    client.stop();
    Serial.println("Client disconnected!");
  }
}

// The setup() function runs once, when the Arduino starts up
void setup() {
  // Initialize the serial port
  Serial.begin(9600);

  // Connect to the WiFi network
  WiFi.begin("SSID", "password");

  // Wait for the connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print the IP address of the Arduino
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
  Serial.println("Server started!");

  // Create the server task
  xTaskCreate(serverTask, "serverTask", 1024, NULL, 1, NULL);
}

// The loop() function runs continuously
void loop() {}
