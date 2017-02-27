#include <SoftwareSerial.h>
#include <UTFT_SPI.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include <Base64.h>
#include "memorysaver.h"
#include "ov5642_regs.h"

// set pin 10 as the slave select for the digital pot:
const int SPI_CS = 10;


ArduCAM myCAM(OV5642,10);
UTFT myGLCD(SPI_CS);
SoftwareSerial bluetooth(2,3); //bluetooth connection
void setup()
{
  uint8_t vid,pid;
  uint8_t temp;
  #if defined (__AVR__)
    Wire.begin(); 
  #endif
  #if defined(__arm__)
    Wire1.begin(); 
  #endif
//  bluetooth.begin(115200);
  bluetooth.begin(9600);
//  Serial.begin(115200);
  delay(200);
//  bluetooth.println("ArduCAM Start!"); 
//  Serial.println("ArduCAM Start!"); 
  delay(1000);

  // set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
//    bluetooth.println("SPI interface Error!");
//    Serial.println("SPI interface Error!");
    while(1);
  }
  
  //Change MCU mode
  myCAM.set_mode(MCU2LCD_MODE);

  //Check if the camera module type is OV5642
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42))
//    bluetooth.println("Can't find OV5642 module!");
    Serial.println("Can't find OV5642 module!");
  else
//    bluetooth.println("OV5642 detected");
    Serial.println("OV5642 detected");
  
  //Change to JPEG capture mode and initialize the OV5642 module  
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV5642_set_JPEG_size(OV5642_640x480);
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);    //VSYNC is active HIGH
}

void loop()
{
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;
  char input = bluetooth.read(); // read from the bluetooth input
  if(input == 'a'){
    start_capture = 1;
//      bluetooth.println("Start Capture");  
//      Serial.println("Start Capture");  
      myCAM.flush_fifo();
  }
  delay(1000);
  //start_capture = 1;
  if(start_capture)
  {
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();   
    //Start capture
    myCAM.start_capture();   
  }
  if(myCAM.get_bit(ARDUCHIP_TRIG,CAP_DONE_MASK))
  {
//    bluetooth.println("Capture Done!");
   
    int inputLen = 0;
    
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
        temp_last = temp;
        temp = myCAM.read_fifo();
        bluetooth.write(temp);
//        Serial.write(temp);
    }
//    Serial.println("Capture Done!");
    delay(2000);
    bluetooth.println("done");   
    // send the battery info
    bluetooth.println("30");
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    start_capture = 0;
  }
}
