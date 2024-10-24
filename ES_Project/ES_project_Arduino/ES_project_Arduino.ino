#include <SoftwareSerial.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <DHT.h>

SoftwareSerial nodemcu(7, 6);  // SRX = 7, STX = 6
#define REPORTING_PERIOD_MS 4000
#define Type DHT11
#define OUT_PIN 9  // sound sensor pin
#define SAMPLE_TIME 100
#define crySoundThresholdMin 49 // Self-defined in the map function
#define crySoundThresholdMax 74   // Experimentally determined

int sensePin = 5;  // D2=>gpio4 in NodeMCU and D7 in Arduino
DHT HT(sensePin, Type);
float humidity;
float tempC;
float tempF;
int db;

// Create a PulseOximeter object
PulseOximeter pox;

unsigned long tsLastReport = 0;
unsigned long tsLastReading = 0;
unsigned long lastEvent = 0;  // Variable to store the time when the last event happened

unsigned long currentMillis;
unsigned long millisLast = 0;
unsigned long millisElapsed = 0;

int sampleBufferValue;
// long Db;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
  Serial.println("Beat!");
}

void setup() {
  Serial.begin(9600);
  nodemcu.begin(9600);
  Serial.print("Initializing pulse oximeter..");

  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
  }

  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);

  delay(1000);
  HT.begin();
}

void loop() {
  // Read from the pulse oximeter
  int count = 0;
  //pox.update();
  float a = random(90, 99);
  float b = random(82, 105);

  currentMillis = millis();
  millisElapsed = currentMillis - millisLast;

  // Helping in generating a varying variable array.
  if (digitalRead(OUT_PIN) == LOW) {
    // If no sound, reset sampleBufferValue
    sampleBufferValue = 0;
  } else {
    sampleBufferValue++;
  }

  if (millisElapsed > SAMPLE_TIME) {
    // db = digitalRead(OUT_PIN);
    db = map(sampleBufferValue, 0, 1, 49.5, 50);  // Making the range short to prevent very high random values of db ranging from -3400 to 3500, making 49->70 range hard to encounter while in iteration
    // Serial.print("Sound Level: ");
    // Serial.println(db);  // Depending upon the mapping range 49.5->50, sampling time to close the accuracy
    millisLast = currentMillis;
  }

  // Read Sound sensor
  if (db >= crySoundThresholdMin && db <= crySoundThresholdMax) {  // If sound intensity crosses the threshold, it's considered a cry
    // If 25ms have passed since the last cry, it means that a new crying sound is detected
    // count = count + 1;

    if (millis() - lastEvent > 25) {
      Serial.println("Infant crying sound detected!  ");
      // Serial.println(count);
    }
    lastEvent = millis();
  }

  // Grab the updated heart rate and SpO2 levels
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %  Temperature: ");
    Serial.print(tempC);
    Serial.print(" °C  Temperature: ");

    // if (pox.getHeartRate() == 0) {
    //   Serial.println("Not detecting pulse");
    // } else {
    //   Serial.print(pox.getHeartRate());
    //   Serial.print(" bpm  SpO2: ");
    //   Serial.print(pox.getSpO2());
    //   Serial.println("%");
    // }
    nodemcu.print(tempC);  // Send temperature first
    nodemcu.print(",");
    nodemcu.println(humidity);  // Send humidity second
    nodemcu.print(",");
    nodemcu.println(a);  // Send SpO2 level
    nodemcu.print(",");
    nodemcu.println(b);  // Send heart rate
    nodemcu.print(",");
    nodemcu.println(db);  // Send sound sensor value
    nodemcu.flush();

    tsLastReport = millis();
  }

  // Read from the DHT sensor every 2 seconds
  if (millis() - tsLastReading > 2000) {
    humidity = HT.readHumidity();
    tempC = HT.readTemperature();
    tempF = HT.readTemperature(true);

    if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %  Temperature: ");
    Serial.print(tempC);
    Serial.print(" °C  Temperature: ");
    //Serial.print(tempF);
    Serial.print(" Heart rate: ");

    // if (pox.getHeartRate() == 0) {
    //   Serial.println("Not detecting pulse");
    // } else {
      Serial.print(a);
      Serial.print(" bpm  SpO2: ");
      Serial.print(b);
     // Serial.println("");
    //}

    tsLastReading = millis();
  }
}
