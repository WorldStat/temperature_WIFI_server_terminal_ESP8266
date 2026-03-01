#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ========== WIFI CONFIGURATION ==========
const char* ssid = "WIFI SSID";
const char* password = "PASSWORD";

// ========== DHT CONFIGURATION ==========
#define DHTPIN 13       // D7 on NodeMCU is GPIO 13
#define DHTTYPE DHT22

// ========== DISPLAY CONFIGURATION ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// ========== NTP CONFIGURATION ==========
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// ========== OBJECTS ==========
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ESP8266WebServer server(80);

// ========== VARIABLES ==========
unsigned long previousMillis = 0;
const long interval = 60000; // Log data every minute

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP8266 Data</title>";
  html += "<meta http-equiv=\"refresh\" content=\"60\">";
  html += "</head><body><h1>Logged Data</h1>";
  html += "<table border='1'><tr><th>Timestamp</th><th>Temperature (C)</th><th>Humidity (%)</th></tr>";
  
  File dataFile = LittleFS.open("/data.csv", "r");
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      String timestamp = line.substring(0, line.indexOf(','));
      String temp = line.substring(line.indexOf(',') + 1, line.lastIndexOf(','));
      String hum = line.substring(line.lastIndexOf(',') + 1);
      html += "<tr><td>" + timestamp + "</td><td>" + temp + "</td><td>" + hum + "</td></tr>";
    }
    dataFile.close();
  } else {
    html += "<tr><td colspan='3'>No data logged yet.</td></tr>";
  }
  
  html += "</table></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for(;;); 
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Start LittleFS
  if(LittleFS.begin()) {
    Serial.println("LittleFS initialized.");
  } else {
    Serial.println("An error has occurred while mounting LittleFS");
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Start NTP client
  timeClient.begin();

  // Start web server
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float h = dht.readHumidity();
    float tC = dht.readTemperature();
    float tF = dht.readTemperature(true);
    
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();

    // Log data to file
    if (!isnan(tC) && !isnan(h)) {
      File dataFile = LittleFS.open("/data.csv", "a");
      if (dataFile) {
        dataFile.print(epochTime);
        dataFile.print(",");
        dataFile.print(tC);
        dataFile.print(",");
        dataFile.println(h);
        dataFile.close();
      } else {
        Serial.println("Error opening data.csv for writing");
      }
    }

    // Uptime Calculation
    unsigned long totalSecs = millis() / 1000;
    int d = totalSecs / 86400;
    int h_up = (totalSecs % 86400) / 3600;
    int m = (totalSecs % 3600) / 60;
    
    display.clearDisplay();

    // --- ROW 1: TEMPERATURE ---
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("TEMPERATURE"));
    
    display.setTextSize(2);
    display.setCursor(0, 10);
    if (isnan(tC)) {
      display.print(F("ERROR"));
    } else {
      display.printf("%.1f C", tC);
      display.setTextSize(1);
      display.setCursor(75, 17); // Shifted F slightly right
      display.printf("%0.1f F", tF);
    }

    // --- ROW 2: HUMIDITY & STATUS ICON ---
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.print(F("HUMIDITY"));
    
    display.setTextSize(2);
    display.setCursor(0, 40);
    if (isnan(h)) {
      display.print(F("ERROR"));
    } else {
      display.printf("%.0f%%", h); // Rounded for space
      
      // STATUS LOGIC
      // Ideal: 18C-26C and 40%-60% Humidity
      display.setCursor(65, 40);
      if (tC >= 18.0 && tC <= 26.0 && h >= 40.0 && h <= 60.0) {
        display.print(F("[ OK ]"));
      } else if (tC > 26.0 || h > 60.0) {
        display.print(F("[ HI ]")); // High temp or humidity
      } else {
        display.print(F("[ !! ]")); // General Warning/Cold/Dry
      }
    }

    // --- ROW 3: UPTIME & IP ADDRESS ---
    display.drawLine(0, 56, 128, 56, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 57);
    display.print(WiFi.localIP());

    display.display();
  }
}