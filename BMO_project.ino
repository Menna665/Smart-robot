#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL2qKuPxVYs"
#define BLYNK_TEMPLATE_NAME "BMO"
#define BLYNK_AUTH_TOKEN "EAAINvHyENAlwrwgndvTsl_FXp2_8104"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <AskGemini.h>

char ssid[] = "Menna";
char pass[] = "Menna@1234";

double setpoint = 0; 
double angle = 0; 
double kp = 20, ki = 0, kd = 1;
double motorOutput = 0;
int xin1 = 2,xin2= 4,yin1= 5,yin2=18, ENA=15, ENB=19;

PID bmoPID(&angle, &motorOutput, &setpoint, Kp, Ki, Kd, DIRECT);
Adafruit_SH1106G BMO = Adafruit_SH1106G(128, 64, &Wire, -1);
MPU6050 mpu(Wire);
String Gemini_APIKey = " ";
String Gemini_Model  = "gemini-2.5-flash";


void setup()
{
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.begin(115200);
  bmoPID.SetMode(AUTOMATIC);
  bmoPID.SetOutputLimits(-255, 255);
  
  pinMode(xin1,OUTPUT);
  pinMode(xin2,OUTPUT);
  pinMode(yin1,OUTPUT);
  pinMode(yin2,OUTPUT);
  pinMode(ENA,OUTPUT);
  pinMode(ENB,OUTPUT);

  BMO.begin(0x3C, true);
  BMO.clearDisplay();
  BMO.setTextSize(2); 
  BMO.setTextColor(SH110X_WHITE);
  BMO.setCursor(0, 0);
  BMO.println("BMO is Online!");
  BMO.display(); 

  Wire.begin();
  mpu.begin();
  mpu.calcOffsets(true,true); 

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nConnected.");

  delay(200);
}

void loop()
{

  Blynk.run();
  Blynk.virtualWrite(V0,Kd);
  Blynk.virtualWrite(V1,Kp);
  Blynk.virtualWrite(V2,Ki);

  mpu.update();

  float x = mpu.getAngleX();
  float y = mpu.getAngleY();
  float z = mpu.getAngleZ();
  Serial.println(x);
  Serial.println(y);
  Serial.println(z);

  angle = y;

  bmoPID.Compute();
  moveMotors(motorOutput);
  delay(10);

   if (Serial.available()) 
  {
    String input = Serial.readStringUntil('\n');

    if (input == "Go") 
    {
      Serial.println("Asking Gemini...");
      String reply = askGemini("hi how are you", "Words", 0.3);
      Serial.println(reply);
    }
  }
}

void moveMotors(float speed)
{
  if (motorOutput > 0) {
  digitalWrite(xin1,HIGH); digitalWrite(xin2,LOW);
  digitalWrite(yin1,HIGH); digitalWrite(yin2,LOW);
  }
  else
  {
    digitalWrite(xin1,LOW); digitalWrite(xin2,HIGH);
  digitalWrite(yin1,LOW); digitalWrite(yin2,HIGH);
  speed = -1 * speed;
  }
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void errorHandler(int code) {
  Serial.printf("AskGemini error: %d\n", code);
}
