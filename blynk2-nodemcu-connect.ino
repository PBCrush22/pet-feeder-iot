#define BLYNK_TEMPLATE_ID "TMPL*********"
#define BLYNK_TEMPLATE_NAME "Pet Feeder"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG

#include "BlynkEdgent.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

const char *ssid = "*******";
const char *password = "*******";

const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
Servo servo_1;

#define motor D0

// Define feeding times in seconds since midnight
const int FEEDING_TIME_MORNING = 8 * 3600;            // 8 AM
const int FEEDING_TIME_EVENING = 18 * 3600;           // 6 PM
bool fedMorning = false;
bool fedEvening = false;

int singleFeed = 0; // State of the single feed button
int autoFeed = 1;   // State of the automatic feeding button

BLYNK_WRITE(V0) {
  singleFeed = param.asInt();
}

BLYNK_WRITE(V1) {
  autoFeed = param.asInt();
  Serial.print("Updated autoFeed value: ");
  Serial.println(autoFeed);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  BlynkEdgent.begin();
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  servo_1.attach(2);
  servo_1.write(0);  // Initialize servo to 0 degrees
  timeClient.begin();
  timeClient.setTimeOffset(28800);
}

void loop() {
  BlynkEdgent.run();  // Run Blynk tasks
  

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi");
  }

  timeClient.update();

  // Update time client
  // if (!timeClient.update()) {
  //   Serial.println("Failed to update time");
  //   delay(1000); // Wait a bit before trying again
  //   return; // Skip the rest of the loop if time update failed
  // }
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Time: ");
  Serial.println(formattedTime);

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute);

  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);

  int currentSeconds = currentHour * 3600 + currentMinute * 60 + currentSecond;
  Serial.print("Time in Seconds: ");
  Serial.println(currentSeconds);

  Serial.print("Autofeed status: ");
  Serial.println(autoFeed);

  // Automatic feeding at fixed times if autoFeed is on
  if (autoFeed == 1) {
    // Check if it's feeding time in the morning
    if (currentSeconds >= FEEDING_TIME_MORNING && currentSeconds < (FEEDING_TIME_MORNING + 60) && !fedMorning) {
      Serial.println("Feeding in the morning...");
      feedPet();
      fedMorning = true;
    }

    // Check if it's feeding time in the evening
    if (currentSeconds >= FEEDING_TIME_EVENING && currentSeconds < (FEEDING_TIME_EVENING + 60) && !fedEvening) {
      Serial.println("Feeding in the evening...");
      feedPet();
      fedEvening = true;
    }

    // Reset the flags at midnight
    if (currentHour == 0 && currentMinute == 0 && currentSecond == 0) {
      fedMorning = false;
      fedEvening = false;
    }
  }

  // Single feed if singleFeed is on
  if (singleFeed == 1) {
    Serial.println("Single feed button pressed...");
    feedPet();
    singleFeed = 0; // Reset button state to prevent continuous rotation
  }

  delay(1000);  // Small delay to prevent overwhelming the loop
}

void feedPet() {
  servo_1.write(90);  // Rotate servo to 90 degrees
  delay(1000);        // Wait for 1 second
  servo_1.write(0);   // Rotate servo back to 0 degrees
  delay(1000);        // Wait for 1 second
  Serial.println("Feeding done...");
}
