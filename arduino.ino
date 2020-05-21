// includes
#include "LiquidCrystal_I2C.h"
#include "DHT.h"

// pins
// A4 and A5 are automatically used by LCD
#define BATTERY_PIN A1
#define REED_PIN D2
#define BUTTON_MENU_PIN D3
#define BUTTON_SELECT_PIN D4
#define BUTTON_CHANGE_PIN D5
#define DHT_PIN D9

// create the lcd
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7);

// create temperature / humidty sensor
DHT dht(DHTPIN, DHT11);

// reed sensor
volatile int reed_count;
volatile unsigned long reed_last_micros;
unsigned long reed_last_mph_conversion;
#define REED_DEBOUNCE_DELAY = 25000

// menu displayed
#define MENU_NORMAL     0
#define MENU_BACKLIT    1
#define MENU_ODOMETER   2
#define MENU_END        3
volatile int menu_displayed;
#define BUTTON_DEBOUNCE_DELAY = 100000
unsigned long button_last_micros;

void setup(){
    // setup lcd
    lcd.begin(20,4);
    lcd.setBacklightPin(3,POSITIVE);
    lcd.setBacklight(HIGH);
    lcd.clear();

    // reed count
    reed_last_micros = micros();
    reed_count = 0;
    reed_last_mph_conversion = 0;
    pinMode(REED_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(REED_PIN), reed_switch_interrupt, FALLING);

    // dht
    dht.begin();

    // setup buttons
    button_last_micros = micros();
    pinMode(BUTTON_MENU_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_CHANGE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_MENU_PIN), change_menu_interrupt, FALLING);

    // enable serial port monitoring
//    Serial.begin(9600);

    // start off in the normal menu
    set_menu_normal();
}

void loop(){
    switch (menu_displayed){
        case MENU_NORMAL: set_menu_normal(); break;
        case MENU_BACKLIT: set_menu_backlit(); break;
        case MENU_ODOMETER: set_menu_odometer(); break;
    }
}

void loop_menu_normal(){
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

void loop_menu_backlit(){

}

void loop_menu_odometer(){

}

void set_menu_normal(){
    // set it
    menu_displayed = MENU_NORMAL;

    // setup the display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.setCursor(0, 2);
    lcd.print("Count: ");
    lcd.setCursor(0, 3);
    lcd.print("Battery: ");
}

void set_menu_backlit(){
    // set it
    menu_displayed = MENU_BACKLIT;

    // setup the display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BACKLIT MENU");
}

void set_menu_odometer(){
    // set it
    menu_displayed = MENU_ODOMETER;

    // setup the display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ODOMETER MENU");
}

void change_menu_interrupt(){
    // ensure there is at least 250 milliseconds between readings...for debounce
    if (micros() - change_menu_last_micros < BUTTON_DEBOUNCE_DELAY){
        // not enough time passed
        return;
    }

    // update the last micros
    change_menu_last_micros = micros();

    // increment to next menu
    menu_displayed += 1;

    // check if at end
    if (menu_displayed >= MENU_END){
        menu_displayed = MENU_NORMAL;
    }

    // change to the appropriate menu
    switch (menu_displayed){
        case MENU_NORMAL: set_menu_normal(); break;
        case MENU_BACKLIT: set_menu_backlit(); break;
        case MENU_ODOMETER: set_menu_odometer(); break;
    }
}

void reed_switch_interrupt(){
    // ensure there is at least 10 milliseconds between readings...for debounce
    if (micros() - reed_last_micros >= REED_DEBOUNCE_DELAY){
        reed_count += 1;
        reed_last_micros = micros();
    }
}
