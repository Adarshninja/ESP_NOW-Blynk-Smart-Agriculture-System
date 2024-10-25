// This is the final code for the Master without Blynk
// Just upload this code to the master ESP for receiving information from the slave and displaying it on the master's LCD

// just Change the wifi credentials!

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Pin definitions
#define RELAY_PIN D5        // Pin connected to relay (for water pump control)

// Initialize the LCD with I2C address 0x27 (adjust if necessary)
LiquidCrystal_PCF8574 lcd(0x27);

// Structure to receive data from the slave
typedef struct struct_message {
  float temperature;
  float humidity;
  int moisture;
} struct_message;

// Create a struct_message object to hold received data
struct_message receivedData;

// Relay control variable
bool relayStatus = false;
int moistureThreshold = 30;  // Threshold for soil moisture to turn on the pump

// Function prototypes
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set up Wi-Fi connection
  WiFi.begin("iot", "00000000"); // Wi-Fi SSID and Password
  // Wait for the Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Initialize the LCD
  lcd.begin(16, 2);          // Set up the LCD's number of columns and rows
  lcd.setBacklight(255);      // Turn on the backlight with maximum brightness

  // Display a startup message on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for data");
  
  // Initialize relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Start with the relay off

  // Set up Wi-Fi in station mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP-NOW Init Fail");
    return;
  }

  // Register the receive callback function to process data from the slave
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Master ready to receive data...");
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  // Copy the received data into the struct
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Print received data to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(receivedData.temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(receivedData.humidity);
  Serial.print(" %, Moisture: ");
  Serial.println(receivedData.moisture);

  // Display received data on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(receivedData.temperature);
  lcd.print(" H:");
  lcd.print(receivedData.humidity);
  
  lcd.setCursor(0, 1);
  lcd.print("MS:");
  lcd.print(receivedData.moisture);
  lcd.print("%");

  // Check the soil moisture level and control the relay
  if (receivedData.moisture < moistureThreshold) {
    // Soil is dry (below threshold), turn on the water pump (relay on)
    relayStatus = true;
    digitalWrite(RELAY_PIN, HIGH);  // Turn relay on
  } else {
    // Soil is wet (above threshold), turn off the water pump (relay off)
    relayStatus = false;
    digitalWrite(RELAY_PIN, LOW);   // Turn relay off
  }
}

void loop() {
  // No longer need to run Blynk
}
