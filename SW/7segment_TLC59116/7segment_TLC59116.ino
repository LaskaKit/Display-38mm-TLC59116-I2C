#include <Wire.h>

#define I2C_ADDR 0x60

int number[] = {0x7E, 0x0C, 0xB6, 0x9E, 0xCC, 0xDA, 0xFA, 0x0E, 0xFE, 0xDE, 0x00};

void setup(){
  Wire.begin(21, 22);
  
  init_TLC59116(I2C_ADDR);
  set_all(I2C_ADDR, 0);
}

void loop(){
  int num = 88;
  
  int n1 = number[num / 10];
  int n2 = number[num % 10];

  n1 |= 1 << 8; // desetinna tecka na prvni pozici zleva
  n2 |= 1 << 8; // desetinna tecka na druhe pozici zleva
  
  print_num(I2C_ADDR, (n1 << 8) | n2, 255, false);
}

void init_TLC59116(int addr) {
  
  Wire.beginTransmission(addr);
  Wire.write(0x80);

  Wire.write(0x00);  // Register 00 /  Mode1  
  Wire.write(0x00);  // Register 01 /  Mode2 

  Wire.write(0x00);  // Register 02 /  PWM LED 1
  Wire.write(0x00);  // Register 03 /  PWM LED 2    
  Wire.write(0x00);  // Register 04 /  PWM LED 3
  Wire.write(0x00);  // Register 05 /  PWM LED 4
  Wire.write(0x00);  // Register 06 /  PWM LED 5
  Wire.write(0x00);  // Register 07 /  PWM LED 6
  Wire.write(0x00);  // Register 08 /  PWM LED 7
  Wire.write(0x00);  // Register 09 /  PWM LED 8
  Wire.write(0x00);  // Register 0A /  PWM LED 9
  Wire.write(0x00);  // Register 0B /  PWM LED 10
  Wire.write(0x00);  // Register 0C /  PWM LED 11
  Wire.write(0x00);  // Register 0D /  PWM LED 12
  Wire.write(0x00);  // Register 0E /  PWM LED 13
  Wire.write(0x00);  // Register 0F /  PWM LED 14
  Wire.write(0x00);  // Register 10 /  PWM LED 15
  Wire.write(0x00);  // Register 11 /  PWM LED 16

  Wire.write(0xFF);  // Register 12 /  Group duty cycle control
  Wire.write(0x00);  // Register 13 /  Group frequency
  Wire.write(0xAA);  // Register 14 /  LED output state 0  // Default alle LEDs auf PWM 0xAA
  Wire.write(0xAA);  // Register 15 /  LED output state 1  // Default alle LEDs auf PWM
  Wire.write(0xAA);  // Register 16 /  LED output state 2  // Default alle LEDs auf PWM
  Wire.write(0xAA);  // Register 17 /  LED output state 3  // Default alle LEDs auf PWM
  Wire.write(0x00);  // Register 18 /  I2C bus subaddress 1
  Wire.write(0x00);  // Register 19 /  I2C bus subaddress 2
  Wire.write(0x00);  // Register 1A /  I2C bus subaddress 3
  Wire.write(0x00);  // Register 1B /  All Call I2C bus address
  Wire.write(0xFF);  // Register 1C /  IREF configuration  
  Wire.endTransmission();
}

void set_all(int addr, int pwm)
{
  Wire.begin();                     
  Wire.beginTransmission(addr);
  Wire.write(0x82);                 // Startregister 02h 
  for (int i=1 ; i < 17; i++){      // 16Bytes (Register 02h bis 11h)
    Wire.write(pwm);
  }
  Wire.endTransmission();
}

void set_pin(int addr, int pin, int pwm)
{
  Wire.begin();         
  Wire.beginTransmission(addr); 
  Wire.write(0x01 + pin);    // Register LED-Nr
  Wire.write(pwm);
  Wire.endTransmission(); 
}

void print_num(int addr, int number, int pwm, bool d)
{
  Wire.begin();                 
  Wire.beginTransmission(addr); 
  Wire.write(0x82);                 // Startregister 02h 
  for (int i=1 ; i < 17; i++){      // 16Bytes (Register 02h bis 11h)
    if (bitRead(number, i)){
      Wire.write(pwm);
      if (d) Serial.print(1);
    }else{
      Wire.write(0);
      if (d) Serial.print(0);
    }
    if (d) if (i == 8) Serial.print(" ");
  }
  if (d) Serial.println();
  Wire.endTransmission();
}
