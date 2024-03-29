#include "application.h"

// Include the port of OneWire
// via https://gist.github.com/wgbartley/8301191
#include "OneWire.h"

// DS18B20 temp sensor DATA is on D1
OneWire one = OneWire(D1);

int spark = D7; // The mini Spark LED
int blue = D3; // Our cold LED
int red = D0; // Our hot LED
int switchTemp = 66;

void setup() {
  Spark.function("tempF", getTempF);
  Spark.function("flashLED", flashLED);
  Spark.function("switchTemp", setSwitchTemp);

  pinMode(spark, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
}

void loop() {
    int current_temp = ((getTemp() * 9.0) / 5.0 + 32.0)*100;
    digitalWrite(spark, LOW);
    delay(300);
    digitalWrite(spark, HIGH);
    delay(300);
    digitalWrite(spark, LOW);
    if ( current_temp < switchTemp ) {
      digitalWrite(blue, HIGH);
      digitalWrite(red, LOW);
    } else {
      digitalWrite(blue, LOW);
      digitalWrite(red, HIGH);
    }
    delay(5000);
}

// Read temp with OneWire class, via @wgbartley gist
float getTemp() {
    uint8_t rom[8];
    uint8_t resp[9];
    
    // Get the ROM address
    one.reset();
    one.write(0x33);
    one.read_bytes(rom, 8);
    
    // Get the temp
    one.reset();
    one.write(0x55);
    one.write_bytes(rom,8);
    one.write(0x44);
    delay(10);
    one.reset();
    one.write(0x55);
    one.write_bytes(rom, 8);
    one.write(0xBE);
    one.read_bytes(resp, 9);
    
    byte MSB = resp[1];
    byte LSB = resp[0];
    
    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    float TemperatureSum = tempRead / 16;
    return TemperatureSum;
}

int getTempF(String command) {
    return ((getTemp() * 9.0) / 5.0 + 32.0)*100;
}

int setSwitchTemp(String temp) {
  switchTemp = temp.toInt();
  return switchTemp;
}

int flashLED(String command) {
  if ( strcmp("red", command.c_str()) == 0 ) {
    flashRed();
  }

  if ( strcmp("blue", command.c_str()) == 0 ) {
    flashBlue();
  }

  return 1;
}

int flashRed() {
  digitalWrite(red, LOW);
  delay(300);
  digitalWrite(red, HIGH);
  delay(300);
  digitalWrite(red, LOW);
  delay(300);
  digitalWrite(red, HIGH);
  delay(300);
  digitalWrite(red, LOW);
  return 0;
}

int flashBlue() {
  digitalWrite(blue, LOW);
  delay(300);
  digitalWrite(blue, HIGH);
  delay(300);
  digitalWrite(blue, LOW);
  delay(300);
  digitalWrite(blue, HIGH);
  delay(300);
  digitalWrite(blue, LOW);
  return 0;
}