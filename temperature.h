//-- ThingSpeak ethernet shield arduino example by Ilias Lamprou
//-- Use Virtuino app to read or write to thingSpeak server
//-- Video tutorial: https://www.youtube.com/watch?v=ng767bg2BNk
//-- 08/22/2016
#include <DHT.h>        // You have to download DHT11  library
#include <OneWire.h>                    // Библиотека протокола 1-Wire
#include <DallasTemperature.h>          // Библиотека для работы с датчиками DS*
#include <LiquidCrystal_1602_RUS.h>
#define ONE_WIRE_BUS 8                 // Шина данных на 8 пине для датчиков DS18B20
#define TEMPERATURE_PRECISION 9         // Точность измерений в битах (по умолчанию 12)

OneWire oneWire(ONE_WIRE_BUS);          // Создаем экземпляр объекта протокола 1-WIRE - OneWire
DallasTemperature sensorsDS(&oneWire);    // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*

DeviceAddress myDsBak1 = { 0x28, 0xFF, 0x3D, 0x1C, 0xB3, 0x16, 0x04, 0x75 };  // адрес датчика DS18B20
DeviceAddress myDsBak2 = { 0x28, 0xFF, 0xCF, 0x04, 0xB3, 0x16, 0x04, 0x74 };  // адрес датчика DS18B20
DeviceAddress myDsBak3 = { 0x28, 0xFF, 0xCF, 0x09, 0xB3, 0x16, 0x03, 0x6A };  // адрес датчика DS18B20
DeviceAddress myDsBak4 = { 0x28, 0xFF, 0x51, 0xBD, 0xA1, 0x16, 0x05, 0x38 };  // адрес датчика DS18B20
DeviceAddress myDsKotel = { 0x28, 0xFF, 0x72, 0x7C, 0x01, 0x17, 0x05, 0x0A };  // адрес датчика DS18B20
DeviceAddress myDsKollektor1 = { 0x28, 0xFF, 0x74, 0xF0, 0xB2, 0x16, 0x03, 0x1C };  // адрес датчика DS18B20
DeviceAddress myDsKollektor2 = { 0x28, 0xFF, 0x57, 0x1E, 0xB3, 0x16, 0x04, 0xDE };  // адрес датчика DS18B20

LiquidCrystal_1602_RUS disp(7, 6, 5, 4, 3, 2);  // объект дисплей

//Attention: For new  DHT11 version  libraries you will need the Adafruit_Sensor library
//Download from here:https://github.com/adafruit/Adafruit_Sensor
//and install to Arduino software

#include <SPI.h> 
#include <Ethernet.h>
 
  #ifdef SPARK
    #include "ThingSpeak/ThingSpeak.h"    // You have to download ThingSpeak  library
  #else
    #include "ThingSpeak.h"
  #endif

#define WRITE_DELAY_FOR_THINGSPEAK 15000
#define DHTPIN 9   // PIN №9 подключения датчика DTH11
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
float tDHT;  
float hDHT;

  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xEF,0xED};
  IPAddress ip(192,168,1,150);                             //Check your router IP
  EthernetClient client;

  unsigned long myKomnataChannelNumber = 355373;            //Replase this code with your thingSpeak channel ID
  unsigned long myKotelChannelNumber = 356371;           //Replase this code with your thingSpeak channel ID
  unsigned long myTempBakChannelNumber = 356572;            //Replase this code with your thingSpeak channel ID
  unsigned long myKollektorChannelNumber = 356770;           //Replase this code with your thingSpeak channel ID
 
  const char * myWriteAPIKey_Komnata = "SB7UNSXSZTVHREYB";  //Replace with your channel Write Key
  const char * myWriteAPIKey_Kotel = "BTNC77I29RBREM1T"; //Replace with your channel Write Key. This key is not used in this example
  const char * myWriteAPIKey_TempBak = "I1TBI0E91YC1NCTU";  //Replace with your channel Write Key
  const char * myWriteAPIKey_Kollektor = "4639DQYPG84EFGCW"; //Replace with your channel Write Key. This key is not used in this example
                                                    //Use this key from android device  

long lastWriteTime = 0;    
long lastReadTime = 0;
long lastDataTime = 0;
float responseValue = 0;

int photoPin = 0;  // фоторезистор подключен 0-му аналоговому входу
float myFotoSens = 0;
float myAtmSensP = 0.09; //переменная значения давления в системе
int x = 0;

void setup() {
  Serial.begin(9600);
  disp.begin(16, 2);  // инициализируем дисплей 2 x 16 символов
 
  sensorsDS.begin(); // Запускаем поиск всех датчиков DS18B20 на шине
    disp.clear();  // очистка экрана
    disp.setCursor(0, 0);

  Ethernet.begin(mac, ip);
  ThingSpeak.begin(client);
  dht.begin();

}


void loop() {
 
   if (millis()-lastReadTime>15000) {       // read value every 15 secs. Avoid delay
    
  myFotoSens = analogRead(photoPin);

  sensorsDS.requestTemperatures();  // Запускаем измерение температуры на всех датчиках DS18B20

  //// Когда температура измерена её можно вывести
 
  //-------- Example 2 read value from second channel "Commands/field1"
  //-------- if value is 1 enable pin 13 board. Write the value to the other channel (Sensors/field 3) as responce
 
 //  if (millis()-lastReadTime>15000) {       // read value every 15 secs. Avoid delay
//      float value = ThingSpeak.readFloatField(myCommandsChannelNumber, 1);
//      Serial.println("Read thingSpeak channel commands/field 1 last value="+ String(value));
//      if (value==0) digitalWrite(7,LOW);
//      else digitalWrite(7,HIGH);
//      responseValue=value;
//      lastReadTime=millis();                // store last read time
   }
 
  //-------- Example 1 write DHT11 sensors' values to thingSpeak channel sensors
  
  if (millis()-lastWriteTime>20000) {       // every 20 secs.  Must be >15000.    

       Serial.println("Read sensors' values...");
       hDHT = dht.readHumidity();
       tDHT = dht.readTemperature();
          Serial.println("Temp="+String(tDHT)+" *C");
          Serial.println("Humidity="+String(hDHT)+" %");
          ThingSpeak.setField(1,tDHT);
          ThingSpeak.setField(2,hDHT);
//          ThingSpeak.setField(3,responseValue);             // This is the value of commands channel
          ThingSpeak.writeFields(myKomnataChannelNumber, myWriteAPIKey_Komnata);                  // write two values
          
           ThingSpeak.setField(1,sensorsDS.getTempC(myDsBak1));
          ThingSpeak.setField(2,sensorsDS.getTempC(myDsBak2));
          ThingSpeak.setField(3,sensorsDS.getTempC(myDsBak3));
		      ThingSpeak.setField(4,sensorsDS.getTempC(myDsBak4));
          ThingSpeak.writeFields(myTempBakChannelNumber, myWriteAPIKey_TempBak);                  // write values

          ThingSpeak.setField(1,sensorsDS.getTempC(myDsKotel));
          ThingSpeak.setField(2,myFotoSens);
          ThingSpeak.writeFields(myKotelChannelNumber, myWriteAPIKey_Kotel);

          ThingSpeak.setField(1,sensorsDS.getTempC(myDsKollektor1));
          ThingSpeak.setField(2,sensorsDS.getTempC(myDsKollektor2));
          ThingSpeak.setField(8,myAtmSensP);
//          ThingSpeak.setField(2,nomer);
          ThingSpeak.writeFields(myKollektorChannelNumber, myWriteAPIKey_Kollektor);                  // write two values

          lastWriteTime=millis();               // store last write time

  
  }
  //----------------------------------------------------------------------------------------------------------
   if (millis()-lastDataTime>5000) {       // Каждые 5 секунд меняются данные на дисплее. 


  switch (x)  {

  case 1 : // Вывод на экран значений температуры и влажности в комнате

    disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
    disp.print(" К о м н а т а");
  disp.setCursor(2, 1);
  disp.print(int(tDHT));
  disp.print("°С   ");
  disp.print(int(hDHT));
  disp.print(" %");
      break;

  case 2 :  // Вывод на экран значений температуры воды на выходе из котла

  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
    disp.print("    К о т е л");
  disp.setCursor(0, 1);
  disp.print("Температура ");
  disp.print(int(sensorsDS.getTempC(myDsKotel)));
  disp.print("°С   ");
     break;

     case 3 :  // Вывод на экран значений температуры воды в баке
     
  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
  disp.print(" Бак:");
  disp.print(" №1=");
  disp.print(int(sensorsDS.getTempC(myDsBak1)));
  disp.print("°С ");
  disp.setCursor(0, 1);    
  disp.print("№2=");
  disp.print(int(sensorsDS.getTempC(myDsBak2)));
  disp.print("°C ");
   disp.print("№3= ");
  disp.print(int(sensorsDS.getTempC(myDsBak3)));
  disp.print("°С ");
 
     break;
   
   case 4 :  // Вывод на экран значений температуры воды в баке
     
  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
  disp.print(" Бак:  датчик");
      disp.setCursor(0, 0);
  disp.print(" Бак:  датчик");
  disp.setCursor(0, 1); 
  disp.print("в центре  ");
  disp.print(int(sensorsDS.getTempC(myDsBak4)));
  disp.print("°С ");
  
       break;
     
     case 5 :  // Вывод на экран значений температуры воды в коллекторе
     
  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
  disp.print("   Коллектор:");
  disp.setCursor(0, 1);    
    disp.setCursor(0, 1);    
  disp.print("№1=");
  disp.print(int(sensorsDS.getTempC(myDsKollektor1)));
  disp.print("°С ");
   disp.print("№2=");
  disp.print(int(sensorsDS.getTempC(myDsKollektor2)));
  disp.print("°С ");
 
     break;
 
  default :
     // код выполняется если  не совпало ни одно предыдущее значение
     break;
}

 x++;

      if (x > 5) {      
        x=1;
      }

      lastDataTime = millis();
   }
} 

