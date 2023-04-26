
// Receiver.ino

// Include the IR data protocol library
#include <IRDataProtocol.h>

// Define the pin number for the relay
#define RELAY_PIN 3

// Create an instance of IRrecvData class
IRrecvData irrecv;

// Define a variable to store the telemetry data
uint64_t telemetry;

// Define a variable to store the control message data
uint64_t control;

// Define a variable to store the temperature
float temperature;

// Define a variable to store the humidity
float humidity;

// Define a variable to store the fan state
bool fanState;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize relay pin as an output
  pinMode(RELAY_PIN, OUTPUT);

  // Initialize fan state to false (off)
  fanState = false;

  // Turn off relay
  digitalWrite(RELAY_PIN, LOW);

  // Enable IR receiver
  irrecv.enableIRIn();
}

void loop() {
  // Check if there is an IR signal received
  if (irrecv.getResults()) {

    // Decode the IR signal
    if (irrecv.decode()) {

      // Check if the protocol is IR data protocol
      if (irrecv.protocolNum == DATA_PROTOCOL) {

        // Check if the data length is 16 bits
        if (irrecv.bits == 16) {

          // Get the data value
          uint64_t data = irrecv.value;

          // Check if the data is a telemetry data or a control message data
          if (data & 0b1000000000000000) { // control message data

            // Get the fan state from the first bit
            fanState = data & 0b0000000000000001;

            // Print fan state to serial monitor
            Serial.print("Received fan state: ");
            Serial.println(fanState ? "ON" : "OFF");

            // Turn on or off relay according to fan state
            digitalWrite(RELAY_PIN, fanState ? HIGH : LOW);

          } else { // telemetry data

            // Get the temperature from the first byte and divide by 10 to get one decimal place
            temperature = (float)(data >> 8) / 10.0;

            // Get the humidity from the second byte and divide by 10 to get one decimal place
            humidity = (float)(data & 0b0000000011111111) / 10.0;

            // Print temperature and humidity to serial monitor
            Serial.print("Received temperature: ");
            Serial.print(temperature);
            Serial.print(" *C, humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
          }
        }
      }
    }

    // Resume IR receiver
    irrecv.enableIRIn();

  }
}
