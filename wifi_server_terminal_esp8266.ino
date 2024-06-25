#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHT_SENSOR_PIN  D7 // The ESP8266 pin D7 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Default Wi-Fi credentials
const char* ssid = "Dr. Flower";
const char* password = "";

// Create an instance of the server
ESP8266WebServer server(80);

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
  page += "<h1>DR.FLOWER</h1>";
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
  page += "</script>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

// Handle commands
void handleCommand() {
  String cmd = server.arg("cmd");
  String response = "";

  if (cmd == "temperature") {
    float temperature_C = dht_sensor.readTemperature();
    float temperature_F = dht_sensor.readTemperature(true);
    if (isnan(temperature_C) || isnan(temperature_F)) {
      response = "Failed to read temperature!";
    } else {
      response = "Temperature: " + String(temperature_C) + "°C (" + String(temperature_F) + "°F)";
    }
  } else if (cmd == "humidity") {
    float humidity = dht_sensor.readHumidity();
    if (isnan(humidity)) {
      response = "Failed to read humidity!";
    } else {
      response = "Humidity: " + String(humidity) + "%";
    }
  } else if (cmd == "pressure") {
    // Placeholder for pressure sensor reading
    response = "Pressure sensor not implemented!";
  } else if (cmd == "soil moisture") {
    // Placeholder for soil moisture sensor reading
    response = "Soil moisture sensor not implemented!";
  } else if (cmd == "servo") {
    // Placeholder for servo motor control
    response = "Servo motor control not implemented!";
  } else {
    response = "Unknown command!";
  }

  server.send(200, "text/plain", response);
}
