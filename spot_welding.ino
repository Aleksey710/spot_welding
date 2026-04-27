/*
https://microkontroller.ru/stm32-projects/nachalo-raboty-s-stm32-blue-pill-s-pomoshhyu-arduino-ide/
https://microkontroller.ru/stm32-projects/podklyuchenie-zhk-displeya-16x2-k-stm32-blue-pill-stm32f103c8t6/

Компоненты:
BluePill(STM32F103C8T6)
1602A
Энкодер


* - разрыв
= - соединение

Operating mode
USB  0=0*0  SWD
USB  0=0*0  SWD

Programming mode
USB  0*0=0  SWD
USB  0=0*0  SWD

-----------------------------
FTDI232
-----------------------------
| A9   - TX1  - RX          |
| A10  - RX1  - TX          |
-----------------------------
LCD 
-----------------------------
| 1  VSS pin to ground      |
| 2  VDD pin to 5V          |
| 3  V0                     |
-----------------------------
| 4  RS  - PB11             |
| 5  RW  - PB10     X       |
| 6  E   - PB1              |
-----------------------------
| 7  D0  - X
| 8  D1  - X
| 9  D2  - X
| 10 D3  - X
-----------------------------
| 11 D4  - PB0              |
| 12 D5  - PA7              |
| 13 D6  - PA6              |
| 14 D7  - PA5              |
-----------------------------
| 15 A                      |
| 16 K                      |
-----------------------------
| BUZZER - PA4              |
-----------------------------
| pinA   - PB12             |
| pinB   - PB13             |
| pinSW  - PB14             |
-----------------------------

*/
//------------------------------------------------
#include <Arduino.h>
#include <HardwareTimer.h>


#include <LiquidCrystal.h> // include the LCD library
// LED
const int led = PC13;

const int speakerPin = PA4; // Пин, к которому подключен "+" спикера

// Initialize the LCD
const int rs = PB11;
const int rw = PB10;
const int en = PB1;
const int d4 = PB0; 
const int d5 = PA7; 
const int d6 = PA6; 
const int d7 = PA5; 

LiquidCrystal lcd(rs, en, d4, d5, d6, d7); 

//------------------------------------------------
// Encoder
const int pinA = PB12;
const int pinB = PB13;
const int pinSW = PB14; // кнопка

// Rotary encoder pins
#define CLK_PIN  PA0
#define DT_PIN   PA1
#define SW_PIN   PA2

volatile int encoderDelta = 0;
volatile unsigned long lastTime = 0;

// Variables declaration
int8_t quad = 0;

//------------------------------------------------
// Меню
int menuIndex = 0;
bool editMode = false;

int value1 = 10;
int value2 = 20;

//------------------------------------------------
void setup() {
  Serial.begin(115200);

  Serial.println(F("START"));

  //------------------------------------------------
  // Воспроизвести звук частотой 1000 Гц (1 кГц) в течение 1 сек
  tone(speakerPin, 1000); 
  delay(5000);
  
  // Выключить звук
  noTone(speakerPin);

  //------------------------------------------------
  // put your setup code here, to run once:
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(led, OUTPUT);

  pinMode(speakerPin, OUTPUT);

  //------------------------------------------------
  // Encoder
//  pinMode(pinA, INPUT_PULLUP);
//  pinMode(pinB, INPUT_PULLUP);
//  pinMode(pinSW, INPUT_PULLUP);
//  
//  attachInterrupt(digitalPinToInterrupt(pinA), readEncoder, FALLING);

  // Pins configuration
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN,  INPUT_PULLUP);
  pinMode(SW_PIN,  INPUT_PULLUP);

    // Включить внутренние подтягивающие резисторы
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(DT_PIN, LOW);
  digitalWrite(SW_PIN, LOW);

  // Enable interrupt on change for rotary encoder CLK & DT pins
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), encoderStatus, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT_PIN),  encoderStatus, CHANGE);

  //------------------------------------------------
  pinMode(rs, OUTPUT);
  pinMode(rw, OUTPUT);
  pinMode(en, OUTPUT);
  pinMode(d4, OUTPUT);
  pinMode(d5, OUTPUT);
  pinMode(d6, OUTPUT);
  pinMode(d7, OUTPUT);

  digitalWrite(rw, LOW);
  
  lcd.begin(16, 2);//We are using a 16*2 LCD

  // lcd.setCursor(column, row) - нумерация с нуля
  lcd.setCursor(0, 0);
  lcd.print("                "); // очистка строки
  lcd.setCursor(0, 0);
  lcd.print("Interfacing str1"); //Print this
  
  lcd.setCursor(0, 1);
  lcd.print("                "); // очистка строки
  lcd.setCursor(0, 1);
  lcd.print("Interfacing str2"); //Print this
  
  delay(2000); //wait for two secounds

  lcd.clear(); //Clear the screen

  //------------------------------------------------
  drawMenu();
}
//------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:
  
//  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);              // wait for a second
//  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);              // wait for a second
//
//  handleEncoder();
//  handleButton();

    delay(100);   // Wait 10 milliseconds
}
//------------------------------------------------
// ===== ENCODER ISR =====
void readEncoder() {
  unsigned long now = micros();
  if (now - lastTime < 200) return;
  lastTime = now;

  if (digitalRead(pinB)) {
    encoderDelta++;
  } else {
    encoderDelta--;
  }
}
//------------------------------------------------
// ===== ОБРАБОТКА ЭНКОДЕРА =====
void handleEncoder() {
  if (encoderDelta == 0) return;

  int delta = encoderDelta;
  encoderDelta = 0;

  if (!editMode) {
    menuIndex += delta;

    if (menuIndex < 0) menuIndex = 0;
    if (menuIndex > 1) menuIndex = 1;
  } else {
    if (menuIndex == 0) value1 += delta;
    if (menuIndex == 1) value2 += delta;
  }

  drawMenu();
}
//------------------------------------------------
// ===== КНОПКА =====
void handleButton() {
  static bool lastState = HIGH;
  bool state = digitalRead(pinSW);

  if (lastState == HIGH && state == LOW) {
    delay(50); // debounce
    editMode = !editMode;
    drawMenu();
  }

  lastState = state;
}
//------------------------------------------------
// ===== ОТРИСОВКА =====
void drawMenu() {
  //lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print("                "); 
  
  // строка 1
  lcd.setCursor(0, 0);
  if (menuIndex == 0) lcd.print(">"); else lcd.print(" ");
  lcd.print("Val1: ");
  lcd.print(value1);

  // строка 2
  lcd.setCursor(0, 1);
  if (menuIndex == 1) lcd.print(">"); else lcd.print(" ");
  lcd.print("quad: ");
  lcd.print(value2);

  // режим
  if (editMode) {
    lcd.setCursor(14, 0);
    lcd.print("*");
  }
}
//------------------------------------------------
// Interrupt Service Routine (ISR)
void encoderStatus(void)
{
    static uint8_t LastRead = 3;
    uint8_t ThisRead = 0;
 
    ThisRead = (digitalRead(CLK_PIN) << 1) | (digitalRead(DT_PIN));
 
    if(ThisRead == LastRead)
      return;
 
    if( bitRead(ThisRead, 0) == bitRead(LastRead, 1) )
    {
      quad -= 1;

      value1 -= 1;
      //value2 -= 1;  
    } else {
      quad += 1;

      value1 += 1;
      //value2 += 1;
    }  
    
    LastRead = ThisRead;

    value2 += encoderGet();

    drawMenu();
}
//------------------------------------------------
// Function to calculate latest number of rotary encoder moves
int8_t encoderGet(void)
{
    int8_t val = 0;
    
    if( abs(quad) >= 4 )
    {
        val  = quad / 4;
        quad %= 4;
    }
 
    return val;
}
//------------------------------------------------
