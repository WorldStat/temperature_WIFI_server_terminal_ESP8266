import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
import numpy as np

# Load the dataset
# You need to get the data.csv file from your ESP8266.
# You can either download it from the web server running on the ESP8266
# or by using the LittleFS file system reader.
try:
    data = pd.read_csv('data.csv', header=None, names=['timestamp', 'temperature', 'humidity'])
except FileNotFoundError:
    print("Error: data.csv not found.")
    print("Please download the data.csv file from your ESP8266 first.")
    exit()

# Feature Engineering: Extract hour and minute of the day
datetimes = pd.to_datetime(data['timestamp'], unit='s')
data['hour'] = datetimes.dt.hour
data['minute'] = datetimes.dt.minute

# Create a 'day_night' cycle feature (e.g., 0 for night, 1 for day)
data['day_night'] = data['hour'].apply(lambda x: 1 if 6 <= x < 18 else 0)

# --- Train Temperature Model ---
print("Training temperature model...")

# Features: hour, minute, and day_night
X_temp = data[['hour', 'minute', 'day_night']]
y_temp = data['temperature']

# Split data
X_train_temp, X_test_temp, y_train_temp, y_test_temp = train_test_split(X_temp, y_temp, test_size=0.2, random_state=42)

# Create and train the model
temp_model = LinearRegression()
temp_model.fit(X_train_temp, y_train_temp)

# Print model coefficients
print(f"Temperature Model Coefficients: Intercept: {temp_model.intercept_}, Coefs: {temp_model.coef_}")
print(f"Temperature Model R^2 score: {temp_model.score(X_test_temp, y_test_temp)}")
print("-" * 20)

# --- Train Humidity Model ---
print("Training humidity model...")

# Features: hour, minute, and day_night
X_hum = data[['hour', 'minute', 'day_night']]
y_hum = data['humidity']

# Split data
X_train_hum, X_test_hum, y_train_hum, y_test_hum = train_test_split(X_hum, y_hum, test_size=0.2, random_state=42)

# Create and train the model
hum_model = LinearRegression()
hum_model.fit(X_train_hum, y_hum)

# Print model coefficients
print(f"Humidity Model Coefficients: Intercept: {hum_model.intercept_}, Coefs: {hum_model.coef_}")
print(f"Humidity Model R^2 score: {hum_model.score(X_test_hum, y_test_hum)}")
print("-" * 20)

print("To use these models in your ESP8266, you will need to manually transfer the intercept and coefficients into your Arduino code.")
print("For example, for the temperature model, the prediction formula would be:")
print(f"predicted_temp = {temp_model.intercept_} + (hour * {temp_model.coef_[0]}) + (minute * {temp_model.coef_[1]}) + (day_night * {temp_model.coef_[2]})")

# Example prediction for the next 10 minutes
print("\n--- Example Prediction for Next 10 Minutes ---")
last_hour = data['hour'].iloc[-1]
last_minute = data['minute'].iloc[-1]

for i in range(1, 11):
    next_minute = (last_minute + i) % 60
    next_hour = (last_hour + (last_minute + i) // 60) % 24
    is_day = 1 if 6 <= next_hour < 18 else 0

    predicted_temp = temp_model.predict([[next_hour, next_minute, is_day]])[0]
    predicted_hum = hum_model.predict([[next_hour, next_minute, is_day]])[0]

    print(f"In {i} minute(s) (at {next_hour:02d}:{next_minute:02d}):")
    print(f"  Predicted Temperature: {predicted_temp:.2f} C")
    print(f"  Predicted Humidity: {predicted_hum:.2f} %")
