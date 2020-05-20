//includes
#include "LiquidCrystal_I2C.h"
#include "DHT.h"

// pins
#define DHTPIN 4
#define REEDPIN 3
#define VOLTPIN A1

// create the lcd
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7);

// create temperature / humidty sensor
DHT dht(DHTPIN, DHT11);

// reed sensor
volatile int reed_count;
volatile unsigned long reed_last_micros;
unsigned long reed_last_mph_conversion;

void setup(){
  // setup lcd
  lcd.begin(20,4);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.setCursor(0, 2);
  lcd.print("Count: ");
  lcd.setCursor(0, 3);
  lcd.print("Battery: ");

  // reed count
  reed_last_micros = micros();
  reed_count = 0;
  reed_last_mph_conversion = 0;
  pinMode(REEDPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REEDPIN), on_reed_switch_trigger, FALLING);

  // dht
  dht.begin();

  Serial.begin(9600);
}
void loop()
{
  // get the temperature
  float temp = dht.readTemperature(true);
  lcd.setCursor(13, 0);
  lcd.print(temp);

  // get the humidity
  float humidity = dht.readHumidity();
  lcd.setCursor(10, 1);
  lcd.print(humidity);

  // convert reed count to mph
  unsigned long micros_now = micros();
  unsigned long delta = micros_now - reed_last_mph_conversion;
  reed_last_mph_conversion = micros_now;
  float rpm = (reed_count / (float)delta) * 1000000 * 60;
  Serial.println(micros_now);
  Serial.println(delta);
  Serial.println(reed_count);
  Serial.println(rpm);
  reed_count = 0;
  lcd.setCursor(7, 2);
  lcd.print(rpm);

  // volts
  int val = analogRead(VOLTPIN);
  float voltage = val * (5.0 / 1023.0);
  lcd.setCursor(9, 3);
  lcd.print(voltage);

  // wait 1 seconds
  delay(1000);
}

void on_reed_switch_trigger(){
  // ensure there is at least 10 milliseconds between readings...for debounce
  if (micros() - reed_last_micros >= 10000){
    reed_count += 1;
    reed_last_micros = micros();
  }
}
