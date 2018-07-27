// #
// # Editor     : Lauren from DFRobot
// # Date       : 08.05.2012

// # Product name: URM V3.2 ultrasonic sensor TTL connection with Arduino
// # Product SKU : SEN0001
// # Version     : 1.0
// # Update the library to make it compatible with the Arduino IDE 1.0 or the latest version

// # Description:
// # The sketch for using the URM37 from DFRobot to read values (0-300) from an ultrasound sensor (3m sensor)
// #   and writes the values to the serialport

// # Connection:
// #       Pin12 (Arduino) -> Pin 1 VCC (URM V3.2)
// #       GND (Arduino)   -> Pin 2 GND (URM V3.2)
// #       Pin 0 (Arduino) -> Pin 9 TXD (URM V3.2)
// #       Pin 1 (Arduino) -> Pin 8 RXD (URM V3.2)
// #
//#include <cy_serdebug.h>
//#include <cy_serial.h>
//#define serdebug
#ifdef serdebug
#define DebugPrint(...) {  Serial.print(__VA_ARGS__); }
#define DebugPrintln(...) {  Serial.println(__VA_ARGS__); }
#else
#define DebugPrint(...) { }
#define DebugPrintln(...) { }
#endif

const char *gc_hostname = "d1miniurm";

#include "cy_wifi.h"
#include "cy_ota.h"
#include "cy_mqtt.h"
#include <Ticker.h>
#include "cy_weather.h"

const char* mqtt_pubtopic = "ATSH28/AUSSEN/WATERLEVEL/1/value";

int URPower = 12; // Ultrasound power pin
int val = 0;
int USValue = 0;
float gv_dist;
int timecount = 0; // Echo counter
boolean lv_flag_data = false;
boolean flag = true;
uint8_t DMcmd[4] = {
  0x22, 0x00, 0x00, 0x22
}; //distance measure command


Ticker senstick;
boolean gv_senstick;

void do_senstick() {
  gv_senstick = true;
}

void setup() {

  Serial.begin(9600);                  // Sets the baud rate to 9600
  //Serial.println("Init the sensor");

  wifi_init(gc_hostname);

  init_ota(gv_clientname);

  init_mqtt(gv_clientname);

  delay(1000);
  check_mqtt();
  delay(1000);

  //pinMode(URPower, OUTPUT);
  //digitalWrite(URPower, HIGH); // Set to High
  //delay(200); //Give sensor some time to start up --Added By crystal  from Singapo, Thanks Crystal.


  do_sensor();
  send_pub_vals();
  gv_senstick = false;
  senstick.attach(60, do_senstick);
}

void loop()
{

  if (gv_senstick == true) {
    do_sensor();

    send_pub_vals();

    gv_senstick = false;
  }

  check_ota();

  check_mqtt();

  delay(500);

}


void do_sensor() {

  get_urm37();

}

void send_pub_vals() {
  send_val(3, gv_dist);

  char buffer[10];
  dtostrf(gv_dist, 0, 1, buffer);
  client.publish(mqtt_pubtopic, buffer, true);
}

void get_urm37() {
  lv_flag_data = false;
  flag = true;
  //Sending distance measure command :  0x22, 0x00, 0x00, 0x22 ;

  //  Serial.swap();

  for (int i = 0; i < 4; i++)
  {
    Serial.write(DMcmd[i]);
  }

  delay(40); //delay for 75 ms
  unsigned long timer = millis();
  while (millis() - timer < 30)
  {
    if (Serial.available() > 0)
    {
      int header = Serial.read(); //0x22
      int highbyte = Serial.read();
      int lowbyte = Serial.read();
      int sum = Serial.read(); //sum

      if (header == 0x22) {
        if (highbyte == 255) {
          USValue = 65525; //if highbyte =255 , the reading is invalid.
        } else  {
          USValue = highbyte * 255 + lowbyte;
        }
        lv_flag_data = true;
        flag = false;
      }
      else {
        while (Serial.available())  byte bufferClear = Serial.read();
        break;
      }
    }
  }

  //Serial.swap();

  if (lv_flag_data == true ) {
    //Serial.print("Distance=");
    //Serial.println(USValue);
    //send_val(3, USValue);
    if (USValue < 230) {
      gv_dist = USValue * -1;
    } else {
      gv_dist = 230;
    }
  }
  else {
    //Serial.print("No Data");
  }

  delay(20);

}
