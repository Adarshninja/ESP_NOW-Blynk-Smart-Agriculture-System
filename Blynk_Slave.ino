// This is the final code for the Slave with Blynk integration
// change WiFi credentials according to yours 
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>
// Include Blynk library

// Blynk template ID, template name, and authentication token
#define BLYNK_TEMPLATE_ID "xxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxxxxxxxxxx" // Replace with your Blynk Auth Token

#include <BlynkSimpleEsp8266.h> 

// Pin definitions
#define DHTPIN D3          // Pin connected to DHT11
#define DHTTYPE DHT11      // DHT 11 type
#define MOISTURE_PIN A0    // Pin connected to Soil Moisture sensor (Analog input)

DHT dht(DHTPIN, DHTTYPE);

// Structure to hold the data to send to master
typedef struct struct_message {
  float temperature;
  float humidity;
  int moisture; // Keep it as int for ease of sending but will convert to percentage
} struct_message;

// Create a struct_message object to hold sensor data
struct_message myData;

// Function prototypes
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Set up Wi-Fi connection
  WiFi.begin("iot", "00000000"); // Wi-Fi SSID and Password
  // Wait for the Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, "iot", "00000000"); // Blynk credentials

  // Set WiFi mode to station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set the role of the device as a slave
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  // Register the send callback function
  esp_now_register_send_cb(OnDataSent);
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(sendStatus == 0 ? "Delivery Success" : "Delivery Fail");
}

void loop() {
  // Read temperature and humidity from DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read soil moisture sensor value
  int moistureReading = analogRead(MOISTURE_PIN);
  
  // Convert moisture level to percentage (0 to 100)
  int moisturePercentage = map(moistureReading, 0, 1023, 100, 0); // Inverted for moisture scale

  // Check if the DHT sensor readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Fill the struct with sensor data
  myData.temperature = temperature;
  myData.humidity = humidity;
  myData.moisture = moisturePercentage; // Store percentage in the struct

  // Debug output to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Moisture: ");
  Serial.println(moisturePercentage); // Print moisture percentage

  Serial.println("Preparing to send data...");

  // Send sensor data to the master ESP8266
  uint8_t masterAddress[] = {0xF4, 0xCF, 0xA2, 0xDA, 0x27, 0x62};  // Master's MAC Address for sending data to the LCD 16x2
  esp_now_send(masterAddress, (uint8_t *)&myData, sizeof(myData));

  // Send data to Blynk
  Blynk.virtualWrite(V0, temperature);     // Send temperature to V0
  Blynk.virtualWrite(V1, humidity);        // Send humidity to V1
  Blynk.virtualWrite(V2, moisturePercentage); // Send moisture to V2

  // Wait for 5 seconds before sending the next packet
  delay(5000);
}
