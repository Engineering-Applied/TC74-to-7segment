# TC74-to-7segment
This project shows my progress over creating the project via Arduino to the PIC18F14K50

The process of the project is by
   1) Gather I2C temperature from the TC74 Temperature Sensor
   2) Decipher the temperature byte to degrees celsius
   3) Convert the temperature to degrees fahrenheit
   4) Convert the tens place value to a byte to send to SIPO shift register 1
   5) Convert the ones place value to a byte to send to SIPO shift register 2
   6) Parallel shift each byte to the 7-segment displays
   
   
The code was originally written in the Arduino IDE for theoretical testing purposes
      and then the code was then ported to MPLAB c code 
