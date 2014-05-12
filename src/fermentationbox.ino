#include "application.h"

// Include the port of OneWire
// via https://gist.github.com/wgbartley/8301191
#include "OneWire.h"

// DS18B20 temp sensor DATA is on D1
OneWire one = OneWire(D1);

int spark = D7; // The mini Spark LED
int blue = D3; // Our cold LED
int red = D0; // Our hot LED

void setup() {
  Spark.function("tempC", getTempC);
  Spark.function("tempF", getTempF);

  pinMode(D3, OUTPUT);
  pinMode(D0, OUTPUT);
}

void loop() {
    int current_temp = ((getTemp() * 9.0) / 5.0 + 32.0)*100;
    digitalWrite(spark, LOW);
    delay(300);
    digitalWrite(spark, HIGH);
    delay(300);
    digitalWrite(spark, LOW);
    if ( current_temp < 66 ) {
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

int getTempC(String command) {
    return getTemp()*100;
}

int getTempF(String command) {
    return ((getTemp() * 9.0) / 5.0 + 32.0)*100;
}