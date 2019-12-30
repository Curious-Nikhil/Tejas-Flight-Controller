// =============================
// == PHASE THREE Devs ====
//==============================

/*Tejas V3 - @Nikhil Mishra
   https://curious-nikhil.github.io/
   Tejas is a rocket computer
*/
// =========================================
// ==         CONFIGURATION              ==
//==========================================


//#define SERVO_MOD
//#define DMP_MOD
#define SERIAL_DEBUG
//#define MPU_CALBIRATION
//#define SOUND
// =============================
// == Include and Define Vars ==
//==============================

//Header Files
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Adafruit_BMP280.h"
#include <Wire.h>
#include "SimpleKalmanFilter.h"
#include <SD.h>


#define INTERRUPT_PIN 2  // 

#define RLED 8// RED  LED
#define GLED 7// Green LED
#define BLED 6 // BLue LED
#define buzzer 5
#define pyroPin 9
// =============================================
// ===          MISC Global Vars             ===
// =============================================

//Timers Vars
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long landprev = 0;
unsigned long currentMillis;
bool launch = 0;
bool pyro = false;
bool pyroFired = false;
bool landed = false;
bool ABORT = false;
bool landcheck = false;
int lastAlt = 0;

// =============================================
// ===              MPU Vars                 ===
// =============================================

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int8_t threshold, count;

float temp;
bool zero_detect;
bool TurnOnZI = false;

bool XnegMD, XposMD, YnegMD, YposMD, ZnegMD, ZposMD;
// =============================================
// ===              BAROMETER                ===
// =============================================

Adafruit_BMP280 bmp280; // I2C
//SimpleKalmanFilter pressureKalmanFilter(1, 1, 0.01);
const int sealvl = 1013.5;
static float alt;
float est_alt;
float temperature;
float pascal;
float base_alt;
// =============================================
// ===              SD CARD                  ===
// =============================================
String filename;
File myFile;
int sd_count = 0;
bool FL = false;



// ================================================================
// ===               THE SETUP FUNCTION                       ===
// ================================================================
void setup() {

  Serial.begin(38400);

  #ifdef SOUND
  //Startup Sound
  tone(buzzer, 2500, 300);
  delay(1000);
  tone(buzzer, 2000, 300);
  delay(500);
  tone(buzzer, 3000, 300);
  tone(buzzer, 3500, 300);
  delay(500);
  tone(buzzer, 2500, 300);
  delay(1000);
  delay(2000);
  #endif
  // configure LED for output
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(BLED, OUTPUT);
  pinMode(pyroPin, OUTPUT);

  digitalWrite(RLED, HIGH);
  digitalWrite(GLED, HIGH);
  digitalWrite(BLED, HIGH);
  digitalWrite(pyroPin, LOW);


  // PASS 2: Initialize SD Module
  initializeSD();

  //PASS 3: Initialize Gyroscope and Servo
  //needs calibration method also.
  initializeMPU();

  //PASS 3: Initialize Baromter
  initializeBMP();

  //PASS 4: Check for RAM
  Serial.println(FreeRam()); //set a threshold for that.

  if (FreeRam() < 275) {
    Serial.println(F("RAM-0"));
    RED();
    while (1);
  }

  //PASS 5: Pyro Check
  if ( pyroPin == true) {
    RED();
    while (1);
  }

  //Get baseline alt
  // float sum = 0;
  // digitalWrite(GLED, LOW);
  // delay(5000);
  // digitalWrite(GLED, HIGH);

  // for (int i = 0; i < 30; i++) {
  //   digitalWrite(GLED, HIGH);
  //   digitalWrite(RLED, LOW);

  //   delay(100);

  //   digitalWrite(GLED, LOW);
  //   digitalWrite(RLED, HIGH);

  //   bmp280.readAltitude(sealvl);
  //   sum += base_alt;
  //   Serial.println(pascal);
  // }

  // base_alt = sum / 30.0;
  // Serial.print(F("BASEH: "));
  // Serial.println(base_alt);

  // delay(1000);
  // digitalWrite(GLED, HIGH);
  // digitalWrite(RLED, HIGH);
}

// ================================================================
// ===                    MAIN LOOP                            ===
// ================================================================
void loop() {
  //Disable Pyros
  digitalWrite(pyroPin, LOW);

  if (launch == false && pyro == false && landed == false) {
    GREEN();
  }
  else if (landed == true) {
    LAND_SIG();
  }


  /*************
     Detect launch
     Enable ABORT - for extreame tilt.
     Detect Apogee
   * * Fire Pyros
     Flight Log
  */

  motion();
  get_alt();

  //Detect if it's launch
  if (ax/16384. > 1.5 || launch == true && landed == false) {
    Serial.println(F("LAUNCH!"));

    // if (launch == false) {
    //   Serial.println(F("LAUNCH!"));
    //   //launchTime = millis();
    //   //delay(1000);
    // }

    launch = true;

    //APOGEE DETECTION PROGRAM


    //Apogee V1

    /**
      if (est_alt - lastAlt <= -0.5 && pyro == false && launch == true && pyroFired == false) {
      //check for drop
      //Store time of Apogee Trigger 1
      delay(20);
      get_alt();
      Serial.println(F("P1"));
      tone(buzzer, 2500, 200);

      if (est_alt - lastAlt <= -1 && pyro == false && launch == true && pyroFired == false) {
        //check for  drop
        //Store time of Apogee Trigger 1

        delay(20);
        get_alt();
        Serial.println(F("P2"));
        tone(buzzer, 2500, 200);

        if (est_alt - lastAlt <= -2 && pyro == false && launch == true && pyroFired == false) {
          //PASS 3
          //Store time of Apogee Pyro Fire
          //Fire Pyros!
          pyro = true;
          Serial.println(F("P3"));
          tone(buzzer, 2500, 200);

        } else {
          lastAlt = est_alt;
        }
      } else {
        lastAlt = est_alt;
      }
      }
      else {
      lastAlt = est_alt;
      }

    **/

    //Apogee V2:
    //Continous Check
    // if (est_alt - lastAlt <= -0.5 && pyro == false && launch == true && pyroFired == false) {

    //   Serial.println("PASS 1");
    //   //Pass 1:
    //   currentMillis = millis();

    //   if (currentMillis - previousMillis2 >= 1000) {
    //     if (est_alt - lastAlt <= -0.5) {
    //       Serial.println("PASS 1");

    //       //Pass 2:
    //       currentMillis = millis();
    //       if (currentMillis - previousMillis2 >= 1000) {

    //         //Pass 3:
    //         if (est_alt - lastAlt <= -0.5) {

    //           //final Pass
    //           Serial.println("Pyro");
    //           pyro = true;

    //         } else {
    //           lastAlt = est_alt;
    //         }
    //         previousMillis2 = currentMillis;
    //       }

    //     } else {
    //       lastAlt = est_alt;
    //     }

    //     previousMillis2 = currentMillis;
    //   }

    // } else {
    //   lastAlt = est_alt;
    // }


  }

  if (pyro == true && launch == true && pyroFired == false) {

    Serial.println(F("pyro"));
    digitalWrite(RLED, HIGH);
    //digitalWrite(pyroPin, HIGH); //FIRE!!
    tone(buzzer, 2500, 1000);

    pyroFired = true;

  }


  //&& pyro == true && pyroFired == true
  //( 0 < est_alt - base_alt < 3 )
  Serial.println("launch ");
  Serial.print(launch);
  Serial.println("landed ");
  Serial.println(landed);



  Serial.println(millis() - landprev);

  //CHECK if it has landed.
  if(ax/16384. > -0.5 && ax/16384. < 0.5 && millis() - landprev > 6000 && landed == false) {
    
    if (ax/16384. > -0.5 && ax/16384. < 0.5 
      && ay/16384. > -0.5 && ay/16384. < 0.5 && landprev != 0) {
      landed = true;
      
      Serial.println("LANDED!");
    }

    landprev = millis();
  } 

  //Flight Logs
  if (launch == true && landed == false) {
    Serial.println("writing");
    Write();
    sd_count++;

    if (sd_count > 100) {
      myFile.flush();
      sd_count = 0;
      Serial.println("FLUSH");
    }
  }  
  if (launch == true && landed == true) {
    myFile.close();
    Serial.println("FClose");
  }
  
}//voidloop end


// ================================================================
// ===               SD CARD Begin                       ===
// ================================================================

void initializeSD() {

  Serial.print(F("InitSD"));

  if (!SD.begin(4)) {
    Serial.println(F("SDFAIL"));
    RED();
    while (1);
  }
  Serial.println(F("SDinit"));

  //Create a file with new name
  if (!loadSDFile()) {
    Serial.println(F("F-0"));
    while (1);
    RED();
  }
  else {
    Serial.println(F("F-1"));
  }

  Serial.println(filename);

  myFile = SD.open(filename, FILE_WRITE);
  Serial.println(myFile);
  if (myFile) {
    //Print Header Files  - - alt, pascal, est_alt, mpuPitch, mpuRoll, mpuYaw, OutputX, OutputY

    myFile.print("Time");
    myFile.print(",");
    myFile.print("Pascal");
    myFile.print(",");
    myFile.print("alt");
    myFile.print(",");
    myFile.print("KMF");
    myFile.print(",");
    myFile.print("ax");
    myFile.print(",");
    myFile.print("ay");
    myFile.print(",");
    myFile.print("az");
    myFile.print(",");
    myFile.print("gx");
    myFile.print(",");
    myFile.print("gy");
    myFile.print(",");
    myFile.print("gz");
    myFile.print(",");
    myFile.print("LaunchTime");
    myFile.print(",");
    myFile.print("ApoTime");
    myFile.print(",");
    myFile.println("LandTime");

    myFile.close();
    Serial.println(F("Fin-1"));

  } else {
    Serial.print(F("Fin-0"));
    RED();
    while (1);
  }
}

// ================================================================
// ===                         BAROMETER                       ===
// ================================================================

void initializeBMP() {

  Serial.print(F("BMP"));

  if (!bmp280.begin()) {
    Serial.println(F("0"));
    while (1);
  }

  /*For drop detection
    osrs_p = 2
    osrs_t = 1
    IIR = 0 (off)
    t_sb = 0 (0.5ms)
  */

  /* Default settings from datasheet. */
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,    /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,      /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

  Serial.println(F("BMPInit1"));
}


// =========================================================
// ===                          MPU init                      ===
// ==========================================================

void initializeMPU() {
  Wire.begin();
  accelgyro.initialize();
  // Serial.println(accelgyro.testConnection() ? "M1" : RED(););

  accelgyro.setAccelerometerPowerOnDelay(3);
  accelgyro.setIntZeroMotionEnabled(TurnOnZI);
  accelgyro.setDHPFMode(1);
  accelgyro.setMotionDetectionThreshold(2);
  accelgyro.setZeroMotionDetectionThreshold(2);
  accelgyro.setZeroMotionDetectionDuration(1);

  //pinMode(LED_PIN, OUTPUT);
}

void motion() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  XnegMD = accelgyro.getXNegMotionDetected();
  XposMD = accelgyro.getXPosMotionDetected();
  YnegMD = accelgyro.getYNegMotionDetected();
  YposMD = accelgyro.getYPosMotionDetected();
  ZnegMD = accelgyro.getZNegMotionDetected();
  ZposMD = accelgyro.getZPosMotionDetected();

  zero_detect = accelgyro.getIntMotionStatus();
  threshold = accelgyro.getZeroMotionDetectionThreshold();

  //Serial.print(temp);Serial.print(",");
  // Serial.print(ax/16384.); Serial.print(",");
  // Serial.print(ay/16384.); Serial.print(",");
  // Serial.println(az/16384.); Serial.print(",");
  // Serial.print(gx/131.072); Serial.print(",");
  // Serial.print(gy/131.072); Serial.print(",");
  // Serial.print(gz/131.072); Serial.print(",");
  // Serial.print(zero_detect); Serial.print(",");
  // Serial.print(XnegMD); Serial.print(",");
  // Serial.println(XposMD);
}

// ================================================================
// ===           SD CARD WRITE AND STUFF                        ===
// ================================================================
//Create a new filename everytime.
boolean loadSDFile() {
  int i = 0;
  boolean file = false;

  while (!file && i < 1024) {
    filename = (String)i + "FL.csv";

    if (!SD.exists(filename)) {
      myFile = SD.open(filename, FILE_WRITE);
      delay(10);
      myFile.close();
      file = true;
    }
    i++;
  }

  return file;
}

void Write() {
  myFile = SD.open(filename, FILE_WRITE);

  //  Serial.println(FreeRam());

  if (myFile) {

    //Writing in SD Card!
    myFile.print(millis());
    myFile.print(",");
    myFile.print(pascal);
    myFile.print(",");
    myFile.print(alt);
    myFile.print(",");
    myFile.print(est_alt);
    myFile.print(",");
    myFile.print(ax/16384.);
    myFile.print(",");
    myFile.print(ay/16384.);
    myFile.print(",");
    myFile.print(az/16384.);
    myFile.print(",");
    myFile.print(gx/131.072);
    myFile.print(",");
    myFile.print(gy/131.072);
    myFile.print(",");
    myFile.print(gz/131.072);
    myFile.print(",");
    myFile.print(launch);
    myFile.print(",");
    myFile.print(pyro);
    myFile.print(",");
    myFile.println(landed);


    myFile.close();

    //delay(10);

  } else {
#ifdef SERIAL_DEBUG
    Serial.println(F("FW-0"));
#endif

    while (1);
  }
}

// ================================================================
// ===                  MISC FUNCTIONS                          ===
// ================================================================


void RED()
{
  digitalWrite(GLED, HIGH);

  digitalWrite(RLED, LOW);
  #ifdef SOUND
  tone(buzzer, 2500, 100);
  #endif
  delay(200);
  digitalWrite(RLED, HIGH);

  #ifdef SOUND 
  tone(buzzer, 2500, 100);
  #endif
  delay(200);
  digitalWrite(RLED, LOW);
 
  #ifdef SOUND 
  tone(buzzer, 2000, 100);
  #endif
  delay(500);
  #ifdef SOUND 
  tone(buzzer, 2000, 100);
  #endif
}

void GREEN() {
  //Everything is fine.. signal.
  unsigned long interval = 1000;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;

    digitalWrite(GLED, LOW);
    #ifdef SOUND
    tone(buzzer, 2500, 100);
    #endif
  }
  else {
    digitalWrite(GLED, HIGH);
  }
}

void LAND_SIG() {
  //Landed

  digitalWrite(RLED, HIGH);

  digitalWrite(GLED, LOW);
  digitalWrite(BLED, HIGH);
  #ifdef SOUND 
  tone(buzzer, 2500, 300);
  #endif
  delay(100);
  digitalWrite(BLED, LOW);
  digitalWrite(GLED, HIGH);
  #ifdef SOUND 
  tone(buzzer, 2000, 300);
  #endif
  delay(100);

  digitalWrite(GLED, LOW);
  digitalWrite(BLED, HIGH);
  #ifdef SOUND 
  tone(buzzer, 2500, 300);
  #endif
  delay(500);
  digitalWrite(BLED, LOW);
  digitalWrite(GLED, HIGH);
  #ifdef SOUND 
  tone(buzzer, 2000, 300);
  #endif
  delay(500);
}

void get_alt() {

  alt = bmp280.readAltitude(1013.5);

  pascal = bmp280.readPressure();

  //est_alt = pressureKalmanFilter.updateEstimate(alt);
}
