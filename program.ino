#define BLYNK_TEMPLATE_ID "TMPL6uDWRFmOv"
#define BLYNK_TEMPLATE_NAME "Ruang 1"
#define BLYNK_AUTH_TOKEN "6McbWRiRg54C9TGn94VEcaohrSsJOwlD"
#define BLYNK_PRINT Serial

#include <DHT.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <MQUnifiedsensor.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define placa "ESP32"
#define Voltage_Resolution 3
#define pin 33
#define type "MQ-2"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

#define led_R 25
#define led_G 26
#define led_B 32
#define buzzer 27

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float t, h;
char SSID[] = "RobotikID";
char PASS[] = "robotikid";

String statTemp;
String statHumi;
String statAQI;

byte termometer[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01110,
  0b11111,
  0b11111,
  0b01110
};

byte humy[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b10001,
  0b01110,
  0b00100
};

byte airQuality[8] = { 
  0b01110, 
  0b11111, 
  0b10101, 
  0b11111, 
  0b01110, 
  0b00100, 
  0b01110, 
  0b01110
};

byte notif[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100
};


int calculateAQI(float CO2) {
  if (CO2 <= 400) return 0;
  if (CO2 <= 1000) return map(CO2, 400, 1000, 0, 50);
  if (CO2 <= 2000) return map(CO2, 1000, 2000, 51, 100);
  if (CO2 <= 5000) return map(CO2, 2000, 5000, 101, 150);
  if (CO2 <= 10000) return map(CO2, 5000, 10000, 151, 200);
}

void sendSensor() {
  MQ135.update();
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  float CO2 = MQ135.readSensor() + 400;
  int AQI = calculateAQI(CO2);

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca DHT!");
    return;
  }
  if (AQI >= 0 && AQI <= 50) {
    statAQI = "Good    ";
    noTone(buzzer);
  } 
  else if (AQI >= 51 && AQI <= 100) {
    statAQI = "Moderate";
    noTone(buzzer);
  } 
  else if (AQI >= 101) {
    statAQI = "Bad      ";
    tone(buzzer, 1000);
  }

  if (t >= 32 && t <= 34) {
    statTemp = "Good    ";
  } 
  else if ((t >= 30 && t <= 31) || (t >= 35 && t <= 36)) {
    statTemp = "Moderate";
  } 
  else {
    statTemp = "Bad     ";
  }

  if (h >= 40 && h <= 60) {
    statTemp = "Good    ";
  } 
  else if ((h >= 35 && h <= 39) || (h >= 61 && h <= 65)) {
    statHumi = "Moderate";
  } 
  else {
    statHumi = "Bad     ";
  }

  if (statHumi == "Bad     " && statTemp == "Bad     ") {
    analogWrite(led_R, 255); //red
    analogWrite(led_G, 0); 
    analogWrite(led_B, 0);
  }
  else if (statHumi == "Moderate" || statTemp == "Moderate") {
    analogWrite(led_R, 0); //blue
    analogWrite(led_G, 0); 
    analogWrite(led_B, 255);
  }
  else if (statHumi == "Good    " && statTemp == "Good    ") {
    analogWrite(led_R, 0);  //green
    analogWrite(led_G, 255); 
    analogWrite(led_B, 0);
  }

  Serial.println(String(h) + "%; " + String(t) + "°C; " + String(CO2) + ";" + String(AQI));
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V5, statTemp);
  Blynk.virtualWrite(V4, statHumi);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V0, AQI);
  Blynk.virtualWrite(V3, statAQI);

  lcd.setCursor(0, 0); 
  lcd.write(byte(0)); 
  lcd.print(":" + String(t) + (char)223 + "C"); 
  lcd.setCursor(0, 1);
  lcd.write(byte(3)); 
  lcd.print(":" + statTemp); 

  lcd.setCursor(11,0);
  lcd.write(byte(1));  
  lcd.print(":" + String(h) + "%");
  lcd.setCursor(11, 1);
  lcd.write(byte(3)); 
  lcd.print(":" + statHumi); 

  lcd.setCursor(4, 2); 
  lcd.write(byte(2)); 
  lcd.print(":" + String(AQI) + " AQI    "); 
  lcd.setCursor(4, 3);
  lcd.write(byte(3)); 
  lcd.print(":" + statAQI); 
}



void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, termometer);  
  lcd.createChar(1, humy);  
  lcd.createChar(2, airQuality);      
  lcd.createChar(3, notif);
  pinMode(buzzer, OUTPUT);
  pinMode(led_R, OUTPUT);
  pinMode(led_G, OUTPUT);
  pinMode(led_B, OUTPUT);
  WiFi.begin (SSID,PASS);
  lcd.setCursor(2, 1); 
  lcd.print("Connect To Wi-Fi");
  while (WiFi.status () != WL_CONNECTED){
    Serial.print(".");
    lcd.setCursor(3, 2); 
    lcd.print("wait a minute");
    delay (500);
  }  
  lcd.clear ();
  Serial.println("");
  Serial.println ("WiFi Connected");            
  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PASS, "sgp1.blynk.cloud",8080);

  MQ135.setRegressionMethod(1);
  MQ135.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1);
  }

    
}

void loop() {
  Blynk.run();
  sendSensor();
  delay(2000);
}
