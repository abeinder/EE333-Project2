// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 28, en = 29, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int NUM_ITERS_BLUETOOTH = 10;
const int NUM_ITERS_LCD = 10000;

Servo servo1;
Servo servo2;

char msg[4];
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
  // Print a message to the LCD.
  lcd.print("hello, world!");
  delay(1000);

  Serial1.begin(115200);
  Serial.begin(9600);

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
  //Serial.print("message: ");
  //Serial.print(message);

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
    
    sendCommand("AT+");
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
  
    int m;
    m = atoi(msg[0]);
    Serial.write(msg);
    Serial.write("\n");
    servo1.write(msg[0]);
    servo2.write(msg[1]);
    
  if (iters_b >= NUM_ITERS_BLUETOOTH) {
    handleBluetooth();
    //Serial.print(m);
    //Serial.print("\n");
    iters_b = 0;
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
