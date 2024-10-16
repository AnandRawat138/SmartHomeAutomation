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
