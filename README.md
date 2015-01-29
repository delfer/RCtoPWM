# RCtoPWM
RC to PWM converter based on AVR

Designed for at90usb162 @ 16MHz

It equals ESC for brushed motors.

Input RC signal on PC7. It is PWM signal about 50Hz with Hi period from 1ms (logical 0%) to 2ms (100%).

Output to PB6 (logical 0 or 1, for reverse) and PB7 (PWM output 1kHz).

Device can work in reverce and no reverce mode.
