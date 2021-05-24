#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PulseSensorPlayground.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

double saved_x = 0, saved_y = 0, saved_z = 0;
double current_x = 0, current_y = 0, current_z = 0;
int flag = 0;

int wrist_position_correct = 2;
int wrist_position_wrong = 3;
int saved_led = 4;
int button_pin = 5;
int change_interface_button_pin = 6;

int button_state = 0;
int interface_state = 0; // 0 for wrist, 1 for pulse sensor
int interface = 0;

int const pulse_sensor_pin = 0;
PulseSensorPlayground pulseSensor;

int x = 0;
int lastx = 0;
int lasty = 0;
int LastTime=0;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;

void setup(void) 
{
  pinMode(wrist_position_correct, OUTPUT);
  pinMode(wrist_position_wrong, OUTPUT);
  pinMode(saved_led, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(change_interface_button_pin, INPUT_PULLUP);

  pulseSensor.analogInput(pulse_sensor_pin);
  pulseSensor.setThreshold(550);
  
   Serial.begin(9600);  

   if(!accel.begin())
   {
      Serial.println("No ADXL345 sensor detected.");
      while(1);
   }

   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

   digitalWrite(saved_led, HIGH);
   display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
}

void loop(void) 
{
  interface_state = digitalRead(change_interface_button_pin);
  int myBPM = pulseSensor.getBeatsPerMinute();
  Serial.print("Button Pin: ");
  Serial.println(digitalRead(button_pin));
  Serial.print("Interface Pin: ");
  Serial.println(digitalRead(change_interface_button_pin));
  if (!interface_state) {
    Serial.println("Changing mods...");
    // Then interface will change
    if (interface == 1) {
      interface = 0;
    } else {
      interface = 1;
    }
    
    delay(500);

    // Resets the leds
    digitalWrite(wrist_position_correct, LOW);
    digitalWrite(wrist_position_wrong, LOW);
    digitalWrite(saved_led, HIGH);

    // Reset flag
    flag = 0;
  }
  
  if (interface == 1){
   sensors_event_t event; 
   accel.getEvent(&event);
   current_x = event.acceleration.x;
   current_y = event.acceleration.y;
   current_z = event.acceleration.z;
   
   display.clearDisplay();
   display.setCursor(0, 0);
   display.println("Saved wrist position: ");
   display.print("x: "); display.print(saved_x); display.print(";");
   display.print("y: "); display.print(saved_y); display.print(";");
   display.print("z: "); display.print(saved_z); display.println(";");
   display.println(button_state);
   display.display();
   
   Serial.print("X: "); Serial.print(current_x); Serial.print("  ");
   Serial.print("Y: "); Serial.print(current_y); Serial.print("  ");
   Serial.print("Z: "); Serial.print(current_z); Serial.print("  ");
   Serial.println("m/s^2 ");

   button_state = digitalRead(button_pin);
   //delay(1000);

   if (flag) {
    digitalWrite(saved_led, LOW);
    
    int difference_x = abs(current_x) - abs(saved_x);
    int difference_y = abs(current_y) - abs(saved_y);
    int difference_z = abs(current_z) - abs(saved_z);

    if (difference_y < 0.5){
      digitalWrite(wrist_position_correct, HIGH);
      digitalWrite(wrist_position_wrong, LOW);
      display.invertDisplay(false);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Correct");
    }
    else {
      digitalWrite(wrist_position_correct, LOW);
      digitalWrite(wrist_position_wrong, HIGH);
      display.clearDisplay();
      display.setCursor(0, 0);
      delay(100);
//      for (int i = 0; i < 128; i++){
//        for (int j = 0; j < 64; j++){
//          display.drawPixel(i, j, WHITE);
//        }
//      }
      display.invertDisplay(true);
    }
   }

   if(!button_state){
    flag = 1;
    saved_x = current_x;
    saved_y = current_y;
    saved_z = current_z;
    
    Serial.print("SAVED!: ");
    Serial.print(saved_x);
    Serial.print(saved_y);
    Serial.print(saved_z);
    Serial.println("");
   }
  } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Heart Rate: ");
      display.println(pulseSensor.getBeatsPerMinute());
      display.print("Heart Rate: ");
      display.println(analogRead(pulse_sensor_pin));
      display.display();
      delay(20);
    }
   
}
