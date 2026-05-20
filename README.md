## Контроллер точечной сварки из подручных компонентов.

BluePill(STM32F103C8T6)
1602A
Энкодер

# Прототипы и инфо
https://microkontroller.ru/stm32-projects/nachalo-raboty-s-stm32-blue-pill-s-pomoshhyu-arduino-ide/
https://microkontroller.ru/stm32-projects/podklyuchenie-zhk-displeya-16x2-k-stm32-blue-pill-stm32f103c8t6/

https://github.com/HamsterTime-r/SpotWelder/tree/main

# Настройка среды
В менеджере плат подключить STM MCU based board
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

Плата:                STM32 MCU Based boards -> Generic STM32F1 series
Board part number:    BluePill F103C8
C runtime library:    Newlib Nano + Float Printf

# Компоненты:
BluePill(STM32F103C8T6)
1602A
Энкодер

# Подключение
* - разрыв
- - соединение

Operating mode
0-0*0  
0-0*0  

Programming mode
0*0-0  
0-0*0  

# FTDI232                     
| MCU |      |                |
|-----|------|----------------|
| A9  | TX1  | RX             |
| A10 | RX1  | TX             |

# LCD 1602                    
| №   | LCD  | MCU            |
|-----|------|----------------|
| 1   | VSS  | to ground      |
| 2   | VDD  | to 5V          |
| 3   | V0   |                |
|-----|------|----------------|
| 4   | RS   | PB11           |
| 5   | RW   | PB10     X     |
| 6   | E    | PB1            |
|-----|------|----------------|
| 7   | D0   | X              |
| 8   | D1   | X              |
| 9   | D2   | X              |
| 10  | D3   | X              |
|-----|------|----------------|
| 11  | D4   | PB0            |
| 12  | D5   | PA7            |
| 13  | D6   | PA6            |
| 14  | D7   | PA5            |
|-----|------|----------------|
| 15  | A    |                |
| 16  | K    |                |


# Звуковая сигнализация       
| BUZZER    | MCU             |
|-----------|-----------------|
|  +        | PA8             |

# Энкодер
|           | MCU             |                     
|-----------|-----------------|
| pinA      | PB12            |
| pinB      | PB13            |
| pinSW     | PB14            |

# Кнопка прожига              
|           | MCU             |
|-----------|-----------------|
| burn_sw   | PA3             |

# Выход на управление прожигом
|            | MCU            |
|------------|----------------|
|burn_signal | PA4            |

