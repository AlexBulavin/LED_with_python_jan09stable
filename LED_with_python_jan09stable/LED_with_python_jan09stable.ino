#include <cvzone.h>
#define LED_PINB 3
#define LED_PINR 5
#define LED_PING 4
#define LED_PIN_EMBEDED 6
#include "led_indication.h"

#include <BfButton.h> //Для работы энкодера

#define ENC_BTN_PIN 7//Кнопка энкодера
#define ENC_DT_PIN 10//DT выход энкодера
#define ENC_CLK_PIN 12//CLK вход энкодера
BfButton btn(BfButton::STANDALONE_DIGITAL, ENC_BTN_PIN, true, LOW);

int encCounter = 0;
//int encAngle = 0;
int encState;
int encLastState;

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_R     0
#define LEDC_CHANNEL_G     1
#define LEDC_CHANNEL_B     2

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

int fading_brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by one step (30ms)


SerialData serialData(1, 1); //(numOfValsRec,digitsPerValRec) в параметре digitsPerValRec указываем размерность числа, которое нам нужно для передачи данных
//нампример, для передачи 0 или 1 достаточна размерность в 1 цифру, для 10 - 99 в 2 цифры, для 100-999 в 3 цифры и так далее
//В нашем случае для управления светодиодом достаточно 1 цифры, поэтому serialData(1, 1);

int valsRec[1]; // array of int with size numOfValsRec

int valSend[2];// array of sending data

int getTimer = 300;
unsigned long getRequestTime = 0;

bool ledToggle = false;
bool ledChangeToggle = true;
int oldValue = 127;

//Button pressed holding function
void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
//      Serial.println("Single push");
      ledToggle = !ledToggle;
//      Serial.print("Toggle = ");
//      Serial.println(ledToggle);
      if (ledToggle) {
      valSend[0] = 1;
      ledChangeToggle = true;
      } else {
      valSend[0] = 0;
      }
      //Отправляем в Python статус LED (ON/OFF)
      //serialData.Send(valSend);
      break;

    case BfButton::DOUBLE_PRESS:
//      Serial.println("Double push");
      valSend[0] = 2; //Отправляем в Python нажатие
     // serialData.Send(valSend);
      break;

    case BfButton::LONG_PRESS:
//      Serial.println("Long push");
      valSend[0] = 3; //Отправляем в Python нажатие
      //serialData.Send(valSend);
      break;
  }

}

void setup() {
  //Serial.begin(9600);
  pinMode(LED_PINR, OUTPUT);
  pinMode(LED_PING, OUTPUT);
  pinMode(LED_PINB, OUTPUT);
  pinMode(LED_PIN_EMBEDED, OUTPUT);
  serialData.begin();
  getRequestTime = millis();
  pinMode(ENC_CLK_PIN, INPUT);
  pinMode(ENC_DT_PIN, INPUT_PULLUP);
  encLastState = digitalRead(ENC_CLK_PIN);
//  Serial.print("encoder data = ");
//  Serial.println(encLastState);

  //button settings
  btn.onPress(pressHandler)
  .onDoublePress(pressHandler)//default timeout
  .onPressFor(pressHandler, 300);//timeout for 0.3 second
}

void loop() {

  btn.read();

  if (!ledToggle && ledChangeToggle) {
    ledChangeToggle = !ledChangeToggle;
    analogWrite(LED_PINB, 0);
    analogWrite(LED_PIN_EMBEDED, 0);
    //valSend[1] = 0; //Отправляем в Python от
    //serialData.Send(valSend);
  }
  if (ledToggle) {
    analogWrite(LED_PINB, oldValue);
    analogWrite(LED_PIN_EMBEDED, oldValue); //
  }
  encState = digitalRead(ENC_CLK_PIN);

  //Encoder rotation tracking
  if (encState != encLastState) {

    if (digitalRead(ENC_DT_PIN) != encState) {
      encCounter ++;
      //      encAngle ++;
    } else {
      encCounter --;
      //      encAngle --;
    }
    if (encCounter >= 100) {
      encCounter = 100;
    }
    if (encCounter <= -100) {
      encCounter = -100;
    }
//    Serial.print("encCounter = ");
//    Serial.println(encCounter);
    oldValue = (encCounter + 100) * 1.28; //Преобразуем шкалу -100... +100 к 0...255
        if (ledToggle) {
          analogWrite(LED_PINB, oldValue);
          analogWrite(LED_PIN_EMBEDED, oldValue); //
        }
    valSend[1] = encCounter; //Отправляем в Python показание энкодера
    //serialData.Send(valSend);
    encLastState = encState;
  }

  if ((millis() - getRequestTime) > getTimer) {
    serialData.Send(valSend);
//    Serial.print("Отправляем на сервер ");
//    Serial.print(valSend[0]);
//    Serial.println(valSend[1]);
    /*
    serialData.Get(valsRec);

    if (valsRec[0] != oldValue) {
      Serial.print("valsRec[0] = ");
      Serial.println(valsRec[0]);
      //      analogWrite(LED_PINB, valsRec[0]);
      //      analogWrite(LED_PIN_EMBEDED, valsRec[0] * 255 / 4); //Включаем встроенный светодиод на 1/4 яркости
      //      ledcAnalogWrite(LEDC_CHANNEL_G, valsRec[0]*255/2);
      //      ledcAnalogWrite(LEDC_CHANNEL_B, valsRec[0]*255/2);
      //      digitalWrite(LED_PINR, valsRec[0]);
      //      digitalWrite(LED_PING, valsRec[0]);
      //      digitalWrite(LED_PINB, valsRec[0]);
      oldValue = valsRec[0];
    

   
  }*/
   getRequestTime = millis();
  }
}
