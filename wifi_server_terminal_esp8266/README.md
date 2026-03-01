# ESP8266 Temperature and Humidity Monitor with Data Logging

This project turns an ESP8266 with a DHT22 sensor and an SSD1306 OLED display into a temperature and humidity monitor that logs data to the local filesystem and serves it over a web interface.

## Features

- Displays current temperature in both Celsius and Fahrenheit.
- Displays current humidity.
- Provides a status indicator:
  - `[ OK ]` for ideal conditions (18-26°C and 40-60% humidity).
  - `[ HI ]` for high temperature (>26°C) or high humidity (>60%).
  - `[ !! ]` for other conditions (e.g., cold or dry).
- Connects to your WiFi network.
- Fetches the current time from an NTP server.
- Logs temperature, humidity, and Unix timestamp to a CSV file (`/data.csv`) every minute.
- Hosts a simple web server to display the logged data.
- Displays the device's IP address on the OLED screen for easy access.

## Hardware Requirements

- ESP8266 development board (e.g., NodeMCU).
- DHT22 temperature and humidity sensor.
- SSD1306 OLED display (128x64 pixels).
- Breadboard and jumper wires.

## Software Requirements

- Arduino IDE.
- ESP8266 board support for the Arduino IDE.
- The following Arduino libraries:
  - `DHT sensor library` by Adafruit.
  - `Adafruit GFX Library`.
  - `Adafruit SSD1306` by Adafruit.
  - `NTPClient` by Fabrice Weinberg.
- Python 3 with `pandas` and `scikit-learn`.

## Wiring

- **DHT22 Sensor:**
  - Connect the sensor's VCC to 3.3V on the ESP8266.
  - Connect the sensor's GND to GND on the ESP8266.
  - Connect the sensor's data pin to GPIO 13 (D7) on the NodeMCU.
- **SSD1306 OLED Display:**
  - Connect the display's VCC to 3.3V on the ESP8266.
  - Connect the display's GND to GND on the ESP8266.
  - Connect the display's SCL pin to GPIO 5 (D1) on the NodeMCU.
  - Connect the display's SDA pin to GPIO 4 (D2) on the NodeMCU.

## Setup and Usage

1.  **Install Arduino IDE and Board Manager:**
    - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
    - In the Arduino IDE, go to `File > Preferences` and add the following URL to the "Additional Boards Manager URLs" field: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`.
    - Go to `Tools > Board > Boards Manager`, search for "esp8266", and install the "esp8266" by "ESP8266 Community".
2.  **Install Libraries:**
    - In the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...`.
    - Search for and install the following libraries:
      - `DHT sensor library` by Adafruit.
      - `Adafruit GFX Library`.
      - `Adafruit SSD1306` by Adafruit.
      - `NTPClient` by Fabrice Weinberg.
3.  **Upload LittleFS Filesystem Uploader:**
    - Go to the [ESP8266 LittleFS Uploader releases page](https://github.com/earlephilhower/arduino-esp8266-littlefs-plugin/releases) and download the `ESP8266FS-X.X.X.zip` file.
    - In the Arduino IDE, go to the `tools` directory and create a directory named `ESP8266FS`.
    - Unzip the downloaded file into the `ESP8266FS` directory. You should now have a `tool` directory inside `ESP8266FS` containing `esp8266fs.jar`.
    - Restart the Arduino IDE.
    - To upload the filesystem image, go to `Tools > ESP8266 LittleFS Data Upload`.
4.  **Configure and Upload:**
    - Open the `wifi_server_terminal_esp8266.ino` sketch in the Arduino IDE.
    - **Important:** Update the `ssid` and `password` variables with your WiFi credentials.
    - Select your ESP8266 board from the `Tools > Board` menu (e.g., "NodeMCU 1.0 (ESP-12E Module)").
    - Select the correct COM port.
    - Click the "Upload" button.

Once uploaded, the ESP8266 will connect to your WiFi network and start logging data. You can view the logged data by navigating to the device's IP address in a web browser. The IP address is displayed on the OLED screen.

## Machine Learning Prediction

This project can be extended to predict the temperature and humidity for the next 10 minutes based on the collected data. This is a multi-step process that involves training a model on your computer and then using the model's parameters on the ESP8266.

This is a lightweight and practical approach for edge computing: the heavy training is done offline, and the ESP8266 only performs fast and simple inference using a mathematical formula.

### 1. Collect Data

Let the ESP8266 run for a while (at least a day) to collect a good amount of data. The data is stored in the `/data.csv` file on the ESP8266's filesystem.

### 2. Get the Data

You can get the `data.csv` file in two ways:

- **Web Server:** Open your web browser and navigate to `http://<ESP8266_IP_ADDRESS>/`. You will see a table with the logged data. Copy and paste this data into a new file named `data.csv` on your computer.
- **Filesystem Reader:** Use a tool like the [ESP LittleFS file system reader](https://github.com/lorol/arduino-esp8266fs-plugin) to download the `/data.csv` file directly from the ESP8266.

### 3. Train the Model

A Python script, `train_model.py`, is provided to train a simple linear regression model. This model uses the hour, minute, and a day/night flag as features to predict temperature and humidity.

1.  **Install Python libraries:**
    ```bash
    pip install pandas scikit-learn
    ```
2.  **Run the training script:**
    ```bash
    python train_model.py
    ```

The script will output the coefficients and intercept for both the temperature and humidity models.

### 4. Use the Model on the ESP8266

The final step is to use the trained model on the ESP8266 to make predictions. Since the model is a simple linear regression, you don't need a heavy TensorFlow Lite library. You can implement the prediction formula directly in the Arduino code.

1.  **Update the `.ino` file:** Open the `wifi_server_terminal_esp8266.ino` file and add the model parameters (intercept and coefficients) you got from the training script. For example:

    ```cpp
    // --- Add these lines for prediction ---
    // Replace with your actual model parameters from the python script
    float temp_intercept = 20.0;
    float temp_coef_hour = 0.1;
    float temp_coef_minute = 0.01;
    float temp_coef_day_night = 1.5;

    float hum_intercept = 55.0;
    float hum_coef_hour = -0.2;
    float hum_coef_minute = -0.02;
    float hum_coef_day_night = -5.0;

    // In the loop() function, you can add a section to display predictions.
    // This example shows the prediction for 5 minutes into the future.
    
    int current_hour = timeClient.getHours();
    int current_minute = timeClient.getMinutes();

    int future_minute = (current_minute + 5) % 60;
    int future_hour = (current_hour + (current_minute + 5) / 60) % 24;
    int is_day_future = (future_hour >= 6 && future_hour < 18) ? 1 : 0;

    float predicted_temp = temp_intercept + (future_hour * temp_coef_hour) + (future_minute * temp_coef_minute) + (is_day_future * temp_coef_day_night);
    float predicted_hum = hum_intercept + (future_hour * hum_coef_hour) + (future_minute * hum_coef_minute) + (is_day_future * hum_coef_day_night);

    // Now you can display the predicted values on the OLED.
    // For example, to show the prediction for 5 minutes ahead on the last line:
    display.setCursor(64, 57);
    display.printf("+5m:%.1fC", predicted_temp);
    ```
2.  **Upload the updated sketch:** Upload the modified sketch to your ESP8266.

The device will now be able to calculate and display predicted temperature and humidity values. You can adapt the code to show predictions for different intervals (e.g., 1 to 10 minutes).
