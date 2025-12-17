
This is a simple servo testor.  Plug the ESP32 into a USB port and open a terminal.
Plug the Servo into the ???

On the termina, issue any of the following commands:

help or ?
cur <uSeconds> (report current)
min <uSeconds> (set min position)
max <uSeconds> (set max position)
set <uSeconds> (set current position)
run <time_sevs>  (run min to max and back, taking this much time)
x               (Stop)
  uSeconds:  (default is 500 to 2500)
    9999 the exact value in uSeconds
    +999 increment by this amount. Limit is 2000 uSec
    -999 decrement by this amount. Limit is 100 uSecs

LIMITED to MIN_PULSE_WIDTH to MAX_PULSE_WIDTH (in servo library)
