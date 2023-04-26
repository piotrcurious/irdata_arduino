
// Sender.ino

// Include the IR data protocol library
#include <IRDataProtocol.h>

// Include the DHT sensor library
#include <DHT.h>

// Define the pin number for the DHT sensor
#define DHT_PIN 2

// Define the type of DHT sensor
#define DHT_TYPE DHT11

// Create an instance of IRsendData class
IRsendData irsend;

// Create an instance of DHT class
DHT dht(DHT_PIN, DHT_TYPE);

// Define a variable to store the temperature
float temperature;

// Define a variable to store the humidity
float humidity;

// Define a variable to store the telemetry data
uint64_t telemetry;

// Define a variable to store the control message data
uint64_t control;

// Define a variable to store the fan state
bool fanState;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize DHT sensor
  dht.begin();

  // Initialize fan state to false (off)
  fanState = false;
}

void loop() {
  // Read temperature and humidity from DHT sensor
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Check if temperature and humidity are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Encode temperature and humidity into telemetry data
  // Use the first byte for temperature and the second byte for humidity
  // Multiply by 10 and cast to integer to preserve one decimal place
  telemetry = ((uint64_t)(temperature * 10) << 8) | (uint64_t)(humidity * 10);

  // Send telemetry data using IR data protocol
  irsend.sendData(telemetry, 16);

  // Print telemetry data to serial monitor
  Serial.print("Sent telemetry data: ");
  Serial.println(telemetry, BIN);

  // Wait for 1 second
  delay(1000);

  // Toggle fan state
  fanState = !fanState;

  // Encode fan state into control message data
  // Use the first bit for fan state and the rest for padding
  control = fanState ? 0b1000000000000000 : 0b0000000000000000;

  // Send control message data using IR data protocol
  irsend.sendData(control, 16);

  // Print control message data to serial monitor
  Serial.print("Sent control message data: ");
  Serial.println(control, BIN);

  // Wait for 1 second
  delay(1000);
}


//Source: Conversation with Bing, 4/26/2023
//(1) Arduino - Controls Fan | Arduino Tutorial - Arduino Getting Started. https://arduinogetstarted.com/tutorials/arduino-controls-fan.
//(2) How To Control Fan using Arduino UNO - Makerguides.com. https://www.makerguides.com/how-to-control-fan-using-arduino-uno/.
//(3) Arduino - Cooling System using DHT Sensor | Arduino Tutorial. https://arduinogetstarted.com/tutorials/arduino-cooling-system-using-dht-sensor.
