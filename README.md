# SmartHomeAutomation

1. Introduction
The project involves an ESP8266 microcontroller controlling four appliances (Fan, AC, TV, and Room Light) via a 4-channel relay module. The system also includes fan speed control based on room temperature and AC status. The goal is to enable both local and remote control of these appliances using Blynk, while extending voice control functionalities through Alexa and Google Assistant using IFTTT.

This report covers:

1 The hardware setup.
2 Code implementation for ESP8266 using Blynk for remote control.
3 Integration of voice control with Alexa and Google Assistant using IFTTT.

2. Hardware Setup
The hardware comprises:

1 ESP8266 microcontroller: The main component to control the relays and handle internet connectivity.
2 4-Channel Relay Module: Connected to the ESP8266 to switch the appliances (Fan, AC, TV, and Room Light).
3 DHT11 Temperature Sensor: Measures the room temperature to automate fan control.
4 Each appliance is connected to a specific GPIO pin of the ESP8266 as follows:

Fan: GPIO 5
AC: GPIO 4
TV: GPIO 0
Room Light: GPIO 2

3. Code Implementation
The ESP8266 is programmed to control the four appliances through relays, adjusting the fan speed automatically based on the room temperature and the state of the AC. For remote control, the Blynk platform is utilized, providing a user interface accessible from anywhere via a smartphone or web application.

Key Features:

1 Relay Control: Each relay is associated with an appliance, turning the appliance on/off based on commands from Blynk or IFTTT.
2 Temperature-Based Fan Speed: The fan speed is adjusted dynamically based on the temperature measured by the DHT11 sensor:
   1 Fan turns on at speed 3 when the temperature drops below 24°C and the AC is on.
   2 Fan runs at maximum speed if the temperature exceeds 26°C.
3 Blynk Control: The Blynk app allows remote control of all four appliances using virtual pins to issue on/off commands for each device.

4. Integration with Alexa and Google Assistant using IFTTT
To allow voice control of the appliances through Alexa and Google Assistant, we employ IFTTT. IFTTT can trigger Blynk commands using Webhooks, enabling a seamless integration between these voice assistants and the ESP8266.

Step-by-Step Process:

Create an IFTTT account:

Log into IFTTT and link your Google Assistant and Alexa accounts.
Set up Blynk Webhooks:

Blynk’s HTTP API allows control of devices via simple URLs. For example, turning on the fan can be done by sending an HTTP request:
http
Copy code
http://blynk-cloud.com/YourBlynkAuthToken/update/D5?value=0
This sends a command to the ESP8266 to activate the relay connected to the fan.

Create IFTTT Applets:

For Alexa: You can create an applet that listens for a voice command like "Turn on the fan" and then triggers a Blynk Webhook to turn on the relay controlling the fan.
For Google Assistant: Similarly, you can create voice commands for each appliance (fan, AC, TV, and light) and map them to Blynk Webhooks.
Example Applet:

Trigger (Google Assistant): "Turn on the fan"
Action (Webhook): Send an HTTP request to http://blynk-cloud.com/YourBlynkAuthToken/update/D5?value=0 to turn on the fan.
This process is repeated for turning on/off each appliance, enabling full control using voice assistants.

5. Conclusion
This project demonstrates how the ESP8266 microcontroller can be effectively used to control multiple household appliances (Fan, AC, TV, Room Light) with both automated features and remote control via Blynk. Furthermore, by integrating IFTTT, Alexa, and Google Assistant, it adds a layer of convenience, allowing users to manage their appliances through voice commands.

The combination of Blynk for remote control and IFTTT for voice assistant integration provides a versatile and user-friendly solution for smart home automation.




code

// Required Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN 2            // DHT sensor connected to GPIO2 (D4 on some boards)
#define DHTTYPE DHT11       // DHT 11 temperature and humidity sensor

// Relay pins for 4-channel relay module
#define RELAY_FAN 5         // Fan relay (GPIO5)
#define RELAY_AC 4          // AC relay (GPIO4)
#define RELAY_TV 0          // TV relay (GPIO0)
#define RELAY_LIGHT 2       // Room light relay (GPIO2)

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Blynk authentication token
char auth[] = "YourBlynkAuthToken";  // Replace with your Blynk Auth Token

// Wi-Fi credentials
char ssid[] = "YourWiFiSSID";        // Replace with your WiFi SSID
char pass[] = "YourWiFiPassword";    // Replace with your WiFi Password

// Variables to store the states
bool acState = false;  // Store AC state (on/off)

// Timer to schedule periodic tasks
BlynkTimer timer;

// Setup function - runs once at the beginning
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Start Blynk with WiFi credentials
  Blynk.begin(auth, ssid, pass);
  
  // Initialize relay pins
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_AC, OUTPUT);
  pinMode(RELAY_TV, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  
  // Set all relays to OFF initially (high state for inactive)
  digitalWrite(RELAY_FAN, HIGH);  // Fan OFF
  digitalWrite(RELAY_AC, HIGH);   // AC OFF
  digitalWrite(RELAY_TV, HIGH);   // TV OFF
  digitalWrite(RELAY_LIGHT, HIGH); // Light OFF
  
  // Start the DHT sensor
  dht.begin();
  
  // Schedule temperature check every 2 seconds
  timer.setInterval(2000L, checkConditions);
}

// Function to control fan speed based on temperature and AC state
void controlFan(float temperature) {
  // Check if the AC is on
  if (acState) {
    // Adjust fan speed based on temperature
    if (temperature < 24) {
      Serial.println("AC on, temperature below 24°C. Setting fan speed to 3.");
      setFanSpeed(3);  // Speed 3
    } else if (temperature > 26) {
      Serial.println("Temperature above 26°C. Setting fan speed to maximum.");
      setFanSpeed(5);  // Maximum speed
    } else {
      Serial.println("Temperature between 24°C and 26°C. Setting fan to medium.");
      setFanSpeed(2);  // Medium speed
    }
  } else {
    Serial.println("AC off, turning fan off.");
    setFanSpeed(0);  // Turn off fan if AC is off
  }
}

// Function to set fan speed based on relay
void setFanSpeed(int speed) {
  switch (speed) {
    case 0:  // Fan off
      digitalWrite(RELAY_FAN, HIGH);  // Deactivate relay
      break;
    case 2:  // Medium speed
    case 3:  // High speed
    case 5:  // Max speed
      digitalWrite(RELAY_FAN, LOW);   // Activate relay to turn fan on
      break;
  }
}

// Function to check temperature and adjust fan speed
void checkConditions() {
  // Read temperature from DHT sensor
  float temperature = dht.readTemperature();
  
  // Check if the temperature reading is valid
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature from sensor.");
    return;
  }
  
  // Print the current temperature
  Serial.print("Current Temperature: ");
  Serial.println(temperature);
  
  // Control fan speed based on the temperature
  controlFan(temperature);
}

// Blynk virtual pin handler to control AC (V1)
BLYNK_WRITE(V1) {
  acState = param.asInt();  // Read AC state from Blynk app (1 = ON, 0 = OFF)
  
  // Turn AC relay on/off
  digitalWrite(RELAY_AC, acState ? LOW : HIGH);
  Serial.println(acState ? "AC turned ON" : "AC turned OFF");
}

// Blynk virtual pin handler to control TV (V2)
BLYNK_WRITE(V2) {
  int tvState = param.asInt();  // Read TV state from Blynk app
  
  // Turn TV relay on/off
  digitalWrite(RELAY_TV, tvState ? LOW : HIGH);
  Serial.println(tvState ? "TV turned ON" : "TV turned OFF");
}

// Blynk virtual pin handler to control Room Light (V3)
BLYNK_WRITE(V3) {
  int lightState = param.asInt();  // Read Room Light state from Blynk app
  
  // Turn Room Light relay on/off
  digitalWrite(RELAY_LIGHT, lightState ? LOW : HIGH);
  Serial.println(lightState ? "Room Light turned ON" : "Room Light turned OFF");
}

// Main loop function - runs continuously
void loop() {
  Blynk.run();  // Run Blynk
  timer.run();  // Run the timer to check conditions regularly
}
