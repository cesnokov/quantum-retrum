# Quantum Retrum - Digital Clock

A simple digital clock built with an **Arduino**, **DS3231 RTC**, and a **TM1652 4-digit LED display module**.
The project supports time/date display, temperature reading from the RTC, and full time/date configuration using three buttons.

![Quantum Retrum](/docs/quantum-retrum.jpg)

## Features

* 4-digit LED display driven by TM1652 (UART mode)
* Accurate timekeeping using DS3231 Real-Time-Clock
* Blinking colon between hours and minutes
* Display current **date**
* Display **temperature** from DS3231 internal sensor
* Full **time and date setup** using buttons
* Adjustable display brightness

## Hardware

* Arduino-compatible microcontroller
* WeAct Studio TM1652 0.8 Digital Tube Module [AliExpress](https://www.aliexpress.com/item/1005007337668399.html)
* DS3231 RTC module
* 3 push buttons
* TEMT6000 Ambient Light Sensor Module
* IKEA BEGÅVNING (703.439.38) Glass Dome [IKEA](https://www.ikea.com/lt/lt/p/begavning-stiklinis-gaubtas-su-pagrindu-70343938/)

### Pin connections

| Device       | Pin           | Arduino |
| ------------ | ------------- | ------- |
| TM1652       | SDA / UART RX | D1 (TX) |
| DS3231       | SDA           | A4      |
| DS3231       | SCL           | A5      |
| Button "SET" |               | D8      |
| Button "+"   |               | D6      |
| Button "-"   |               | D4      |
| TEMT6000     |               | A0      |

## Controls

| Button | Function                          |
| ------ | --------------------------------- |
| SET    | Enter Setup mode / Confirm        |
| +      | Increase value / Show Date        |
| -      | Decrease value / Show Temperature |



Libraries used: `adafruit/RTClib` and `Wire`. 
The display is controlled directly via `Serial.write()` without additional libraries. 
The TM1652 module operates in **UART mode** and requires odd parity: `Serial.begin(19200, SERIAL_8O1);`. 
The colon between hours and minutes is implemented using the decimal point of the second digit.

## License

MIT License
