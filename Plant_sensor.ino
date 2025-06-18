#include <SoftwareSerial.h>
#include <Wire.h> //This one might not need
#include <U8g2lib.h>
#include <DHT.h>

#define relay1 5
#define relay2 6
#define RE 8
#define DE 7
#define Rx 3
#define Tx 4
#define DHTpin 2
#define DHTTYPE DHT22
// Create dht object
DHT dht(DHTpin, DHTTYPE);
#define LDRApin A0
#define MSpin A1

SoftwareSerial nutr(Rx, Tx);
byte data[7]; //The NPK will give 7 bytes response

//RTU request for NPK
const byte nitr[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

//Create u8g2 object
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

void setup() 
{
 Serial.begin(9600);
 nutr.begin(4800);
 dht.begin();
 u8g2.begin();
 u8g2.clear();
 u8g2.firstPage();
 do
 {
  u8g2.setFont(u8g2_font_boutique_bitmap_9x9_bold_tf);
  u8g2.drawStr(5, 30, "Initializing...");
 }
  while (u8g2.nextPage());
 
 pinMode(RE, OUTPUT);
 pinMode(DE, OUTPUT);
 pinMode(Rx, INPUT);
 pinMode(Tx, OUTPUT);
 pinMode(LDRApin, INPUT);
 pinMode(MSpin, INPUT);
 pinMode(relay1, OUTPUT);
 pinMode(relay2, OUTPUT);
 digitalWrite(relay1, LOW);
 digitalWrite(relay2, LOW);
 delay(5000);
}

void loop() 
{
  //read nutrients values
  //read temperature and humidity
  //read light intensity
  //read soil's moisture
  //display all measurement on OLED
  int nitrogen = npk_read(nitr, sizeof(nitr));
  int phosphorus = npk_read(phos, sizeof(phos));
  int potassium = npk_read(pota, sizeof(pota));
  delay(2000);
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  //Check if sensor fail to read 
  //if (isnan(humid) || isnan(temp))
  //{
    //Serial.println("DHT failed to read");
    //return 22;
  //}
  int light = map(analogRead(LDRApin), 0, 1023, 100, 0);
  int moist = map(analogRead(MSpin), 0, 1023, 100, 0);
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_boutique_bitmap_9x9_bold_tf);
    u8g2.setCursor(10, 15);
    u8g2.print("N: "); u8g2.print(nitrogen);
    u8g2.print(" P: "); u8g2.print(phosphorus);
    u8g2.print(" K: "); u8g2.print(potassium);
    u8g2.setCursor(10, 25);
    u8g2.print("T: "); u8g2.print(temp);
    u8g2.print(" H: "); u8g2.print(humid);
    u8g2.setCursor(10, 35);
    u8g2.print("Brightness: "); u8g2.print(light);
    u8g2.setCursor(10, 45);
    u8g2.print("Soil: "); u8g2.print(moist);
  }
  while (u8g2.nextPage());
  
  
  if (moist < 30)
  {
    while (moist < 30)
    {
      digitalWrite(relay1, HIGH);
      delay(1000);
      moist = map(analogRead(MSpin), 0, 1023, 100, 0);
    }
    digitalWrite(relay1, LOW);
  }

  if (nitrogen < 60 || phosphorus < 30 || potassium < 30)
  {
    while (nitrogen < 60 || phosphorus < 30 || potassium < 30)
    {
      digitalWrite(relay2, HIGH);
      delay(1000);
      nitrogen = npk_read(nitr, sizeof(nitr));
      phosphorus = npk_read(phos, sizeof(phos));
      potassium = npk_read(pota, sizeof(pota));
    }
    digitalWrite(relay2, LOW); //change
  }
}

byte npk_read(const byte *request, byte lenght)
{ 
  // Send the request
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  nutr.write(request, lenght);
  // Recieve the data
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  unsigned long time = millis();
  while (nutr.available() < 7 && millis() - time < 2000)
  {

  }
  if (nutr.available() == 7)
  {
    for (byte i = 0; i < 7; i++)
    {
      data[i] = nutr.read();
    }
  }
  else Serial.println("Unable to masure the nutrients");
  Serial.println(data[4]);
  return data[4];
}

