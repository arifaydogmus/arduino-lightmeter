//#include <SPI.h>
//#include <Wire.h>
#include <avr/pgmspace.h>
//#include <EEPROM.h>

#include <SoftwareSerial.h>
#include <Nextion.h>
#include <BH1750FVI.h>

#define maxAperture 38
#define maxShutter  64
#define maxISO      41

SoftwareSerial nextion(12, 13); // TX to pin 12 and RX to pin 13
Nextion lcd(nextion, 9600);
BH1750FVI LightSensor;


String aAperture   = "?";
int    aISO        = 100;
double aLux        = 0;
//double aEV       = 0;
int    idxAperture = 0;  // Default f is 1 which is 0 of aperture array
int    idxShutter  = 42; // Default is 1/200
int    idxISO      = 8;  // Default is ISO-100
int    isFlash     = false;

const double arrAperture[] = {
  1, 1.1, 1.2, 1.3, 1.4, 1.6, 1.8, 2, 2.2, 2.5, 2.8, 3.2, 3.5, 4, 4.5, 5, 5.6, 6.3, 7.1, 8, 9, 10, 11, 13, 14, 16, 18, 20, 22, 25, 29, 32, 36, 40, 45, 51, 57, 64 // 38 aperture value
};

const double arrShutter[] = {
  30, 25, 20, 15, 13, 10, 8, 6, 5, 4, 3, 2.5, 2, 1.6, 1.3, 1, // 16 over a second
  1.3, 1.5, 1.6, 2, 2.5, 3, 4, 5, 6, 8, 10, 13, 15, 20, 25, 30, 40, 45, 50, 60, 80, 90, 100, 125, 160, 180, 200, 250, 320, 350, 400, 500, 640, 750, 800, 1000, 1250, 1500, 1600, 2000, 2500, 3000, 3200, 4000, 5000, 6000, 6400, 8000 // 48 Fraction of a second
};

const int arrISO[] = {
  25, 32, 35, 40, 50, 64, 70, 80, 100, 125, 140, 160, 200, 250, 280, 320, 400, 500, 560, 640, 800, 1000, 1100, 1250, 1600, 2000, 2200, 2500, 3200, 4000, 4500, 5000, 6400, 8000, 9000, 10100, 12800, 16100, 18100, 20300, 25600 // 41 ISO value
};

void setup() {
  //if (EEPROM.read(1) >= 0) EEPROM.get(1, idxShutter);
  //if (EEPROM.read(2) >= 0) EEPROM.get(2, idxISO);
  Serial.begin(9600);
  lcd.init();
  LightSensor.Begin(Addr_LOW, Continuous_L);
  //LightSensor.SetMTReg(69);
  //LightSensor.SetSensitivity(1.00);
  //LightSensor.Sleep();
}


void loop() {
  String message = lcd.listen(); // Message format example => 65 0 c 0 ffff ffff ffff   65 00 02 00 FF FF FF
  if (message != "") {
    //Serial.println(message);
    // Trim message for easy reading
    message.replace(" 0 ffff ffff ffff", "");
    message.replace("65 0 ", "");

    if (message == "2") {
      // aperture -
    }

    if (message == "3") {
      // aperture +
    }

    if (message == "4") {
      idxShutter = --idxShutter;
      if (idxShutter <= 0) idxShutter = 0;
      Serial.println("------");
      Serial.print(idxShutter);
      Serial.print(" : ");
      Serial.println(getShutterText());
      lcd.setComponentText("page0.txts", String(getShutterText()));
    }
    if (message == "5") {
      idxShutter = ++idxShutter;
      if (idxShutter > 63) idxShutter = 63;
      Serial.println(idxShutter);
      lcd.setComponentText("page0.txts", String(getShutterText()));
    }

    if (message == "6") {
      idxISO = --idxISO;
      if (idxISO <= 0) idxISO = 0;
      lcd.setComponentText("page0.txti", String(getISO()));
    }
    if (message == "7") {
      idxISO = ++idxISO;
      if (idxISO > 40) idxISO = 40;
      lcd.setComponentText("page0.txti", String(getISO()));
    }

    if (message == "8") {
      getAperture();
    }

  }



}

String getShutterText() {
  String retVal = String(arrShutter[idxShutter]);
  if (idxShutter < 16) {
    retVal = retVal +"\"";
  } else {
    retVal = "1/"+ retVal;
  }
  retVal.replace(".00", "");
  retVal.replace(".10", ".1");
  retVal.replace(".20", ".2");
  retVal.replace(".30", ".3");
  retVal.replace(".40", ".4");
  retVal.replace(".50", ".5");
  retVal.replace(".60", ".6");
  retVal.replace(".70", ".7");
  retVal.replace(".80", ".8");
  retVal.replace(".90", ".9");
  retVal.replace("\"", "\\\"");
  return retVal;
}

double getShutterVal() {
  if (idxShutter < 16) {
    return arrShutter[idxShutter];
  } else {
    return 1 / arrShutter[idxShutter];
  }
}

int getISO() {
  return arrISO[idxISO];
}

void getClosestAperture(double value)
{
  double diff   = 0;
  double retVal = 0;
  for (int i = 0; i < maxAperture; i++) {
    if (i == 0) {
      retVal = arrAperture[i];
      diff = (value - retVal);
    }
    if ((arrAperture[i] - value) < diff) {
      retVal = arrAperture[i];
      diff = (value - retVal);
    }
  }
  aAperture = retVal;
  aAperture.replace(".00", "");
  aAperture.replace(".10", ".1");
  aAperture.replace(".20", ".2");
  aAperture.replace(".30", ".3");
  aAperture.replace(".40", ".4");
  aAperture.replace(".50", ".5");
  aAperture.replace(".60", ".6");
  aAperture.replace(".70", ".7");
  aAperture.replace(".80", ".8");
  aAperture.replace(".90", ".9");
}

void getAperture() {
  aLux = 0;
  double initLux = 0;
  double m1Lux = 0;
  for (int i = 0; i < 1000; i++) {
    m1Lux = LightSensor.GetLux();
    if (i == 0) initLux = m1Lux;
    if (m1Lux > aLux) aLux = m1Lux;
    delay(1);
  }
  isFlash = (aLux - initLux > 100) ? 1 : 0;
  getClosestAperture(sqrt((aLux * getISO() * getShutterVal()) / 250)); // N^2/t = E*S/C => C: calibration constant 250 for incident lights
  refreshDisplay();
}

void refreshDisplay() {
  lcd.setComponentText("page0.txta", String("f/"+ aAperture));
  lcd.setComponentText("page0.txtLux", String(aLux) +" Lux");
  lcd.setComponentValue("page0.btType", isFlash);
}

