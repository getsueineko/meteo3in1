//Импорт библиотек для работы с графикой
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
//Импорт библиотек для работы по цифре с MH-Z19
#include <SoftwareSerial.h>
//Импорт библиотек для RTC DS1307RTC
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
//Импорт библиотек для BMP280
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//Файл иконок
#include "images.h"

//Константы цвета
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

//Константы подключения дисплея
#define __CS 10
#define __DC 9
#define __RS 8

//Константы датчика BME180
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

#define MAX_DEVIATION 0.5

// I2C
Adafruit_BME280 bme; 

struct MyDataOutput
{
float avtemp;
float hum;
float ap;
float co2;
int dhour;
int dmin;
int dsec;
int dday;
int dmonth;
int dyear;
};

TFT_ILI9163C display = TFT_ILI9163C(__CS, __DC, __RS);

float p = 3.1415926;

SoftwareSerial mySerial(A0, A1); // A0 - к TX сенсора, A1 - к RX
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];

byte previous_hour = 0;
byte previous_min = 0;
byte previous_sec = 0;
byte previous_day = 0;
byte previous_month = 0;

float previous_temp = 0;
float previous_hum = 0;
float previous_co2 = 0;
float previous_ap = 0;

const int buttonPin = 2;

int buttonPushCounter = 0;   
int buttonState = 0;        
int lastButtonState = 0;     

void setup() {
  display.begin();
  display.cp437(true); //Поправка на 256-й символ в функции русификации
  
  bme.begin(); //Инициализация BMP280
    
  mySerial.begin(9600);

  pinMode(buttonPin, INPUT);
  
  uint16_t time = millis();
  time = millis() - time;

  Serial.begin(9600);
  while (!Serial) ; // ждем готовности серийного порта
  setSyncProvider(RTC.get);   // функция получает время от RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time"); 
}

void loop() {

MyDataOutput data = GetSensorsData();   
//ShowResultSerial(data);

buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
//Serial.print(buttonState);
//Serial.print( lastButtonState);
//Serial.println( buttonPushCounter);
    if (buttonState == HIGH) {
        display.clearScreen();
//        Serial.print(buttonState);
//        Serial.print( lastButtonState);
//        Serial.println( buttonPushCounter);
        buttonPushCounter++;
       }
delay(50);
}

if (buttonPushCounter > 2) {
    lastButtonState = 0;
    buttonPushCounter = 0;
   }
else {
     lastButtonState = buttonState;
    }


switch (buttonPushCounter) {
    case 1:
      InfoMode(data);       
      break;
    case 2:
      GraphMode(data);
      break;
    default: 
      ClockMode(data); 
      LittleInfo(data);
    break;
  }

}


MyDataOutput GetSensorsData()
{
  
MyDataOutput result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

result.avtemp = bme.readTemperature();
result.ap = (bme.readPressure() / 133.322);
result.hum = bme.readHumidity();

 mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    result.co2 = (float)((256*responseHigh) + responseLow);
  }
  
 tmElements_t tm;

  if (RTC.read(tm)) {
//    Serial.println("Ok, Time is Ready");
//    Serial.println();
  result.dhour = tm.Hour;
  result.dmin = tm.Minute;
  result.dsec = tm.Second;
  result.dday = tm.Day;
  result.dmonth = tm.Month;
  result.dyear = tmYearToCalendar(tm.Year);
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  
  }

return result;
}


void ShowResultSerial(MyDataOutput data)
{
  print2digits(data.dhour);
  Serial.write(':');
  print2digits(data.dmin);
  Serial.write(':');
  print2digits(data.dsec);
  Serial.print(" ");
  print2digits(data.dday);
  Serial.write('/');
  print2digits(data.dmonth);
  Serial.write('/');
  Serial.println(data.dyear);
  
  Serial.print("Temperature = ");
  Serial.print(data.avtemp);
  Serial.println(" *C");
  
  Serial.print("Humidity = ");
  Serial.print(data.hum);
  Serial.println(" %");
  
  Serial.print("Pressure = ");
  Serial.print(data.ap);
  Serial.println(" mm Hg");
  
  Serial.print("CO2 Level = ");
  Serial.print(data.co2);
  Serial.println(" ppm");

}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void InfoMode(MyDataOutput data)
{

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1,0);
  display.println(utf8rus("C"));
  display.setCursor(1,21);
  display.println(utf8rus("Е"));
  display.setCursor(1,43);
  display.println(utf8rus("Й"));
  display.setCursor(1,65);
  display.println(utf8rus("Ч"));
  display.setCursor(1,86);
  display.println(utf8rus("А"));
  display.setCursor(1,110);
  display.println(utf8rus("С"));
    
  display.setCursor(25,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(utf8rus("ТЕМПЕРАТУРА"));
  display.setTextSize(1);
  display.setCursor(75,11);
  display.print(" 0");
  display.setTextSize(2);
  display.println("C");
  display.setCursor(25,23);
  display.setTextSize(1);
  display.println("---------------");
  
  display.setTextSize(1);
  display.setCursor(25,29);
  display.println(utf8rus("ВЛАЖНОСТЬ"));
  display.setTextSize(2);
  display.setCursor(75,40);
  display.println(" %");
  display.setCursor(25,55);
  display.setTextSize(1);
  display.println("---------------");
 
  display.setTextSize(1);
  display.setCursor(25,61);
  display.println(utf8rus("УРОВЕНЬ CO2"));
  display.setTextSize(2);
  display.setCursor(60,72);
  display.println("  ppm");
  display.setCursor(25,85);
  display.setTextSize(1);
  display.println("---------------");

  display.setTextSize(1);
  display.setCursor(25,91);
  display.println(utf8rus("ДАВЛЕНИЕ"));
  display.setTextSize(2);
  display.setCursor(60,102);
  display.println(utf8rus("  мм"));
  display.setCursor(25,116);
  display.setTextSize(1);
  display.println("---------------");

// Вывод на экран с обновлением

  if(previous_temp != data.avtemp)
{
float deviation = data.avtemp -  previous_temp;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_temp = data.avtemp;
   display.fillRect(25, 8, 50, 18, 0);
   display.setTextSize(2);
   display.setCursor(25,11);
   display.print(data.avtemp,1);
  } 
}

  if(previous_hum != data.hum)
{
float deviation = data.hum -  previous_hum;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_hum = data.hum;
   display.fillRect(25, 39, 50, 18, 0);
   display.setTextSize(2);
   display.setCursor(25,40);
   display.print(data.hum,1);
  } 
}

  if(previous_co2 != data.co2)
{
float deviation = data.co2 -  previous_co2;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_co2 = data.co2;
   display.fillRect(25, 70, 50, 18, 0);
   display.setTextSize(2);
   display.setCursor(25,72);
   display.print(data.co2,0);
  } 
}

  if(previous_ap != data.ap)
{
float deviation = data.ap -  previous_ap;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_ap = data.ap;
   display.fillRect(25, 101, 50, 18, 0);
   display.setTextSize(2);
   display.setCursor(25,102);
   display.print(data.ap,0);
  } 
}

//Цветовые индикаторы
 if (data.avtemp > 30) {
   display.fillRect(121, 11, 7, 7, RED);
  } else {
   display.fillRect(121, 11, 7, 7, GREEN);  
  }

   if (data.hum > 70 || data.hum < 30) {
   display.fillRect(121, 40, 7, 7, RED);
  } else {
   display.fillRect(121, 40, 7, 7, GREEN);  
  }

   if (data.co2 > 750) {
   display.fillRect(121, 72, 7, 7, RED);
  } else {
   display.fillRect(121, 72, 7, 7, GREEN);  
  }

   if (data.ap > 750) {
   display.fillRect(121, 102, 7, 7, RED);
  } else {
   display.fillRect(121, 102, 7, 7, GREEN);  
  }


}

void GraphMode(MyDataOutput data) 
{
 
  //Выводим иконки  
  display.drawBitmap(0, 0, temp, 64, 49, WHITE);
  display.setTextSize(1);
  display.setCursor(100,10);
  display.print("0");
  display.setTextSize(3);
  display.setCursor(105,15);
  display.println("C");
  display.setTextSize(5);
  display.setCursor(35,65);
  display.print(data.avtemp,0);
  
  if (data.avtemp > 30) 
{
   display.fillRect(0, 125, 128, 5, RED);
  } 
else 
{
   display.fillRect(0, 125, 128, 5, GREEN);  
  }
  
  delay(5000);
  display.clearScreen();
  
  display.drawBitmap(0, 0, hum, 33, 45, WHITE);
  display.setTextSize(3);
  display.setCursor(105,15);
  display.println("%");
  display.setTextSize(5);
  display.setCursor(35,65);
  display.print(data.hum,0); 
   
  if (data.hum > 70 || data.hum < 30 ) 
{
   display.fillRect(0, 125, 128, 5, RED);
  } 
else 
{
   display.fillRect(0, 125, 128, 5, GREEN);  
  }
  
  delay(5000);
  display.clearScreen();
  
  display.drawBitmap(0, 0, CO2, 64, 50, WHITE);
  display.setTextSize(2);
  display.setCursor(85,15);
  display.println("ppm");
  display.setTextSize(5);

  if(data.co2 < 999)
  {
    display.setCursor(23,65);  
    }
  else 
  {
    display.setCursor(5,65);
    }
  display.print(data.co2,0); 

  if (data.co2 > 750) 
{
   display.fillRect(0, 125, 128, 5, RED);
  } 
else 
{
   display.fillRect(0, 125, 128, 5, GREEN);  
  }
  
  delay(5000);
  display.clearScreen();
  
  display.drawBitmap(0, 0, airpress, 40, 49, WHITE);
  display.setTextSize(3);
  display.setCursor(85,15);
  display.print(utf8rus("мм"));
  display.setTextSize(5);
  display.setCursor(23,65);
  display.print(data.ap,0); 

  if (data.ap > 750) 
{
   display.fillRect(0, 125, 128, 5, RED);
  } 
else 
{
   display.fillRect(0, 125, 128, 5, GREEN);  
  }
  
  delay(5000);
  display.clearScreen();
  
}

void ClockMode(MyDataOutput data)
{
  display.setCursor(22,15);
  display.setTextSize(3);
  display.setTextColor(CYAN);
  printDigits2(data.dhour);
  display.setCursor(58,15);
  if (data.dsec % 2 == 0) 
 {
  display.print(":");
  }
else 
 {
  display.fillRect(58, 15, 14, 26, 0);
  } 
  display.setCursor(73,15);
  printDigits2(data.dmin);

  if(previous_hour !=data.dhour)
{
display.fillRect(18, 15, 40, 26, 0);
previous_hour = data.dhour; //сохраняем текущее время как старое
}

  if(previous_min !=data.dmin)
{
display.fillRect(72, 15, 40, 26, 0);
previous_min = data.dmin; //сохраняем текущее время как старое
}

//  if(previous_sec !=data.dsec)
//{
//display.fillRect(61, 14, 14, 26, 0);
//previous_sec = data.dsec; //сохраняем текущее время как старое
//}

  if(previous_day !=data.dday)
{
display.fillRect(4, 45, 25, 15, 0);
previous_day = data.dday; //сохраняем текущее время как старое
}

  if(previous_month !=data.dmonth)
{
display.fillRect(40, 45, 25, 15, 0);
previous_month = data.dmonth; //сохраняем текущее время как старое
}

  display.setCursor(5,45);
  display.setTextSize(2);
  printDigits2(data.dday);
  display.print(".");
  printDigits2(data.dmonth);
  display.print(".");
  display.print(data.dyear); 
  display.println(); 

}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  display.print(":");
  
  if(digits < 10)
    display.print('0');
    
  display.print(digits);
}

void printDigits2(int digits){
  if(digits < 10)
    display.print('0');
    
  display.print(digits);
}

void LittleInfo(MyDataOutput data) 
{
  display.setTextSize(1);
  display.setTextColor(CYAN);
  display.drawLine(0, 65, 128, 65, CYAN);
  display.setCursor(3,75);
  display.print(utf8rus("Температура:     C"));
  display.setCursor(3,85);
  display.print(utf8rus("Влажность:       %"));
  display.setCursor(3,95);
  display.print(utf8rus("Уровень CO2:     ppm"));
  display.setCursor(3,105);
  display.print(utf8rus("Давление:        мм"));

  if(previous_temp != data.avtemp)
{
float deviation = data.avtemp -  previous_temp;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_temp = data.avtemp;
   display.fillRect(76, 74, 27, 11, 0);
   display.setTextSize(1);
   display.setCursor(76,75);
   display.print(data.avtemp,1);
  } 
}

  if(previous_hum != data.hum)
{
float deviation = data.hum -  previous_hum;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_hum = data.hum;
   display.fillRect(76, 84, 27, 11, 0);
   display.setTextSize(1);
   display.setCursor(76,85);
   display.print(data.hum,1);
  } 
}

  if(previous_co2 != data.co2)
{
float deviation = data.co2 -  previous_co2;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_co2 = data.co2;
   display.fillRect(76, 93, 27, 11, 0);
   display.setTextSize(1);
   display.setCursor(76,95);
   display.print(data.co2,0);
  } 
}

  if(previous_ap != data.ap)
{
float deviation = data.ap -  previous_ap;
  if (deviation<0)
  {
   deviation = deviation * - 1;
  }
  if (deviation >=MAX_DEVIATION)
  {
   previous_ap = data.ap;
   display.fillRect(76, 104, 27, 11, 0);
   display.setTextSize(1);
   display.setCursor(76,105);
   display.print(data.ap,0);
  } 
}

  }


/* Recode russian fonts from UTF-8 to Windows-1251 */
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}


