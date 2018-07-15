#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial SIM900(7,8);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

//const high of sensor dalam to bawah in CM
double maxTong = 48.0;

//relay pin
int pinrelay = 17;

//servo pin
int servoPin = 11;

//ultrasonic luar pin
int ultratrig1 = 2;
int ultraecho1 = 3;

//ultrasonic dalam pin
int ultratrig2 = 4;
int ultraecho2 = 5;

//pin auto switch on SIM900
int autosim900 = 9;

//variable for sensor
double capacity = 0.0;
int condition = 0;
boolean hasOpen = false;

void setup() {
  
  //initialize Serial
  Serial.begin(9600);
  servo.attach(servoPin);
  //initialize LCD and erase 
  lcd.begin();
  lcd.backlight();
  lcd.clear();

  /* initialize modem SIM900*/
  SIM900.begin(19200);
  pinMode(autosim900, OUTPUT);
  digitalWrite(9, HIGH);
  lcd.print("Init Modem...");
  delay(8000);
  if (SIM900.available()){
    lcd.clear();
    lcd.print("Modem Ready");
    delay(2000);
    lcd.clear();
  }
  /* end of init modem SIM900 */
  
  /*set pinMode */
  pinMode(ultratrig1, OUTPUT);
  pinMode(ultraecho1, INPUT);
  pinMode(ultratrig2, OUTPUT);
  pinMode(ultraecho2, INPUT);
  pinMode(pinrelay, OUTPUT);
  digitalWrite(pinrelay, HIGH);
  /* end of set pinMode */

  /* start condition */
  capacity = getPercent(getCapacity(), maxTong);
  
  /*while (true){
    if (capacity == 100.0)
      capacity = getPercent(getCapacity(), maxTong);
    else 
      break;
  }
  updateLCD(capacity);*/

  /* end of start condition */
  
}

void loop() {

  //define variable on detect human
  long timePeriodultra1, timePeriodultra2, ultracm1, ultracm2;

  //start with detect human, with ultrasonic depan
  digitalWrite(ultratrig1, LOW);
  delayMicroseconds(2);
  digitalWrite(ultratrig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultratrig1, LOW);
  
  //get first time period 
  timePeriodultra1 = pulseIn(ultraecho1, HIGH);

  //convert to cm in first pulse
  ultracm1 = microsecToCentimeters(timePeriodultra1);

  //check if human want throw rubish
  if (ultracm1 < 200 && ultracm1 > 0){
    Serial.println(ultracm1);
    //wait for 2 sec
    delay(2000);

    //get two time period
    digitalWrite(ultratrig1, LOW);
    delayMicroseconds(2);
    digitalWrite(ultratrig1, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultratrig1, LOW);
    
    timePeriodultra2 = pulseIn(ultraecho1, HIGH);
    
    //convert to cm in two pulse
    ultracm2 = microsecToCentimeters(timePeriodultra2);

    //check again for true someone want throw rubish
    if (ultracm2 < ultracm1 && ultracm2 < 150 ){
      Serial.println(ultracm2);
      //open gate
      if (hasOpen == false){
      openGate();
      }
      Serial.println("Sampah terbuka");
      onLamp();
      delay(6000);
      hasOpen = true;
    }
  }
  else if (ultracm1 >= 200){
    Serial.println("Sampah tertutup");
    offLamp();
    if (hasOpen == true){
      closeGate();
      condition = 1;
      hasOpen = false;
    }
    
  }

  delay(300);
 /* if (condition == 1){
    capacity = getPercent(getCapacity(), maxTong);
  
    while (true){
      if (capacity == 100.0)
        capacity = getPercent(getCapacity(), maxTong);
      else 
        break;
    }
    updateLCD(capacity);
    if (capacity > 85.00 ){
      sendSMS();
    }
    delay(3000);
    condition = 0;
  }
*/
  
}


/* Motor servo controller */

void openGate(){
  //create pulse for servo 

  //run servo in speed ratio 40 clockwise
  servo.write(35);
  //delay for stop rotation
  delay(1400);
  //stop rotation of servo 
  
  servo.write(90);

  //stop create pulse for servo 
}

void closeGate(){
  //create pulse for servo 

  //run servo in speed 140 other clockwise
  servo.write(180);
  delay(200);
  servo.write(90);
  //servo.detach();
  //stop create pulse for servo 
  delay(3000);
  servo.write(170);
  delay(200);
  servo.write(90);
  delay(3000);
  servo.write(170);
  delay(200);
  servo.write(90);

  //stop create pulse for servo 
}

/* End of Motor Servo controller */

/* LCD display controller */
void updateLCD(double status){  

  //set cursor on first line
  lcd.setCursor(0,0);
  lcd.print("Kapasitas Sampah");

  //set cursor on two line
  lcd.setCursor(0,1);
  lcd.print(status);
  lcd.print(" %");
}
/* End of LCD Display controller */

/* Sensor dalam controller */

double getPercent(double value,double max){
  //get percentage of capacity
  double percent = value/max*100.0;
  //return maximal size of content sampah
  return 100-percent;
}

//return is distance sensor with rubish in cm
double getCapacity(){

  //generate start pulse width 10 microsconds
  digitalWrite(ultratrig2, LOW);
  delayMicroseconds(2);
  digitalWrite(ultratrig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultratrig2, LOW);
  //get timePeriod of pulse from echo
  long timePeriod2 = pulseIn(ultraecho2, HIGH);
  return microsecToCentimeters(timePeriod2);
}

/* end of sensor dalam controller */


/* relay controller */
void onLamp(){
  digitalWrite(pinrelay, LOW);
}

void offLamp(){
  digitalWrite(pinrelay, HIGH);
}

/* end of relay controller */

/* MicrotoCM controller */
long microsecToCentimeters(long microsec){
  return microsec/29/2;
}
/* end of MicrotoCM cont */

/* sending SMS controller */

void sendSMS() {

  SIM900.print("AT+CMGF=1\r"); 
  delay(100);
  
  SIM900.println("AT + CMGS = \"+XXXXXXXXXXXX\""); //number of phone
  delay(100);
  
  SIM900.println("Status kapasitas sampah penuh"); 
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26); 
  delay(100);
  SIM900.println();
  delay(5000); 
}
/* end of sending SMS controller */
