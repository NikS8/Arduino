#include <iarduino_RTC.h>
#include <DHT.h>        // You have to download DHT11  library
#include <OneWire.h>                    // Библиотека протокола 1-Wire
#include <DallasTemperature.h>          // Библиотека для работы с датчиками DS*
#include <LiquidCrystal_1602_RUS.h>
#include <SPI.h> 
#include <Ethernet.h>
#include <SD.h>
//------------
#define SWITCH_TO_W5100 digitalWrite(4,HIGH); digitalWrite(10,LOW)
#define SWITCH_TO_SD digitalWrite(10,HIGH); digitalWrite(4,LOW)
#define ALL_OFF digitalWrite(10,HIGH); digitalWrite(4,HIGH)
//---------------
#define ONE_WIRE_BUS 9                 // Шина данных на 9 пине для датчиков DS18B20 (ардуино UNO)
#define TEMPERATURE_PRECISION 9         // Точность измерений в битах (по умолчанию 12)
#define DHTPIN 8                        // PIN №8 подключения датчика DTH11
#define WRITE_DELAY_FOR_THINGSPEAK 15000
//-----------
#define DHTTYPE DHT11 
//------------
iarduino_RTC time(RTC_DS1302, 5, 6, 7); // подключаем RTC модуль на базе чипа DS1302, выводы Arduino к выводам модуля RST, CLK, DAT
char dateTime;
int lastTime = -1;
int lastTime2 = -1;
long lastWriteTime = 0;    
long lastReadTime = 0;
long lastDataTime = 0;
float responseValue = 0;
//-------------
DHT dht(DHTPIN, DHTTYPE);
float sensorDhtTempKitchen;  
float sensorDhtHumKitchen;
//------------
OneWire oneWire(ONE_WIRE_BUS);            // Создаем экземпляр объекта протокола 1-WIRE - OneWire
DallasTemperature sensorsDS(&oneWire);    // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*

DeviceAddress sensorDsTankLow = { 0x28, 0xFF, 0x3D, 0x1C, 0xB3, 0x16, 0x04, 0x75 };       // адрес датчика DS18B20 на стенке бака внизу
DeviceAddress sensorDsTankMiddle = { 0x28, 0xFF, 0xCF, 0x04, 0xB3, 0x16, 0x04, 0x74 };    // адрес датчика DS18B20 на стенке бака посередине
DeviceAddress sensorDsTankHigh = { 0x28, 0xFF, 0xCF, 0x09, 0xB3, 0x16, 0x03, 0x6A };      // адрес датчика DS18B20 на стенке бака вверху
DeviceAddress sensorDsTankInside = { 0x28, 0xFF, 0x51, 0xBD, 0xA1, 0x16, 0x05, 0x38 };    // адрес датчика DS18B20 внутри бака
DeviceAddress sensorDsBoiler = { 0x28, 0xFF, 0x72, 0x7C, 0x01, 0x17, 0x05, 0x0A };        // адрес датчика DS18B20 на выходном патрубке котла
DeviceAddress sensorDsCollectorIn = { 0x28, 0xFF, 0x74, 0xF0, 0xB2, 0x16, 0x03, 0x1C };   // адрес датчика DS18B20 на входе коллектора
DeviceAddress sensorDsCollectorOut = { 0x28, 0xFF, 0x57, 0x1E, 0xB3, 0x16, 0x04, 0xDE };  // адрес датчика DS18B20 на выходе коллектора
//----------------
//LiquidCrystal_1602_RUS disp(7, 6, 5, 4, 3, 2);      // объект дисплей
LiquidCrystal_1602_RUS disp(32, 30, 28, 26, 24, 22);  // объект дисплей (MEGA 2560)
char dispStr1[17];
char dispStr2[17];
//--------------
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xEF,0xED};
  IPAddress ip(192,168,1,156);                             //Check your router IP
EthernetClient client;
EthernetServer server(3003);
boolean check;
double tempin;
double tempout;
char timeServer[] = "nist1-ny.ustiming.org";
//-----------------
int photoPin = A5;          // фоторезистор подключен 0-му аналоговому входу
float sensorPhotoBoiler = 0;
//float myAtmSensP = 0.09;  //переменная значения давления в системе
//-----------------
File myFile;                                          // Создаем класс для работы с SD
const int chipSelect = 4;
char filename[] = "00000000.csv";
const byte NUM_FIELDS = 1;                            //You can add extra fields but then you must list them on the next line. Read the tutorial at sparkfun! It will help.
const String fieldNames[NUM_FIELDS] = {"temp"};       //This has to be the same as the field you set up when you made the data.sparkfun.com account i.e. replace "temp" with what you used.
String fieldData[NUM_FIELDS];
//-----------------
int switchX = 0;

//-----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(9600);
  time.begin();
  //--------------
  disp.begin(16, 2);  // инициализируем дисплей 2 x 16 символов
     disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
    disp.print(L" Климат контроль");
	disp.setCursor(0, 1);
	disp.print(time.gettime("d.m.Y H:i")); // выводим время
	//------------------
  SWITCH_TO_SD;
  pinMode(4, OUTPUT);
  pinMode(10, OUTPUT);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
 //     return;
  }
  
  Serial.println("initialization done.");
Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
  pinMode(4, OUTPUT);
  pinMode(10, OUTPUT);
  SWITCH_TO_W5100;
  Serial.print("Initializing Ethernet...");
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Serial.println("Initializing Ethernet direct...");
    Ethernet.begin(mac, ip);
  }else{
    Serial.println("initialization done.");
  }

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("My gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  ALL_OFF;
  
	
//----------    
  sensorsDS.begin(); // Запускаем поиск всех датчиков DS18B20 на шине
  dht.begin();
//------------
SWITCH_TO_W5100;
  Ethernet.begin(mac, ip);
  delay(3000);// Дадим время шилду на инициализацию
  Serial.println("connecting...");
  if (client.connect(timeServer, 80)) {
    Serial.println("---------------");
    // Создаем HTTP-запрос
    client.println("GET /utc/now HTTP/1.1");
    client.println("Host: www.timeapi.org");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
       client.stop();
//------------
}
//------------------------------------------------------------------------------------------------------

void loop() {

   myWeb();

  

    if (millis()-lastDataTime>5000) {       // Каждые 5 секунд меняются данные на дисплее. 

   MyDispPrint(); 
   
	switchX++;
      if (switchX > 5) {      
        switchX=1;
      }
    lastDataTime = millis();
	
		sensorDhtHumKitchen = dht.readHumidity();
		sensorDhtTempKitchen = dht.readTemperature();
		sensorPhotoBoiler = analogRead(photoPin);
		sensorsDS.requestTemperatures();  // Запускаем измерение температуры на всех датчиках DS18B20
}
   
//------------------------
   if (millis()-lastReadTime>15000) {       // read value every 15 secs. Avoid delay
   
  lastReadTime = millis();
    }
 
  //-------- 
  
  if (millis()-lastWriteTime>59000) {       // every 60 secs.  Must be >15000.    
//// Когда температура измерена её можно вывести
       Serial.println("Read sensors' values..."); //Измерение температуры и влажности на кухне
          Serial.println("Temp="+String(sensorDhtTempKitchen)+" *C");
          Serial.println("Humidity="+String(sensorDhtHumKitchen)+" %");

              ALL_OFF;
		  getFilename(filename);
		  fileWriteSD();
  //--------------------------------------
 //         Serial.print("Temp=");
//         Serial.println(int(sensorsDS.getTempC(sensorDsCollectorIn)));

          lastWriteTime=millis();               // store last write time

//     client.stop();
  }
  //---------------------------------------
  
} 

/////////////////////////////////////////////////////////////////////////////////////////////////
//			Дополнительные
//				функции
//////////////////////////////////////////////////////////////////////////////////////////////////////

//	Вывод информации на дисплей
int MyDispPrint(){
  switch (switchX)  {

  case 1 : // Вывод на экран значений температуры и влажности в комнате

    disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
    disp.print(L" К о м н а т а");
  disp.setCursor(2, 1);
  disp.print(int(sensorDhtTempKitchen));
  disp.print("°C   ");
  disp.print(int(sensorDhtHumKitchen));
  disp.print(" %");
      break;

  case 2 :  // Вывод на экран значений температуры воды на выходе из котла

  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
    disp.print(L"    К о т е л");
  disp.setCursor(0, 1);
  disp.print(L"температура ");
  disp.print(int(sensorsDS.getTempC(sensorDsBoiler)));
  disp.print("°C   ");
     break;

     case 3 :  // Вывод на экран значений температуры воды в баке
     
  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
  disp.print(L" Бак:");
  disp.print(" N1=");
  disp.print(int(sensorsDS.getTempC(sensorDsTankHigh)));
  disp.print("°C ");
   disp.setCursor(0, 1);    
  disp.print("N2=");
  disp.print(int(sensorsDS.getTempC(sensorDsTankMiddle)));
  disp.print("°C  ");
  disp.print("N3=");  
  disp.print(int(sensorsDS.getTempC(sensorDsTankLow)));
  disp.print("°C ");
 
     break;
   
   case 4 :  // Вывод на экран значений температуры воды в баке
     
   disp.clear();  // очистка экрана
   disp.setCursor(0, 0);
  disp.print(L" Бак:");
  disp.print(L"  датчик ");
  disp.setCursor(0, 1); 
  disp.print(L"внутри  ");
     disp.setCursor(0, 0);
  disp.print(L" Бак:");
  disp.print(L"   датчик ");
  disp.setCursor(0, 1); 
  disp.print(L"  внутри  ");
  disp.print(int(sensorsDS.getTempC(sensorDsTankInside)));
  disp.print("°C");
  
       break;
     
     case 5 :  // Вывод на экран значений температуры воды в коллекторе
     
  disp.clear();  // очистка экрана
    disp.setCursor(0, 0);
  disp.print(L"   Коллектор:");
    disp.setCursor(0, 1);    
  disp.print("N1=");
  disp.print(int(sensorsDS.getTempC(sensorDsCollectorIn)));
  disp.print("°C  ");
   disp.print("N2=");
  disp.print(int(sensorsDS.getTempC(sensorDsCollectorOut)));
  disp.print("°C ");
 
     break;
 
  default :
     // код выполняется если  не совпало ни одно предыдущее значение
     break;
}
}

///////////////////////////////////////////////////////////////////////////////

void getFilename(char *filename) {

  int year = time.year;
  int month = time.month;
  int day = time.day;

  filename[0] = '2';
  filename[1] = '0';
  filename[2] = year / 10 + '0';
  filename[3] = year % 10 + '0';
  filename[4] = month / 10 + '0';
  filename[5] = month % 10 + '0';
  filename[6] = day / 10 + '0';
  filename[7] = day % 10 + '0';
  filename[8] = '.';
  filename[9] = 'c';
  filename[10] = 's';
  filename[11] = 'v';
 
  return;
}

////////////////////////////////////////////////////////////////////////////////

//----------------ПИШЕМ НА SD КАРТУ ---------------------------------------------------------------//
void fileWriteSD(){
   Serial.println("SD work start");
   SWITCH_TO_SD;

  if (SD.exists("filename")) {
    Serial.println("file exists.");
  }
  else {
    Serial.println("file doesn't exist.");
  }
  
      myFile = SD.open(filename, FILE_WRITE);
   Serial.println("file opened");			// if the file opened okay, write to it:
   
   if (myFile) {
    Serial.println("Writing to data file...");
            myFile.print(time.gettime("Y.m.d H:i"));        // записываем дату и время
            myFile.print(F(" ; sensorDhtTempKitchen ; "));                // записываем температуру
            myFile.print(sensorDhtTempKitchen);
            myFile.print(F(" ; sensorDhtHumKitchen ; "));               // записываем влажность
            myFile.print(sensorDhtHumKitchen);
            myFile.print(F(" ; sensorPhotoBoiler ; "));       // записываем значение величины фотосопротивления
            myFile.print(sensorPhotoBoiler);
            myFile.print(F(" ; sensorDsBoiler ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsBoiler)));
            myFile.print(F(" ; sensorDsTankHigh ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsTankHigh)));
            myFile.print(F(" ; sensorDsTankMiddle ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsTankMiddle)));
      			myFile.print(F(" ; sensorDsTankLow ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsTankLow)));
            myFile.print(F(" ; sensorDsCollectorOut ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsTankInside)));
      			myFile.print(F(" ; sensorDsCollectorIn ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsCollectorIn)));
            myFile.print(F(" ; sensorDsCollectorOut ; "));                // записываем температуру
            myFile.print(int(sensorsDS.getTempC(sensorDsCollectorOut)));
	      		myFile.println(" ; end... ");

  // close the file:
    myFile.close();
    Serial.println("done. SD work end.");
   } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data file.");
    myFile.close();
    Serial.println("SD work end.");
   }
   ALL_OFF;
}
   //////////////////////////////////////////////////////////////////////////////
void myWeb(){
   SWITCH_TO_W5100;
    EthernetClient client = server.available();
 if (client)
 {
  // Проверяем подключен ли клиент к серверу
  while (client.connected())
  {
   if (client.available()) {
   // Выводим HTML страницу
   client.println("HTTP/1.1 200 OK");
   client.println("Content-Type: text/html");
   client.println();
   client.println();
   client.println("<!DOCTYPE html>");
   client.println("<html lang=\"ru\">");
   client.println("<head>");
   client.println("<meta charset=\"UTF-8\">");
   client.println("<title>Home</title>");
   client.println("</head>");
   client.println("<body>");
   client.println("<h1>Home Server</h1>");
   client.println(time.gettime("d-m-Y, H:i:s, D"));
   client.println("<h1>Комната</h1>");
   client.println("<p>Температура: ");
   client.println(sensorDhtTempKitchen);
   client.println("</p>");
   client.println("<p>Влажность: ");
   client.println(sensorDhtHumKitchen);
   client.println("</p>");

  //----------------------- 
   
    char* params;
//  if (params = ethernet.serviceRequest()) {
    
    
    //when you type in browser http://192.168.0.101/all
 //   if (strcmp(params, "all") == 0){
      
   
    
    client.print("<center>");
    client.print("<font color='teal'>");
    client.print("<h1>Климат контроль</h1>"); 
    client.print("</font>");
    client.print("<br>");
    
    client.print("<h2>sensorDsBoiler Temp: "); 
    client.print(sensorsDS.getTempC(sensorDsBoiler)); 
    client.print("</h2>");
    
    client.print("<h2>sensorPhotoBoiler: "); 
    client.print(sensorPhotoBoiler); 
    client.print("</h2>");
    
    client.print("<h2>sensorDsTankHigh Temp: "); 
    client.print(sensorsDS.getTempC(sensorDsTankHigh)); 
    client.print("</h2>");
    
    client.print("<h2>sensorDsTankMiddle Temp: "); 
    client.print(sensorsDS.getTempC(sensorDsTankMiddle)); 
    client.print("</h2>");
  
    client.print("<h2>sensorDsTankLow Temp: "); 
    client.print(sensorsDS.getTempC(sensorDsTankLow)); 
    client.print("</h2>");
    
    client.print("<h2>sensorDsTankInside Temp: "); 
    client.print(sensorsDS.getTempC(sensorDsTankInside)); 
    client.print("</h2>");

    client.print("<h2>sensorDsCollectorIn Temp in: "); 
    client.print(sensorsDS.getTempC(sensorDsCollectorIn)); 
    client.print("</h2>");
    
    client.print("<h2>sensorDsCollectorOut Temp out: "); 
    client.print(sensorsDS.getTempC(sensorDsCollectorOut)); 
    client.print("</h2>");
    
    check = true;  
    client.print("<h2>Alarm: "); 
 //   client.print(alarm); 
    client.print("</h2>");
 
    
    client.print("<br>");
    client.print("<br>");
   
   client.print("</center>");
    
   
  }
   client.println("</body>");
   client.println("</html>");
   client.stop();  
   //---------------------------
   } 
   
  }

}
 
