
This is a simple servo testor, using the Arduino 'servo' library.

Platforms:
  It uses the Arduino 'Servo' library. Any platform supported by that library *should* work,
    although it was developed (and only tested) on an Ardiuno Nano. This would includee the 
    entire Arduino line, any ESP32's, and a lot of other processor types

  The file 'Configuration.h' contains all the definitions (pin numbers, timings, etc) that
    may need to be adjusted to adapt this code to another processor.

Wiring:
  The wiring is simple. A servo has 3 pins
    * power (5Volt) which is normally red. Connect this to the 5 volt pin on an Arduino, or
      from any 5 volt power source. Current draw can get to 1 amp under some conditions.

    * Ground which is typically Brown or black. Connect to Ground. Note that the ground needs to be common to BOTH the ground on the processor, AND the 5 volt source.
  
    * Signal - this can be connected directly to the data output pin of your processor - in my code, its pin D2 on the NANO.


The frequency of the pulses is 50/sec - 20 milliseconds. This is 'standard' for hobby sevros.
The pulse width can be varied from 500 to 2500 uSecs by default.

USAGE:
  Plug in the servo under test, and start a terminal emulator program (I use putty) and connect to the appropriate 'COM' device.

  The PWM is set for 1000 usecs by default, which should be somewhere near the center of the servo's range.

  The screen should show a list of commands near the top, and the current settings starting in the middle.  Note the first column in the settings list is a '+' (plus sign) for values settable by
  the user.  The '-' (miinus sign) indicates values that are calculated.

  Most are self explanattory - they set the indicated value (min, max, set(position), cycle).
  
  The 'run' command will cycle the servo a number of times (1 time if not specified).
  * First, the servo is set to the minimum position. A delay of 1 second ensures that the
     servo has time to move there.

  * The servo position is then set from the configured min (usecs) to max (usecs). The step
    rate, step size, etc are all as defined in the 'Current Settings' display. This will continue
    for the requested number of times.

  * At any time, the 'STOP' command can be typed to stop the sequencer.
  
