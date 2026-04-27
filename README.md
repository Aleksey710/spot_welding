## Контроллер точечной сварки из подручных компонентов.

# Настройка среды
https://microkontroller.ru/stm32-projects/nachalo-raboty-s-stm32-blue-pill-s-pomoshhyu-arduino-ide/
https://microkontroller.ru/stm32-projects/podklyuchenie-zhk-displeya-16x2-k-stm32-blue-pill-stm32f103c8t6/

# Компоненты:
BluePill(STM32F103C8T6)
1602A
Энкодер

# Подключение
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
