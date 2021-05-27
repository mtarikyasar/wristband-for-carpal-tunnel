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
int button_pin = 5;
int change_interface_button_pin = 6;

int button_state = 0;
int interface_state = 0; // 0 for wrist, 1 for pulse sensor
int interface = 0;

int const pulse_sensor_pin = 0;
PulseSensorPlayground pulseSensor;

int x=0;
int lastx=0;
int lasty=0;
int LastTime=0;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;

void setup(void) 
{
  pinMode(wrist_position_correct, OUTPUT);
  pinMode(wrist_position_wrong, OUTPUT);
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

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(60, 25);
  display.println("Hi!");
  display.display();
  delay(500);
  display.clearDisplay();
}

void loop(void) 
{
  interface_state = digitalRead(change_interface_button_pin);
  int myBPM = pulseSensor.getBeatsPerMinute();
  
  if (!interface_state) {
    Serial.println("Changing mods...");
    display.clearDisplay();
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

    // Reset flag
    flag = 0;
  }
  
  if (interface == 0){
   button_state = digitalRead(button_pin);

   if (flag) {
    sensors_event_t event; 
    accel.getEvent(&event);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Save      Change Mod");
    display.setCursor(50, 25);
    display.println("Saved!");
    display.display();
   
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
    Serial.println("m/s^2 ");
   
    int difference_x = abs(event.acceleration.x) - abs(saved_x);
    int difference_y = abs(event.acceleration.y) - abs(saved_y);
    int difference_z = abs(event.acceleration.z) - abs(saved_z);

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
      display.invertDisplay(true);
    }
   } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Save      Change Mod");
      display.setCursor(0, 25);
      display.println("Please press 'Save' button to calibrate");
      display.display();
   }

   if(!button_state){
    sensors_event_t event; 
    accel.getEvent(&event);

    flag = 1;

    saved_x = event.acceleration.x;
    saved_y = event.acceleration.y;
    saved_z = event.acceleration.z;
    
    Serial.print("SAVED!: ");
    Serial.print(saved_x);
    Serial.print(saved_y);
    Serial.print(saved_z);
    Serial.println("");
   }
  } else {
      if(x>127){
        display.clearDisplay();
        x=0;
        lastx=x;
      }
 
      int value=analogRead(pulse_sensor_pin);
      display.setTextColor(WHITE);
      int y=60-(value/16);
      display.writeLine(lastx,lasty,x,y,WHITE);
      lasty=y;
      lastx=x;
     
      if(value>550) {
        if(BeatComplete) {
          BPM=millis()-LastTime;
          BPM=int(60/(float(BPM)/1000));
          BPMTiming=false;
          BeatComplete=false;
        }
        if(BPMTiming==false) {
          LastTime=millis();
          BPMTiming=true;
        }
      }
      if((value<500)&(BPMTiming))
        BeatComplete=true;
        
        // display bpm
        display.setCursor(0, 0);
        display.println("Save      Change Mod");
        display.writeFillRect(0,50,128,16,BLACK);
        display.setCursor(0,50);
        display.print(BPM);
        display.print(" BPM");
        display.display();
        x++;
  } 
}
