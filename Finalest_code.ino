//#include <SafeString.h>
#include <Servo.h>
#include <Arduino.h>
#include <Stepper.h>
#include "FSR.h"
#include <LiquidCrystal.h>
#include "ClickEncoder.h"
#include <TimerOne.h>

#define FSR_PIN_1	A10

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
using namespace std; // not sure if necessary

//force sensor variables
FSR fsr(FSR_PIN_1);

//stepper variables
const int stepsPerRevolution = 32;
Stepper myStepper = Stepper(stepsPerRevolution, 11, 9, 10, 8);

//lcd variables
const int rs = 22, en = 2, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//servos variables
int pos = 0;
int pos2 = 0;
int turning_angle=0;
Servo myservo2;
Servo myservo;

// running test variables
int softPoint = 100;
int heavyPoint = 1000;
int test = 1;
int selected_test = 1;
int counter =0;
int temp;
int minForce;
int maxForce;

const int numReadings = 10;
int readings[numReadings];

float max_val;

boolean hard;
boolean heavy;

boolean IR ;


//time variables
unsigned long time;
long time0;

//menu logic variables
int menuitem = 1;
int page = 1;
int setting = 1;
int option = 1;
int speed = 5;

boolean up = false;
boolean down = false;
boolean middle = false;
boolean running = false;

String testType[4] = {"Single Run","Continuous","IR Activated","Test Sensor"};

String setting_selection[2][2] = { {"Speed", "Test"}, {"Hardness", "Weight"} };

//rotary encoder vars
ClickEncoder *encoder;
int16_t last, value;


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  encoder = new ClickEncoder(A1, A0, A2);
  encoder->setAccelerationEnabled(false);
   
  lcd.begin(16, 2);    
  lcd.write("Starting up");

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }  

  while (millis() < 5000) 
  {
    float sensorValue = fsr.getForce();

    // record the maximum sensor value
    if (sensorValue > maxForce) {
      maxForce = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < minForce) {
      minForce = sensorValue;
    }
  }

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = encoder->getValue();

  myservo.attach(12);  

  analogReference(DEFAULT);

  lcd.clear(); 

  Menu();

  Serial.begin(921600);
  while (!Serial);

}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  this function is called to execute any code that is run by the program - here it determines what test to run and with what parameters
  menuitem checks where it was called
    = 1 means run test
    = 2 shouldnt be able to happen
    = 3 calibrate
  the option is different depending on the code running but determines what value goes into the function being called
*/
void runCode(int menuitem, int option = 1)
{
  if (menuitem == 1 )
  {
    myStepper.setSpeed(speed*5);
    do
    {    
      temp = bouncetest(selected_test,3);
      readClick();
      if (middle)
      {
        running = false;
        middle = false;
      }
    }while (running);
  }

  else if (menuitem == 3)
  {
    if (option == 1)
    {
      lcd.clear();
      lcd.print("Hardness Point");
      delay(1000);
      softPoint = calibrate(option);
    }
    else if (option == 2)
    {
      lcd.clear();      
      lcd.print("Weight Reference");      
      delay(1000);

      heavyPoint = calibrate(option);

    }
  }
  Menu();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void displayIntMenuPage(String title, int choice = 1)
{
  lcd.setCursor(0, 0);
  lcd.print("   "+title+"          ");
  lcd.setCursor(0, 1);

  if (page==2)
  {
    lcd.print(">"+setting_selection[menuitem-2][choice-1]+"          ");
  }    

  else if (page == 3 && choice == 1)
  {
    lcd.print("     ");
    lcd.print(speed);
    lcd.print("     ");
  }

  else if (page ==3 && choice == 2)
  {
    lcd.print(">"+testType[test-1]+"     ");    
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/* main line of  code - runs the display then checks for any updates from the controls --------------------------------------------*/

void loop() 
{
  Menu();
  readRotaryEncoder();
  readClick(); 

 /*checks how to update display when knob is turned anti-clockwise*/

  if (up && page == 1 ) 
  {    
    up = false;

    menuitem--;

    if (menuitem == 0)
    {
      menuitem = 3;
    } 
  }

  else if (up && page == 2 && menuitem == 1 ) 
  {
    up = false;
  }

  else if (up && page == 2 && menuitem == 2 ) 
  {
    up = false;

    setting++;

    if (setting == 3)
    {
      setting = 1;      
    }
  }

  else if (up && page == 2 && menuitem == 3 ) 
  {
    up = false;

    option++;

    if (option == 3)
    {
      option = 1;      
    }
  }

  else if (up && page == 3 && menuitem == 2 && setting == 1)
  {
    up = false;

    speed++;

    if (speed == 11)
    {
      speed = 1;
    }
  }

  else if (up && page == 3 && menuitem == 2 && setting == 2 )
  {
    up = false;

    test++;

    if (test == 5)
    {
      test = 1;
    }    
  }

 /*checks how to update display when knob is turned clockwise*/

  if (down && page == 1) 
  {
    down = false;
    menuitem++;  
    if (menuitem == 4) 
    {
      menuitem = 1;
    }
  }

  else if (down && page == 2 && menuitem == 1) 
  {
    down = false;
  }

  else if (down && page == 2 && menuitem == 2) 
  {
    down = false;
    setting--;
    if (setting == 0)
    {
      setting = 2;
    }
  }

  else if (down && page == 2 && menuitem == 3 ) 
  {
    down = false;
    option--;

    if (option == 0)
    {
      option = 2;      
    }
  }

  else if (down && page == 3 && menuitem == 2 && setting == 1 )
  {
    down = false;
    speed--;
    if (speed == 0)
    {
      speed = 10;
    }    
  }

  else if (down && page == 3 && menuitem == 2 && setting == 2 )
  {
    down = false;
    test--;

    if (test == 0)
    {
      test = 4;
    }    
  }

 /*checks how to update display when an option is selected*/

  if (middle)
  {
    middle = false;

    if (page == 1 && menuitem != 1) 
    {
      page=2;
    }

    /* checks the different options where code needs to run */
    else if ((page == 2 && menuitem != 2) || (page == 1 & menuitem == 1) || (page == 2 && menuitem != 4)) 
    {
      page=1; 
      displayIntMenuPage("Running Test");
      runCode(menuitem, option);
    }

    else if (page == 2 && menuitem == 2)
    {
      page=3;      
    }

    else if (page == 3)
    {
      page=1;
      selected_test = test;
    }

    Menu();

  }
}   

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*function which updates how the knob has been turned*/
void readRotaryEncoder()
{
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    delay(150);
    lcd.clear();
    Menu();
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    delay(150);
    lcd.clear();
    Menu();
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*function which checks if the button has been pressed*/
void readClick()
{
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
    case ClickEncoder::Clicked:
        middle=true;
      break;
    }
  }   
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*function to update the menu display given the current screen it takes a string to display first*/
void displayMenuItem(String item)
{
  lcd.setCursor(0, 0);

  lcd.print("   MAIN MENU          ");

  lcd.setCursor(0, 1);

  lcd.print(">"+item+"        ");
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*idrk what this does but its a part of the button check*/
void timerIsr() 
{
  encoder->service();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  when this is called it will update the menu based on what has been selected - 
  page 1 means nothing has been selected
  page 2 means its inside of the previosly selected option
  page 3 is the same as 2
*/
void Menu()
{
  if (page==1) 
  {    
    
    lcd.setCursor(3, 0);
    lcd.print("MAIN MENU");

    if(menuitem == 1)
    {   
      displayMenuItem("Start");
    }

    else if(menuitem == 2)
    {
      displayMenuItem("Settings");
    }

    else if(menuitem == 3)
    {
      displayMenuItem("Calibrate");
    }
  }

  else if (page == 2 && menuitem == 2) 
  {
   displayIntMenuPage("Settings", setting);
  }

  else if (page == 2 && menuitem == 3) 
  {
   displayIntMenuPage("Calibrate", option);
  }

  else if (page == 3 && menuitem == 2)
  {
    displayIntMenuPage(setting_selection[0][setting-1], setting);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  calibrate function returns the value at which the ball is hard or heavy depending on the option selected
  the option input is carried forwaed to the test function
*/
int calibrate(int option)
{
  int val1;
  int val2;

  //hardness
  lcd.clear();
  lcd.print("feed 1st ball");
  delay(100);

  val1 = bouncetest(1,option);

  lcd.clear();
  lcd.print("feed 2nd ball");
  delay(100);

  val2 = bouncetest(1,option);

  return max(val1,val2);

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
`function which checks if there is an IR detection for the feeding mechanism
*/
boolean readIR()
{
  return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  this function carries out the tests
  test type determines how (many times) the test is ran
    test type 1 means the test will run once
    test type 2 means the test will continually run until the button is pressed
    test type 3 means the test will only run when the IR sensor detects a ball being fed
    test type 4 means it wont run any test but only output the sensor values to serial for testing purposes

  returnval is used to check which value to return - this is used for calibration or tests
    returnVal 1 means it only returns hardness
    returnVal 2 means it only returns weight
    returnVal 3 means it will check the values and print to the lcd
*/
int bouncetest(int test_type, int returnVal)
{

  if (test_type == 1)
  {
    //single run
    running = false;

  }
  else if (test_type == 2)
  {
    //cont. run
    running = true;

  }
  else if (test_type == 3)
  {
    //ir activated
    running = true;
  } 

  else if (test_type == 4)
  {

    float fsrForce;
    float total = 0;

    do
    {

      float tempForce = fsr.getForce();

      for (int readIndex=0;readIndex<numReadings;readIndex++)
      {
        fsrForce = fsr.getForce();  
        fsrForce = max(fsrForce, maxForce);
        readings[readIndex] = fsrForce;
        total = total + readings[readIndex];
      }
    
      tempForce = total/numReadings;    
      total = 0;

      Serial.println(tempForce);

      readClick();

    }while (!middle);
  }


  if ( test_type == 3 )
  {
    do
    {
      boolean IR = readIR();
    }
    while (!IR);
    IR = false;
  }

  myStepper.step(stepsPerRevolution/4);
  lcd.setCursor(0,1);
  lcd.print("ball fed");

  int measuredForce;
  int max_val = 0;

  time = millis();
  
  int total=0;
  int fsrForce;
  int tempForce = 0;
  int force = 0;

  while (tempForce>=force || millis()-time < 10)
  {
    force = tempForce;
    time0 = millis();

    for (int readIndex=0;readIndex<numReadings;readIndex++)
    {
      fsrForce = fsr.getForce();  
      fsrForce = max(fsrForce, maxForce);
      readings[readIndex] = fsrForce;
      total = total + readings[readIndex];
    }
    
    tempForce = total/numReadings;    
    total = 0;

    Serial.println(tempForce);
    
  }

  max_val = force;

  int bounceTime = ((millis()-time0)*2);

  /*
  gets readings for one bounce
  readings are compared to setup values from background noise
  the readings are saved to two diff variables
  */
  if (returnVal == 1)
  {
    //returns hardness value mapped 
    return bounceTime; 
  }

  else if (returnVal == 2)
  {
    //returns weight value mapped
    return max_val;
  }

  else if (returnVal ==3)
  {
    //outputs value of both tests onto lcd

    if (max_val >= heavyPoint)
    {
      heavy = true;
    }
    else
    {
      heavy = false;
    }

    if (bounceTime >= softPoint)
    {
      hard = false;
    }
    else
    {
      hard = true;
    }

    lcd.clear();

    if (hard==true)
    {
      if (heavy==true)
      {
        lcd.print("hard & heavy");
      }
      else if (heavy == false)
      {
        lcd.print("hard & light");
      }
    }

    else if (hard == false)
    {
      if (heavy==true)
      {
        lcd.print("soft & heavy");
      }            
      else if (heavy == false)
      {
        lcd.print("soft & light");
      }
    }

    counter++;
    return 0;

  }
  delay(2000);
}


