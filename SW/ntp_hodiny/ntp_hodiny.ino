/* WiFi NTP Clock for LaskaKit Segment Display
 * ESP32 based internet clock with NTP synchronization
 * and optional 180° display rotation.
 *
 * Board:           LaskaKit microESP-C3:                     https://www.laskakit.cz/laskakit-microesp/
 * Display board:   2x LaskaKit Driver Board 1.5″ TLC59116    https://www.laskakit.cz/deska-pro-2x-1-5--7segmentove-zobraovace--i2c/
 * Display:         2x Shenzhen Zhihao Elec FJ15101BH         https://www.laskakit.cz/shenzhen-zhihao-elec-fj15101bh-sedmisegmentovy-displej-1-5-spolecna-anoda--cerveny/
 *
 * Email: podpora@laskakit.cz
 * Web: laskakit.cz
 */


#include <WiFi.h>
#include "time.h"
#include <Wire.h>

#define SCL 10
#define SDA 8

#define DIG1 0x60
#define DIG2 0x61

const char *ssid = "xxxxxxx";
const char *wifipw = "xxxxxx";

bool rotateDisplay = true;

int dp = 0;

int number[] = {
  0x7E, // 0
  0x0C, // 1
  0xB6, // 2
  0x9E, // 3
  0xCC, // 4
  0xDA, // 5
  0xFA, // 6
  0x0E, // 7
  0xFE, // 8
  0xDE, // 9
  0x00  // blank
};

int fnumber[] = {
  0x7E, // 0
  0x60, // 1
  0xB6, // 2
  0xF2, // 3
  0xE8, // 4
  0xDA, // 5
  0xDE, // 6
  0x70, // 7
  0xFE, // 8
  0xFA, // 9
  0x00  // blank
};

String tzone = "CET-1CEST,M3.5.0,M10.5.0/3";

void init_TLC59116(int addr);
void set_all(int addr, int pwm);
void set_pin(int addr, int pin, int pwm);
void print_num(int addr, int number, int pwm, bool d);

void setTimezone(String timezone) {
  Serial.printf("Setting Timezone to %s\n", timezone.c_str());

  setenv("TZ", timezone.c_str(), 1);
  tzset();
}

void initTime(String timezone) {
  struct tm timeinfo;

  Serial.println("Setting up time");

  configTime(0, 0, "tik.cesnet.cz");

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println("Got time from NTP");

  setTimezone(timezone);
}

void startWifi() {

  WiFi.begin(ssid, wifipw);

  Serial.println("Connecting Wifi");

  const int wifiAnim[][2] = {

    // horní hrana
    {DIG1, 9},
    {DIG1, 1},
    {DIG2, 9},
    {DIG2, 1},

    // pravá hrana
    {DIG2, 2},
    {DIG2, 3},

    // spodní hrana
    {DIG2, 4},
    {DIG2, 12},
    {DIG1, 4},
    {DIG1, 12},

    // levá hrana
    {DIG1, 5},
    {DIG1, 6}
  };

  int step = 0;
  int steps = sizeof(wifiAnim) / sizeof(wifiAnim[0]);

  while (WiFi.status() != WL_CONNECTED) {

    set_all(DIG1, 0);
    set_all(DIG2, 0);

    set_pin(
      wifiAnim[step][0],
      wifiAnim[step][1],
      255
    );

    step++;

    if (step >= steps) {
      step = 0;
    }

    delay(120);
  }

  set_all(DIG1, 0);
  set_all(DIG2, 0);

  Serial.println("WiFi connected");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}

void setup() {

  Serial.begin(115200);

  Wire.begin(SDA, SCL);

  init_TLC59116(DIG1);
  init_TLC59116(DIG2);

  set_all(DIG1, 0);
  set_all(DIG2, 0);

  startWifi();

  initTime(tzone);
}

void loop() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeHour[3];
  char timeMin[3];

  strftime(timeHour, 3, "%H", &timeinfo);
  strftime(timeMin, 3, "%M", &timeinfo);

  int hours = atoi(timeHour);
  int minutes = atoi(timeMin);

  int h1 = hours / 10;
  int h2 = hours % 10;

  int m1 = minutes / 10;
  int m2 = minutes % 10;

  if (h1 == 0) {
    h1 = 10;
  }

  int dig1_left;
  int dig1_right;

  int dig2_left;
  int dig2_right;

  if (!rotateDisplay) {

    // normální orientace

    dig1_left  = number[h1];
    dig1_right = number[h2];

    dig2_left  = fnumber[m2];
    dig2_right = fnumber[m1];

  } else {

    // otočený displej o 180°

    dig1_left  = fnumber[m2];
    dig1_right = fnumber[m1];

    dig2_left  = number[h1];
    dig2_right = number[h2];
  }

  dig1_right |= dp << 8;
  dig2_right |= dp << 8;

  print_num(
    DIG1,
    (dig1_left << 8) | dig1_right,
    255,
    false
  );

  print_num(
    DIG2,
    (dig2_left << 8) | dig2_right,
    255,
    false
  );

  delay(500);

  dp = !dp;
}

void init_TLC59116(int addr) {

  Wire.beginTransmission(addr);

  Wire.write(0x80);

  Wire.write(0x00);
  Wire.write(0x00);

  for (int i = 0; i < 16; i++) {
    Wire.write(0x00);
  }

  Wire.write(0xFF);
  Wire.write(0x00);

  Wire.write(0xAA);
  Wire.write(0xAA);
  Wire.write(0xAA);
  Wire.write(0xAA);

  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);

  Wire.write(0xFF);

  Wire.endTransmission();
}

void set_all(int addr, int pwm) {

  Wire.beginTransmission(addr);

  Wire.write(0x82);

  for (int i = 0; i < 16; i++) {
    Wire.write(pwm);
  }

  Wire.endTransmission();
}

void set_pin(int addr, int pin, int pwm) {

  Wire.beginTransmission(addr);

  Wire.write(0x01 + pin);

  Wire.write(pwm);

  Wire.endTransmission();
}

void print_num(int addr, int number, int pwm, bool d) {

  Wire.beginTransmission(addr);

  Wire.write(0x82);

  for (int i = 1; i < 17; i++) {

    if (bitRead(number, i)) {

      Wire.write(pwm);

      if (d) {
        Serial.print("1");
      }

    } else {

      Wire.write(0);

      if (d) {
        Serial.print("0");
      }
    }

    if (d && i == 8) {
      Serial.print(" ");
    }
  }

  if (d) {
    Serial.println();
  }

  Wire.endTransmission();
}