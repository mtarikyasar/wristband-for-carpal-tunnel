#include <Wire.h>

#define DEVICE_A (0x1D)    //first ADXL345 device address
#define DEVICE_B (0x53)    //second ADXL345 device address
#define TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)

byte buff[TO_READ] ;      //6 bytes buffer for saving data read from the device
char str[512];            //string buffer to transform data before sending it to the serial port

int regAddress = 0x32;      //first axis-acceleration-data register on the ADXL345
int xa = 0, ya = 0, za = 0;  
int xb = 0, yb = 0, zb = 0;

void writeTo(int device, byte address, byte val) {
   Wire.beginTransmission(device); //start transmission to device 
   Wire.write(address);        // send register address
   Wire.write(val);        // send value to write
   Wire.endTransmission(); //end transmission
}

void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  //Turning on the both ADXL345s
  writeTo(DEVICE_A, 0x2D, 24);   
  writeTo(DEVICE_B, 0x2D, 24);
}
  
void loop()
{  
  readFrom(DEVICE_A, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345  
   //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
   //thus we are converting both bytes in to one int
  xa = (((int)buff[1]) << 8) | buff[0];   
  ya = (((int)buff[3])<< 8) | buff[2];
  za = (((int)buff[5]) << 8) | buff[4];
  
  readFrom(DEVICE_B, regAddress, TO_READ, buff); //read the acceleration data from the second ADXL345
  xb = (((int)buff[1]) << 8) | buff[0];   
  yb = (((int)buff[3])<< 8) | buff[2];
  zb = (((int)buff[5]) << 8) | buff[4];
  
  //we send the x y z values as a string to the serial port
  sprintf(str, "X1:%d Y1:%d Z1:%d X2:%d Y2:%d Z3:%d", xa, ya, za, xb, yb, zb);  
  Serial.print(str);
  Serial.write(10);
  
  //It appears that delay is needed in order not to clog the port
  delay(2500);
}
