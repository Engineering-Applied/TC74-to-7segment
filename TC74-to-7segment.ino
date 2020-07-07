/* TC74 Temperature sensor to 2 digit 7-segment display
 * Simple Temperature to display
 * Created 05/07/20
 */

/* Variables for SIPO Shift Register */
int data1Pin = 2;
int data2Pin = 3;
int latch1Pin = 4;
int latch2Pin = 5;
int clock1Pin = 6; 
int clock2Pin = 7;
int reset = 8;
#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  pinMode(data1Pin, OUTPUT);
  pinMode(data2Pin, OUTPUT);
  pinMode(latch1Pin, OUTPUT);
  pinMode(latch2Pin, OUTPUT);
  pinMode(clock1Pin, OUTPUT);
  pinMode(clock2Pin, OUTPUT);
  pinMode(reset, OUTPUT);
  digitalWrite(reset, LOW);
  delay(50);
  digitalWrite(reset, HIGH);
}

/* Value to BCD pin conversion
 *  Takes in the value of tens/ones place and returns respective BCD segments
 *  From MSB to LSB, A to G then last bit is DP(unused)
 *  Returned value is of Byte Format
 *  Created on 05/09/20
 */
byte BCD(int value) {
  byte sequence;
  switch(value){
    case 1:
      sequence = 0b01100000;
      break;
    case 2:
      sequence = 0b11011010;
      break;
    case 3:
      sequence = 0b11110010;
      break;
    case 4:
      sequence = 0b01100110;
      break;
    case 5:
      sequence = 0b10110110;
      break;
    case 6:
      sequence = 0b10111110;
      break;
    case 7:
      sequence = 0b11100000;
      break;
    case 8:
      sequence = 0b11111110;
      break;
    case 9:
      sequence = 0b11110110;
    default:
      sequence = 0b11111100;
      break;
  }
  // Serial.println(sequence, BIN);
  return sequence;
}

/*Shift-Register byte shifter
 * Receives the byte to display and shifts it to the BCD
 * Created on 05/10/20
 */
void shifting(int SIPO, byte data){
  //Serial.println(SIPO);
  if (SIPO == 1) {
    digitalWrite(latch1Pin, LOW);
    shiftOut(data1Pin, clock1Pin, MSBFIRST, data);
    delay(20);
    digitalWrite(latch1Pin, HIGH);
  }
  if (SIPO == 2) {
    digitalWrite(latch2Pin, LOW);
    shiftOut(data2Pin, clock2Pin, MSBFIRST, data);
    delay(20);
    digitalWrite(latch2Pin, HIGH);
  }
}

void loop() {
  byte TC74 = 0b1001000;         // TC74 Address
  Wire.beginTransmission(TC74); // Talk to Temperature device with the 0x48 (7-bit) address
  Wire.write(0);                     // Tell device to read from temperature register with address 0x00
  Wire.endTransmission();           // Transmits write bytes and sends stop byte to end master command

  Wire.requestFrom(TC74, 1);      // Request one Byte from the specified slave device
  while(Wire.available() == 0); // Delay until byte is received
  int temp_c = Wire.read();    // Feed read byte
  int temp_f = (temp_c * 9/5) + 32;
  Serial.print("The temperature is ");
  Serial.print(temp_f);
  Serial.println("* Fahrenheit");

/*For translating temperature to BCD values (0-9)
  *Starts from tens place then to ones place
*/
  int tens = temp_f / 10;   // Tens place value determined
  int ones = temp_f % 10;   // Ones place value determined
                           //Serial.print("The First Digit is ");        
                           //Serial.print(tens);                         // For Debugging Purposes
                           //Serial.print("\t The Second Digit is ");
                          //Serial.println(ones);
  /*For shifting out translated temperature to BCDs
    *Shifts to first segment then second segment
   */
  int SIPO = 1;
  byte Segment1 = BCD(tens);  //Convert tens value to input bit sequence for BCD 1
  shifting(SIPO, Segment1);   //Shifts the tens value to BCD1
  SIPO = 2;
  byte Segment2 = BCD(ones);  //Convert ones value to input bit sequence for BCD 2
  shifting(SIPO, Segment2);   //Shifts the ones value to BCD2
  delay(1000);                // Wait 1 second before talking again  
}
