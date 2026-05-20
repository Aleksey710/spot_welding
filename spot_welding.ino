/*
# Настройка среды
В менеджере плат подключить STM MCU based board
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

Плата:                STM32 MCU Based boards -> Generic STM32F1 series
Board part number:    BluePill F103C8
C runtime library:    Newlib Nano + Float Printf
------------------------------------------------------------------------
# Прототипы и инфо
https://microkontroller.ru/stm32-projects/nachalo-raboty-s-stm32-blue-pill-s-pomoshhyu-arduino-ide/
https://microkontroller.ru/stm32-projects/podklyuchenie-zhk-displeya-16x2-k-stm32-blue-pill-stm32f103c8t6/

https://github.com/HamsterTime-r/SpotWelder/tree/main
------------------------------------------------------------------------
# Компоненты:
BluePill(STM32F103C8T6)
1602A
Энкодер
------------------------------------------------------------------------
# Подключение
* - разрыв
- - соединение

Operating mode
USB  0-0*0  SWD
USB  0-0*0  SWD

Programming mode
USB  0*0-0  SWD
USB  0-0*0  SWD

-------------------------------
| FTDI232                     |
-------------------------------
| A9   | TX1  | RX            |
| A10  | RX1  | TX            |
-------------------------------
| LCD 1602                    |
-------------------------------
| 1  | VSS  | to ground       |
| 2  | VDD  | to 5V           |
| 3  | V0   |                 |
-------------------------------
| 4  | RS   | PB11            |
| 5  | RW   | PB10     X      |
| 6  | E    | PB1             |
-------------------------------
| 7  | D0   | X               |
| 8  | D1   | X               |
| 9  | D2   | X               |
| 10 | D3   | X               |
-------------------------------
| 11 | D4   | PB0             |
| 12 | D5   | PA7             |
| 13 | D6   | PA6             |
| 14 | D7   | PA5             |
-------------------------------
| 15 | A    |                 |
| 16 | K    |                 |
-------------------------------
| Звуковая сигнализация       |
-------------------------------
| BUZZER    | PA8             |
-------------------------------
| Энкодер                     |
-------------------------------
| pinA      | PB12            |
| pinB      | PB13            |
| pinSW     | PB14            |
-------------------------------
| Кнопка прожига              |
-------------------------------
| burn_sw   | PA3             |
-------------------------------
| Выход на управление прожигом|
-------------------------------
burn_signal | PA4             |
-------------------------------
*/
//------------------------------------------------
#include <Arduino.h>
#include <HardwareTimer.h>
#include <LiquidCrystal.h>  // include the LCD library

#include <FreeRTOS.h>
#include <task.h>

//------------------------------------------------
// Initialize the LCD
#define lcd_rs_pin          PB11
#define lcd_rw_pin          PB10
#define lcd_en_pin          PB1
#define lcd_d4_pin          PB0
#define lcd_d5_pin          PA7
#define lcd_d6_pin          PA6
#define lcd_d7_pin          PA5

LiquidCrystal lcd(lcd_rs_pin, lcd_en_pin, lcd_d4_pin, lcd_d5_pin, lcd_d6_pin, lcd_d7_pin); 

// Encoder
#define enc_a_pin           PA0
#define enc_b_pin           PA1
#define enc_sw_pin          PA2   // кнопка энкодера

#define burn_sw_pin         PA3   // кнопка прожига

#define burn_pin            PA4   // выход на силовой ключ

#define led_pin             PC13   // индикация прожига

#define speaker_pin         PA8    // "+" спикера
//------------------------------------------------
// Рабочие тайминги
volatile int16_t wt_pre_burn = 100;                // Время перед сварочными импульсами, программно убирает тримминг кнопки прожига (по умолчанию 0.1 сек)

#define wt_burn_min 100
#define wt_burn_max 499
volatile int16_t wt_burn     = wt_burn_min;        // длительность прожига одного импульса

#define wt_pause_min 100
#define wt_pause_max 999
volatile int16_t wt_pause    = wt_pause_max;       // задержка между сериями импульсов

#define burn_number_min 1
#define burn_number_max 5
volatile int16_t burn_number = burn_number_min;    // количество импульсов в серии

//------------------------------------------------
// режим работы 
enum WORK_MODE
{
  WAITING_WORK_MODE   = 0,
  CHANGE_WORK_MODE    = 1,
  BURN_WORK_MODE      = 2
};

volatile WORK_MODE work_mode;

// режим редактирования
enum MENU_LEVEL
{
  NO_MENU_LEVEL                = 0,
  CHECK_VALUE_TYPE_MENU_LEVEL  = 1,
  VALUE_EDIT_MENU_LEVEL        = 2
};

volatile MENU_LEVEL menu_level = NO_MENU_LEVEL;

// режим редактирования
enum EDIT_MODE
{
  NO_EDIT_MODE           = 0,
  MENU_INDEX_EDIT_MODE   = 1,
  WT_BURN_EDIT_MODE      = 2,
  WT_PAUSE_EDIT_MODE     = 3,
  BURN_NUMBER_EDIT_MODE  = 4
};

volatile EDIT_MODE edit_mode = NO_EDIT_MODE;

#define menu_index_min 1 // (MENU_INDEX_EDIT_MODE)
#define menu_index_max 4 // (BURN_NUMBER_EDIT_MODE)

volatile int menu_index = 0;

//------------------------------------------------
volatile bool lcdNeedUpdate = false;

//------------------------------------------------
volatile int encoderDelta       = 0;

// Для void print_timings()
char buffer[4];

//------------------------------------------------
void print_timings(volatile int16_t* time, uint8_t col, uint8_t row){  

  snprintf(buffer, sizeof(buffer), "%-3d", *time);

  lcd.setCursor(col,row);
  lcd.print(buffer);
}
//------------------------------------------------
void print_general()
{ 
  // Указатель на изменяемый параметр (только при изменении)
  if (menu_index == (uint8_t) WT_BURN_EDIT_MODE)     { lcd.setCursor( 0, 0); lcd.print(">"); } else { lcd.setCursor( 0, 0); lcd.print(" "); }
  if (menu_index == (uint8_t) WT_PAUSE_EDIT_MODE)    { lcd.setCursor( 0, 1); lcd.print(">"); } else { lcd.setCursor( 0, 1); lcd.print(" "); }
  if (menu_index == (uint8_t) BURN_NUMBER_EDIT_MODE) { lcd.setCursor(10, 0); lcd.print(">"); } else { lcd.setCursor(10, 0); lcd.print(" "); }

  //                        "%-4d"
  print_timings(&wt_burn,      4+1, 0);  
  print_timings(&wt_pause,     4+1, 1); 
  print_timings(&burn_number, 12+1, 0); 
  
  lcd.setCursor(10,1);
  switch (work_mode)
  {
    case WAITING_WORK_MODE : lcd.print("      "); break;
    case CHANGE_WORK_MODE :  lcd.print("  <*> "); break;
    case BURN_WORK_MODE :    lcd.print("CBAPKA"); break;
    default :                lcd.print("      "); break;
  }
}
//------------------------------------------------
// прожиг
void burning()
{
  Serial.println(F("start burning"));

  vTaskDelay(pdMS_TO_TICKS(wt_pre_burn));  // Время перед сварочными импульсами, программно убирает тримминг кнопки прожига
      
  work_mode = BURN_WORK_MODE;
  lcdNeedUpdate = true;

  for(int i = 0; i < burn_number; i++)
  {
    digitalWrite(burn_pin, LOW);           // страховка

    //tone(speaker_pin, 1000);             // включит писк  (1000 Гц)
    
    vTaskDelay(pdMS_TO_TICKS(wt_pause));   // задержка между импульсами

    digitalWrite(led_pin,  HIGH);          // включить индикацию прожига
    noInterrupts();                        // запретить ВСЕ прерывания
    digitalWrite(burn_pin, HIGH);          // начало прожига
    delayMicroseconds(wt_burn*1000);       // длительность прожига
    digitalWrite(burn_pin, LOW);           // конец прожига
    interrupts();                          // разрешить ВСЕ прерывания
    digitalWrite(led_pin,  LOW);           // выключить индикацию прожига
    
    //noTone(speaker_pin);                 // выключить писк 
  }

  work_mode = WAITING_WORK_MODE;
  lcdNeedUpdate = true;

  Serial.println(F("finish burning"));
}
//------------------------------------------------
void taskMain(void *p)
{
  while(1)
  {
    //Serial.println(F("taskMain"));

    static bool oldEncBtn = HIGH;

    bool s = digitalRead(enc_sw_pin);

    if(oldEncBtn == HIGH && s == LOW)
    {
            switch(menu_level)
      {
        case NO_MENU_LEVEL: // 0
          {
            menu_level = CHECK_VALUE_TYPE_MENU_LEVEL;                   
            edit_mode  = MENU_INDEX_EDIT_MODE;
            work_mode  = CHANGE_WORK_MODE;
            menu_index = menu_index_min; // перейти к редактированию
          } break;
        case CHECK_VALUE_TYPE_MENU_LEVEL: // 1
          {
            // if(menu_index == menu_index_min/*MENU_INDEX_EDIT_MODE = 1*/)   // выйти из редактирования
            // {
            //   menu_level = NO_MENU_LEVEL;              
            //   edit_mode  = NO_EDIT_MODE;
            //   work_mode  = WAITING_WORK_MODE;
            //   menu_index = 0;
            // } else // Перейти к редактированию параметра
            {
              menu_level = VALUE_EDIT_MENU_LEVEL;     

              switch(menu_index)
              {
                case 2/*WT_BURN_EDIT_MODE     */: edit_mode = WT_BURN_EDIT_MODE;      break;
                case 3/*WT_PAUSE_EDIT_MODE    */: edit_mode = WT_PAUSE_EDIT_MODE;     break;
                case 4/*BURN_NUMBER_EDIT_MODE */: edit_mode = BURN_NUMBER_EDIT_MODE;  break;
                default                         : edit_mode = WT_BURN_EDIT_MODE;      break;
              }
              
              work_mode  = CHANGE_WORK_MODE;
            }
          } break;
        case VALUE_EDIT_MENU_LEVEL: // 2
          {
            menu_level = NO_MENU_LEVEL; 
            edit_mode  = NO_EDIT_MODE;
            work_mode  = WAITING_WORK_MODE;
            menu_index = 0;
            
          } break;
      }

      lcdNeedUpdate = true;
    }

    oldEncBtn = s;
/*
    // Обработка смены режимов редактирования
    if(!digitalRead(enc_sw_pin))
    {

    };
*/
    // Если нажата кнопка прожига
    if(!digitalRead(burn_sw_pin))
    {
      //set_burning = true
      burning();
    } 

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
//------------------------------------------------
void encoderISR()
{
  if(digitalRead(enc_b_pin))
    encoderDelta--;
  else
    encoderDelta++;
}
//------------------------------------------------
void taskEncoder(void *p)
{
    int8_t delta;

    while(1)
    {
        //Serial.println(F("taskEncoder"));

        noInterrupts();
        delta = encoderDelta;
        encoderDelta = 0;
        interrupts();

        if(delta != 0)
        {
          switch(edit_mode)
          {
            case NO_EDIT_MODE:           {
              } break;
            case MENU_INDEX_EDIT_MODE:   {
              menu_index += delta;

              (menu_index  > menu_index_max)  ? menu_index  = menu_index_min  : menu_index;  
              (menu_index  < menu_index_min)  ? menu_index  = menu_index_max  : menu_index; 
              } break;
            case WT_BURN_EDIT_MODE:      { 
              wt_burn += delta;  
              (wt_burn  > wt_burn_max)        ? wt_burn     = wt_burn_min     : wt_burn;  
              (wt_burn  < wt_burn_min)        ? wt_burn     = wt_burn_max     : wt_burn; 
              } break;
            case WT_PAUSE_EDIT_MODE:     { 
              wt_pause += delta; 
              (wt_pause > wt_pause_max)       ? wt_pause    = wt_pause_min    : wt_pause; 
              (wt_pause < wt_pause_min)       ? wt_pause    = wt_pause_max    : wt_pause; 
              } break;
            case BURN_NUMBER_EDIT_MODE:  { 
              burn_number += delta; 
              (burn_number > burn_number_max) ? burn_number = burn_number_min : burn_number; 
              (burn_number < burn_number_min) ? burn_number = burn_number_max : burn_number; 
              } break;
          }

          lcdNeedUpdate = true;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
//------------------------------------------------
void taskLCD(void *p)
{
    // Начальный вывод (один раз отрисовать и больше не трогать для ускорения отображения)
    // +1 - место для вывода указателя на изменяемый параметр (при необходимости)
    lcd.setCursor( 0+1, 0); lcd.print("T_b:");  
    lcd.setCursor( 0+1, 1); lcd.print("T_p:");  
    lcd.setCursor(10+1, 0); lcd.print("n:");    

    while(1)
    {
        //Serial.println(F("taskLCD"));

        if(lcdNeedUpdate)
        {
            lcdNeedUpdate = false;
            print_general();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
//------------------------------------------------
void taskDebug(void *p)
{
    char debug_buffer[128];
    while(1)
    {
        Serial.println(F("------------------------------------------"));
        Serial.println(F("taskDebug"));
        snprintf(debug_buffer, sizeof(debug_buffer), "WM: %d, ML: %d, EM: %d, MI: %d, upd: %d", 
          (int)work_mode, 
          (int)menu_level, 
          (int)edit_mode, 
          (int)menu_index, 
          (int)lcdNeedUpdate
          );

        //snprintf(debug_buffer, sizeof(debug_buffer), "%-4d", *time);
        Serial.println(debug_buffer);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
//------------------------------------------------
void setup() {
  Serial.begin(115200);

  Serial.println(F("START"));
  //------------------------------------------------
  // pinMode:
  // INPUT            	вход
  // INPUT_PULLUP	      кнопки
  // INPUT_PULLDOWN    	кнопки
  // OUTPUT	            выход
  // OUTPUT_OPEN_DRAIN	open drain
  //------------------------------------------------
  pinMode(speaker_pin,  OUTPUT);
  pinMode(led_pin,      OUTPUT);  
  pinMode(burn_pin,     OUTPUT);

  pinMode(burn_sw_pin,  INPUT_PULLUP);

  //------------------------------------------------
  // Encoder
  pinMode(enc_a_pin,    INPUT_PULLUP);
  pinMode(enc_b_pin,    INPUT_PULLUP);
  pinMode(enc_sw_pin,   INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(enc_a_pin), encoderISR, FALLING);
  //attachInterrupt(digitalPinToInterrupt(enc_b_pin), encoderISR, CHANGE);
  
  //------------------------------------------------
  // LCD
  pinMode(lcd_rs_pin,   OUTPUT);
  pinMode(lcd_rw_pin,   OUTPUT);
  pinMode(lcd_en_pin,   OUTPUT);
  pinMode(lcd_d4_pin,   OUTPUT);
  pinMode(lcd_d5_pin,   OUTPUT);
  pinMode(lcd_d6_pin,   OUTPUT);
  pinMode(lcd_d7_pin,   OUTPUT);

  //------------------------------------------------
  // Воспроизвести звук частотой 1000 Гц (1 кГц)
  tone(speaker_pin, 1000); 
  digitalWrite(led_pin, HIGH);
  
  delay(1000);

  digitalWrite(led_pin, LOW);
  
  // Выключить звук
  noTone(speaker_pin);

  //------------------------------------------------
  digitalWrite(lcd_rw_pin, LOW);
  
  lcd.begin(16, 2);              // We are using a 16*2 LCD

  // lcd.setCursor(column, row) - нумерация с нуля
  lcd.setCursor(0, 0);
  lcd.print("                "); // очистка строки
  lcd.setCursor(0, 0);
  lcd.print("Spot welding v.1"); // Print this
  
  lcd.setCursor(0, 1);
  lcd.print("                "); // очистка строки
  lcd.setCursor(0, 1);
  lcd.print("  (2026.05.05)  "); // Print this
  
  delay(2000);                   // wait for two secounds

  lcd.clear();                   // Clear the screen

  //------------------------------------------------
  print_general();

  interrupts();               // разрешить ВСЕ прерывания

  xTaskCreate(taskMain,       "taskMain",      256,  NULL,1,NULL);

  xTaskCreate(taskEncoder,    "taskEncoder",   256,  NULL,1,NULL);
  xTaskCreate(taskLCD,        "taskLCD",       512,  NULL,1,NULL);
  xTaskCreate(taskDebug,      "taskDebug",     256,  NULL,1,NULL);
  
  vTaskStartScheduler();
}
//------------------------------------------------
void loop() 
{
}
//------------------------------------------------
