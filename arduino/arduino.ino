// includes
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <LiquidMenu.h>
#include <DHT.h>

// pins
// A4 and A5 automatically used by LCD
#define PIN_BUTTON_MENU 4
#define PIN_BUTTON_MENU_NEXT 5
#define PIN_BUTTON_MENU_CHANGE 6
#define DHT_PIN 8
#define REED_PIN 2
#define BATTERY_PIN A1

// constants
#define CALIBRATED_ARDUINO_VOLTAGE 5.0
#define CALIBRATED_VOLTAGE_DIVIDER_FACTOR 12.0

// lcd
LiquidCrystal_I2C lcd(0x27,20,4);

// buttons
Bounce button_menu = Bounce();
Bounce button_menu_next = Bounce();
Bounce button_menu_change = Bounce();

// dht
DHT dht(DHT_PIN, DHT11);

// variables
bool backlight_on = false;
float distance = 0.0;
float speed = 0.0;
float temperature = 0.0;
char* temperature_units = "F   ";
char* temperature_name = "Temp:     ";
float battery = 0.0;
char* battery_units = "V   ";
float battery_percent = 0.0;
unsigned long menu_main_last_update = 0;
volatile unsigned long distance_total_revolutions = 0;
volatile unsigned long speed_revolutions = 0;

// menu
LiquidLine menu_main_battery(0, 0, "Battery:  ", battery, battery_units);
LiquidLine menu_main_speed(0, 1, "Speed:    ", speed, "  ");
LiquidLine menu_main_distance(0, 2, "Distance: ", distance, "  ");
LiquidLine menu_main_temperature(0, 3, temperature_name, temperature, temperature_units);
LiquidScreen menu_main(menu_main_battery, menu_main_speed, menu_main_distance, menu_main_temperature);
LiquidLine menu_options_togglebacklight(2, 0, "Toggle Backlight");
LiquidLine menu_options_resetdistance(2, 1, "Reset Distance");
LiquidScreen menu_options(menu_options_togglebacklight, menu_options_resetdistance);
LiquidMenu menu(lcd);

// setup
void setup(){
  // setup the lcd
  lcd.init();
  lcd.backlight();
  backlight_on = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("booting...");

  // start dht
  dht.begin();

  // reed switch
  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), reed_switch_interrupt, FALLING);

  // setup the buttons
  button_menu.attach(PIN_BUTTON_MENU, INPUT_PULLUP);
  button_menu_next.attach(PIN_BUTTON_MENU_NEXT, INPUT_PULLUP);
  button_menu_change.attach(PIN_BUTTON_MENU_CHANGE, INPUT_PULLUP);
  button_menu.interval(25);
  button_menu_next.interval(25);
  button_menu_change.interval(25);

  // menu
  menu_main_battery.set_decimalPlaces(2);
  menu_main_speed.set_decimalPlaces(2);
  menu_main_distance.set_decimalPlaces(2);
  menu_main_temperature.set_decimalPlaces(2);
  menu_options_togglebacklight.attach_function(1, on_toggle_backlight);
  menu_options_resetdistance.attach_function(1, on_reset_distance);
  menu_options.set_focusPosition(Position::LEFT);
  menu.add_screen(menu_main);
  menu.add_screen(menu_options);
}

// loop
void loop(){
  // update the buttons
  button_menu.update();
  button_menu_next.update();
  button_menu_change.update();

  // check for button presses
  if (button_menu.fell()){
    on_button_menu();    
  }
  else if (button_menu_next.fell()){
    on_button_menu_next();
  }
  else if (button_menu_change.fell()){
    on_button_menu_change();
  }
  
  // see if time to update the main menu
  unsigned long now = millis();
  unsigned long delta = now - menu_main_last_update;
  if (delta >= 1000){
    // update battery
    // get the voltage
    float volts = analogRead(BATTERY_PIN) / 1023.0 * CALIBRATED_ARDUINO_VOLTAGE * CALIBRATED_VOLTAGE_DIVIDER_FACTOR;
    // every 3 seconds cycle between voltage and percent
    if (now % 6000 < 3000){
      battery = volts;
      battery_units = "V   ";
    }
    else{
      battery = volts * 100.0 / 48.0;
      battery_units = "%   ";
    }

    // update speed
    // num_rev * circumference_in_feet / feet_per_mile * seconds_per_hour
    float wheel_radius_in_feet = 0.5;
    speed = speed_revolutions * (3.1415927 * 2 * wheel_radius_in_feet) / 5280 * 3600;
    speed_revolutions = 0;

    // update distance
    // num_rev * circumference_in_feet / feet_per_mile
    distance = distance_total_revolutions * (3.1415927 * 2 * wheel_radius_in_feet) / 5280;
        
    // update the temp
    // every 3 seconds cycle temp, humidity, heat index
    if (now % 9000 < 3000){
      // temp
      temperature_name = "Temp:     ";
      temperature_units = "F   ";
      temperature = dht.readTemperature(true);    
    }
    else if (now % 9000 < 6000){
      // humidity
      temperature_name = "Humidity: ";
      temperature_units = "%   ";
      temperature = dht.readHumidity();
    }
    else{
      // heat index
      temperature_name = "HeatIndx: ";
      temperature_units = "F   ";
      temperature = dht.computeHeatIndex(true);
    }

    // show on the menu
    menu.softUpdate();

    // update the last time updated
    menu_main_last_update = now;
  }
}

void on_button_menu(){
  menu.next_screen();
}

void on_button_menu_next(){
  menu.switch_focus();
//  menu.update();
}

void on_button_menu_change(){
  if (menu.is_callable(1)){
    menu.call_function(1);
  }
}

void on_toggle_backlight(){
  if (backlight_on){
    lcd.noBacklight();
    backlight_on = false;  
  }
  else{
    lcd.backlight();
    backlight_on = true;
  }
}

void on_reset_distance(){
  distance_total_revolutions = 0.0;  
}

void reed_switch_interrupt(){
    // ensure there is at least 25 milliseconds between readings...for debounce
    static unsigned long last_millis = 0;
    unsigned long now = millis();
    if (now - last_millis >= 25){
        distance_total_revolutions += 1;
        speed_revolutions += 1;
        last_millis = now;
    }
}
