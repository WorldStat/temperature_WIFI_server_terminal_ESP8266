#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Wire.h>

// DHT sensor setup
#define DHT_SENSOR_PIN  D7 // The ESP8266 pin D7 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Onboard LED setup
#define LED_PIN D0

// Default Wi-Fi credentials
const char* ssid = "Dr. Flower";
const char* password = "";

// Create an instance of the server
ESP8266WebServer server(80);

// Variable to store log history
String logHistory = "";

// Setup function
void setup() {
  Serial.begin(115200);

  // Set the ESP8266 as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Initialize the DHT sensor
  dht_sensor.begin();

  // Initialize the onboard LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Define the root URL
  server.on("/", handleRoot);
  server.on("/command", handleCommand);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

// Loop function
void loop() {
  server.handleClient();
}

// Handle the root URL
void handleRoot() {
  // HTML content with a terminal window
  String page = "<!DOCTYPE html><html>";
  page += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>";
  page += "<title>DR.FLOWER</title>";
  page += "<style>";
  page += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; color: #333; }";
  page += ".terminal { width: 80%; height: 300px; margin: 20px auto; padding: 10px; background-color: #000; color: #0F0; font-family: monospace; overflow-y: scroll; }";
  page += "input { width: 80%; padding: 10px; font-size: 16px; }";
  page += "</style>";
  page += "</head>";
  page += "<body>";
  page += "<h1><a href='https://edubox.ai' target='_blank'>DR.FLOWER</a></h1>";
  page += "<div class='terminal' id='terminal'></div>";
  page += "<input type='text' id='commandInput' placeholder='Enter command...'><button onclick='sendCommand()'>Send</button>";
  page += "<script>";
  page += "function sendCommand() {";
  page += "  var command = document.getElementById('commandInput').value;";
  page += "  var xhr = new XMLHttpRequest();";
  page += "  xhr.open('GET', '/command?cmd=' + encodeURIComponent(command), true);";
  page += "  xhr.onload = function () {";
  page += "    if (xhr.status === 200) {";
  page += "      var terminal = document.getElementById('terminal');";
  page += "      terminal.innerHTML += '> ' + command + '<br>' + xhr.responseText + '<br>';";
  page += "      terminal.scrollTop = terminal.scrollHeight;";
  page += "      document.getElementById('commandInput').value = '';";
  page += "    }";
  page += "  };";
  page += "  xhr.send();";
  page += "}";
  page += "window.onload = function() { sendCommand('/command?cmd=help'); };";
  page += "</script>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

// Handle commands
void handleCommand() {
  String cmd = server.arg("cmd");
  String response = "";
  logHistory += "> " + cmd + "<br>";

  if (cmd == "1" || cmd == "temperature") {
    float temperature_C = dht_sensor.readTemperature();
    float temperature_F = dht_sensor.readTemperature(true);
    if (isnan(temperature_C) || isnan(temperature_F)) {
      response = "Failed to read temperature!";
    } else {
      response = "Temperature: " + String(temperature_C) + "°C (" + String(temperature_F) + "°F)";
    }
  } else if (cmd == "2" || cmd == "humidity") {
    float humidity = dht_sensor.readHumidity();
    if (isnan(humidity)) {
      response = "Failed to read humidity!";
    } else {
      response = "Humidity: " + String(humidity) + "%";
    }
  } else if (cmd == "3" || cmd == "pressure") {
    response = "Pressure sensor not implemented!";
  } else if (cmd == "4" || cmd == "soil moisture") {
    response = "Soil moisture sensor not implemented!";
  } else if (cmd == "5" || cmd == "servo") {
    response = "Servo motor control not implemented!";
  } else if (cmd == "6" || cmd == "current usage") {
    response = "Current usage not implemented!";
  } else if (cmd == "7" || cmd == "cpu frequency") {
    response = "CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz";
  } else if (cmd == "8" || cmd == "memory used") {
    response = "Memory used: " + String(ESP.getFreeHeap()) + " bytes";
  } else if (cmd == "9" || cmd == "uptime") {
    response = "Uptime: " + String(millis() / 1000) + " seconds";
  } else if (cmd == "10" || cmd == "log history") {
    response = logHistory;
  } else if (cmd == "11" || cmd == "onboard led on") {
    digitalWrite(LED_PIN, HIGH);
    response = "Onboard LED turned on.";
  } else if (cmd == "12" || cmd == "onboard led off") {
    digitalWrite(LED_PIN, LOW);
    response = "Onboard LED turned off.";
  } else if (cmd == "13" || cmd == "wifi networks") {
    int n = WiFi.scanNetworks();
    if (n == 0) {
      response = "No Wi-Fi networks found";
    } else {
      response = "Wi-Fi networks:<br>";
      for (int i = 0; i < n; ++i) {
        response += String(i + 1) + ": " + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)<br>";
      }
    }
  } else if (cmd == "help") {
    response = "Available commands:<br>";
    response += "1. temperature<br>";
    response += "2. humidity<br>";
    response += "3. pressure<br>";
    response += "4. soil moisture<br>";
    response += "5. servo<br>";
    response += "6. current usage<br>";
    response += "7. cpu frequency<br>";
    response += "8. memory used<br>";
    response += "9. uptime<br>";
    response += "10. log history<br>";
    response += "11. onboard led on<br>";
    response += "12. onboard led off<br>";
    response += "13. wifi networks<br>";
  } else {
    response = "Unknown command!";
  }

  logHistory += response + "<br>";
  server.send(200, "text/plain", response);
}
