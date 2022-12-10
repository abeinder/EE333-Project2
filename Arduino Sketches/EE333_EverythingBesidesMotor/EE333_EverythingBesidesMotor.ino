// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 26, en = 27, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int NUM_ITERS_BLUETOOTH = 100;
const int NUM_ITERS_LCD = 10000;

Servo servo1;
Servo servo2;

char msg[4];
char servo1Pos;
char servo2Pos;
bool increaseSpeed;
bool decreaseSpeed;
int q0;
int q1;
int q2;
int rw = 0;
int iters_b = 0;
int iters_l = 0;


void sendCommand(const char * command){
  Serial1.println(command);
  
  char reply[100];
  int i = 0;
  while (Serial1.available()) {
    reply[i] = Serial1.read();
    i += 1;
  }
  //end the string
  reply[i] = '\0';
}

void writeToBLE(char value) {
  Serial1.write(value);
}

void readFromBLE(char* reply) {
  int i = 0;
  while (Serial1.available()) {
    reply[i] = Serial1.read();
    i++;
  }

  reply[i] = '\0';
  
  
  return;
}

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  // Print a message to the LCD.
  delay(1000);

  Serial1.begin(115200);
  Serial.begin(115200);

  sendCommand("AT");

  servo1.attach(12);
  servo2.attach(13);
}

void handleBluetooth() {
  char message[20];

  int i;
  for (i=0; i<20; i++) {
    message[i] = ' ';
  }
  
  readFromBLE(message);
  int o = 0;
  int p = 0;

  char command[3];
  command[0] = 'A';
  command[1] = 'T';
  command[2] = '+';

  char blank[3];
  blank[0] = '\0';
  blank[1] = ' ';
  blank[2] = ' ';

  if (message[0] == command[0]) {
    if (message[1] == command[1]) {
      if (message[1] == command[2]) {
        o = 1;
      }
    }
  }

  if (message[0] == blank[0]) {
    if (message[1] == blank[1]) {
      if (message[1] == blank[2]) {
        o = 1;
      }
    }
  }
  
  delay(1);

  if (o != 1 && p != 1) {
    msg[0] = message[0];
    msg[1] = message[1];
    msg[2] = message[2];
    msg[3] = '\0';

    int q01;
    int q11;
    q01 = msg[0]-'0';
    q11 = msg[1]-'0';
    q2 = (msg[2]-'0'+48);

    //Serial.print(msg);
    //Serial.print("\n");

    if (q01 != -16 && q01 != 78 && q01 != 21 && q01 != 29 && q01 != 69) {
      q0 = (q01+48)*2;
      if (q0 < 55) {
        q0 = 55;
      }

      int speedCommand = msg[0] >> 5;
      if (speedCommand == 1) {
        increaseSpeed = true;
        //Serial.write("Increase speed\n");
      } else if (speedCommand == 2) {
        decreaseSpeed = true;
        //Serial.write("Decrease speed\n");
      }
      
      char servo1PosPotential = (((msg[0] - (speedCommand << 5)) + (msg[1] >> 6)) << 3)-1;
      byte valCast = servo1PosPotential;
      if (valCast < 128 && valCast != 7) {
        servo1Pos = servo1PosPotential;
        byte val = servo1Pos;
        Serial.println(val);
      }
      
      servo1Pos = servo1PosPotential;
      //Serial.write("Servo 1 rotation: ");
      //Serial.write(servo1Pos);
      //Serial.write("\n");
      
    }

    if (q11 != -16 && q11 != -48) {
      q1 = (q11+48)*2;

      char servo1LastBit = msg[1] >> 6;
      servo2Pos = (msg[1] - (servo1LastBit << 6)) << 2;
      //Serial.write("Servo 2 rotation: ");
      //Serial.write(servo2Pos);
      //Serial.write("\n");
    
    //sendCommand("AT+");
  }
  }
}

void handleLCD() {
  // set the cursor to column 0, line 1
    
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print(msg);
    lcd.print("   ");
    //Serial.print(msg);
    //Serial.print("\n");
    
}

void loop() {
  
    //Serial.write(msg);
    //Serial.write("\n");
    
  if (iters_b >= NUM_ITERS_BLUETOOTH) {
    handleBluetooth();
    //Serial.print(m);
    //Serial.print("\n");
    iters_b = 0;
    servo1.write(servo1Pos);
    servo2.write(servo2Pos);
  } else {
    iters_b++;
  }

  if (iters_l >= NUM_ITERS_LCD) {
    handleLCD();
    iters_l = 0;
  } else {
    iters_l++;
  }
    
  
}
