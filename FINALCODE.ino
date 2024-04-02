#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "KellerLD.h"

KellerLD sensor;
SoftwareSerial CO2Serial(2,3);   //Rx, Tx
SoftwareSerial CO2Serial2(4,5);   //Rx, Tx
unsigned char hexData[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};  // read gas density command /dont change order

//change this to match your sd module
const int chipSelect = 10;

int solenoidpin = 7;  //Set valve and arduino connection to pin 7
File dataFile;  //CO2 #1  
File dataFile2; //CO2 #2
File dataFile3; //Pressure
File dataFile4; //Valve state

void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(9600);
  // check for error in SD card
  if (!SD.begin(chipSelect)) {       
    Serial.println("Card initialization failed!"); //error in SD card initialization
    return;
  }
  //print card initialized if no error
  Serial.println("Card initialized.");

  CO2Serial.begin(9600);  //initializes serial communication between first CO2 sensor and arduino

  Wire.begin(); //initialize I2C communication                            //needed for data(SDA) and clock(SCL)
  sensor.init(); //pressure sensor initialization
  sensor.setFluidDensity(997);   // kg/m^3 (freshwater, 1029 for seawater)

  //check for pressure sensor initialization
  if (sensor.isInitialized()){  
    Serial.println("Sensor connected"); //initialized
  } else {
    Serial.println("Sensor not connected"); //not initialized
  }

  pinMode(solenoidpin, OUTPUT); //set pin for valve as output
  digitalWrite(solenoidpin, HIGH); //Open valve
  
  //Sd card file reads for checking with computer

  //CO2 sensor #1 check
  dataFile = SD.open("data.csv");
    if (dataFile) {
      Serial.println("data.csv:");

      // read from the file until there's nothing else in it:
      while (dataFile.available()) {
        Serial.write(dataFile.read());
    }
    // close the file:
    dataFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.csv");
  }
  //CO2 sensor #2 check
  dataFile2 = SD.open("data2.csv");
    if (dataFile2) {
      Serial.println("data2.csv:");

      // read from the file until there's nothing else in it:
      while (dataFile2.available()) {
        Serial.write(dataFile2.read());
    }
    // close the file:
    dataFile2.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data2.csv");
  }
  //Pressure check
  dataFile3 = SD.open("data3.csv");
    if (dataFile3) {
      Serial.println("data3.csv:");

      // read from the file until there's nothing else in it:
      while (dataFile3.available()) {
        Serial.write(dataFile3.read());
    }
    // close the file:
    dataFile3.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data3.csv");
  }
  //Valve check
  dataFile4 = SD.open("data4.csv");
    if (dataFile4) {
      Serial.println("data4.csv:");

      // read from the file until there's nothing else in it:
      while (dataFile4.available()) {
        Serial.write(dataFile4.read());
    }
    // close the file:
    dataFile4.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data4.csv");
  }

  //delete files on sd cards for reruns (not required)
  SD.remove("data.csv");
  SD.remove("data2.csv");
  SD.remove("data3.csv");
  SD.remove("data4.csv");
  
}

void loop() {
  int closed = 0;  //valve state variable
  long CO2sens1;  //CO2 sensor #1 variable 
  long CO2sens2;  //CO2 sensor #2 variable 
  sensor.read();  //reads data from sensor
  delay(500);
  int pressure = sensor.pressure();  //retrieves pressure data
  String pdata = String(pressure);
  if (sensor.pressure() > 1100){ // should open around 2.1-2.2 meters underwater
    digitalWrite(solenoidpin, LOW); //close valve
    closed = 1; // set variable to print valve state
   }
  
  //prints pressure in mbar
  
  Serial.print("Pressure: "); 
  Serial.print(sensor.pressure()); 
  Serial.println(" mbar");

  CO2Serial.write(hexData,9);
  delay(500);
  
  //CO2 #1
  for (int i = 0; i < 9; i++)
  {
    if (CO2Serial.available() > 0)  //if sensor setup properly run
    {
      long hi, lo, CO2;
      int ch = CO2Serial.read();  //read values from sensor

      if (i == 2) {
        hi = ch;  //High concentration
      }
      if (i == 3) {
        lo = ch;  //Low concentration
      }
      if (i == 8){
        CO2sens1 = hi * 256 + lo;  //CO2 concentration
      
      // Print sensor values
      Serial.print("CO2 concentration:");
      Serial.print(CO2sens1);
      Serial.println("ppm");
      CO2Serial2.begin(9600); //Open communications with second CO2 sensor
      }

    } else {
      Serial.println("CO2 sensor1 not working");  //print error if not working
    }
  } 
  CO2Serial2.write(hexData, 9);
  delay(500);
  
  //CO2 #2
  for (int i = 0, j = 0; i < 9; i++)
  {
    if (CO2Serial2.available() > 0) //If sensor is connected properly run
    {
      long hi, lo;
      int ch = CO2Serial2.read(); //read values from sensor
    
      if (i == 2) {
        hi = ch;    //High concentration
      }
      if (i == 3) {
        lo = ch;    //Low concentration
      }
      if (i == 8) {
        CO2sens2 = hi * 256 + lo; //CO2 concentration
        Serial.print("CO2 concentration2: ");
        Serial.print(CO2sens2);
        Serial.println("ppm");
        CO2Serial.begin(9600); //start communication with first sensor again
      }
    }
  }
  String data = String(CO2sens1);
  String Sensor2 = String(CO2sens2);
  //Call functions to write to files
  writeDataToCSV(data);
  writeDataToCSV2(Sensor2);
  writeDataToCSV3(pdata);
  writeDataToCSV4(closed);
}
//Write CO2 #1 values
void writeDataToCSV(String data) {
  //Open the file
  dataFile = SD.open("data.csv", FILE_WRITE);
  //If file opened okay, write to it
  if (dataFile) {
    dataFile.println(data);
    //Close file
    dataFile.close();
  }
  else {
    //If file didn't open, print an error
    Serial.println("error opening data.csv");
  }
}
//Write CO2 #2 values
void writeDataToCSV2(String Sensor2) {
  //Open the file
  dataFile2 = SD.open("data2.csv", FILE_WRITE);
  //If file opened okay, write to it
  if (dataFile2) {
    dataFile2.println(Sensor2);
    //Close file
    dataFile2.close();
  }
  else {
    //If file didn't open, print an error
    Serial.println("error opening data2.csv");
  }
}
//Write pressure values
void writeDataToCSV3(String pdata) {
  //Open the file
  dataFile3 = SD.open("data3.csv", FILE_WRITE);
  if (dataFile3) {
    dataFile3.println(pdata);
    //Close file
    dataFile3.close();
  }
  //If file opened okay, write to it
  else {
    //If file didn't open, print an error
    Serial.println("error opening data3.csv");
  }
}
//Write 1 to file if valve closed
void writeDataToCSV4(int closed) {
  //Open the file
  dataFile4 = SD.open("data4.csv", FILE_WRITE);
  //If file opened okay, write to it
  if (dataFile4 && closed == 1) {
    dataFile4.println("1");
    //Close file
    dataFile4.close();
  }
  if (dataFile4 && closed == 0){
    dataFile4.println("0");
    //Close file
    dataFile4.close();
  }
  else {
    //If file didn't open, print an error
    Serial.println("error opening data4.csv");
  }
}
