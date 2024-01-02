#include "Arduino.h"
#include <ThingerESP8266.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "uRTCLib.h"
#include <Key.h>
#include <Keypad.h>
#include <Keypad_I2C.h>


#define THINGER_SERIAL_DEBUG
ThingerESP8266 thing("Ali_Mehdipour", "D1", "YFbhDSkVJjEJM8s_");


//functions : 
void fan2_controll(int);
void fan1_controll(int);
void start_mode(int);
void resume_mode(int);

//What is the type of egg inside incubator?
int working_mode = 2;

//How many days eggs need to hatch
int hatching_duration = 1;

// first n days setting 
int first_n_days = 0;

// second m days setting
int second_n_days = 0;


// the time between eaech turn
int motor_interval = 0;
 
//TEMP & HUMIDITY =====================================
#define DHTPIN2 D5    // external sensor
#define DHTPIN1 D9    // internal sensor

//Declarations
#define DHTTYPE1 DHT22   // DHT 22
#define DHTTYPE2 DHT22   // DHT 22
DHT dht1(DHTPIN1, DHTTYPE1);
DHT dht2(DHTPIN2, DHTTYPE2);

//Variables
double h1, t1;
double h2, t2;
//====================================
//relays pins
int heater = D1; // HEATER
int led = D2; // پین رله 2
int humidifier = D0;
//LCD===================================
// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
//RTC======================================
// uRTCLib rtc;
uRTCLib rtc(0x68);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//FANS=====================================
// Motor A connections
int enA = D8;
int in1 = D11;
int in2 = D10;

int i =25;
// Motor B connections
int enB = D12;
int in3 = D6;
int in4 = D7;

int fan1_speed=0,fan2_speed=0;
//=============================================
//KEYPAD=======================================
#define I2CADDR 0x20 // Set the Address of the PCF8574

const byte ROWS = 4; // Set the number of Rows
const byte COLS = 4; // Set the number of Columns

// Set the Key at Use (4x4)
char keys [ROWS] [COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// define active Pin (4x4)
byte rowPins [ROWS] = {0, 1, 2, 3}; // Connect to Keyboard Row Pin
byte colPins [COLS] = {4, 5, 6, 7}; // Connect to Pin column of keypad.

// makeKeymap (keys): Define Keymap
// rowPins:Set Pin to Keyboard Row
// colPins: Set Pin Column of Keypad
// ROWS: Set Number of Rows.
// COLS: Set the number of Columns
// I2CADDR: Set the Address for i2C
// PCF8574: Set the number IC
Keypad_I2C keypad (makeKeymap (keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574_size);




//USER SETTINGS ===========
double prefer_temp_min=37;
double prefer_temp_max=38;
double prefer_hum_min =50;
double prefer_hum_max =60;

int state = 0 ;
void setup() {
  delay(3000); // wait for console opening
  delay(500);
  dht1.begin();
  dht2.begin();
  Wire .begin (); // Call the connection Wire
  keypad.begin (makeKeymap (keys)); // Call the connection
  // تنظیم پین‌های رله به حالت خروجی
  pinMode(heater, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(humidifier, OUTPUT);
  digitalWrite(heater, HIGH); 
  digitalWrite(led, HIGH);
  digitalWrite(humidifier,HIGH);

    // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

//RTC==================================================
  // rtc.set(40, 6, 22, 2, 24,7, 23);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Sunday, 7=Saturday)
//FANS=================================================
	// Set all the motor control pins to outputs
	pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
	pinMode(in1, OUTPUT);
	pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
	pinMode(in4, OUTPUT);
  analogWriteFreq(20);

	// Turn off motors - Initial state
	digitalWrite(in1, LOW);
	digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
	digitalWrite(in4, LOW);


//THINGER SETTINGS 
   thing.add_wifi("ayda.arash", "1261241134");
   	thing["DHT22-IN"] >> [](pson& out){
		// Add the values and the corresponding code
		out["humidity"] = dht1.readHumidity();
		out["celsius"] = dht1.readTemperature();
	};
     	thing["DHT22-OUT"] >> [](pson& out){
		// Add the values and the corresponding code
		out["humidity"] = dht2.readHumidity();
		out["celsius"] = dht2.readTemperature();
	};
  thing["LED_IN"] << [](pson& in){
    if(in.is_empty()){
      in = (bool) digitalRead(led);
      }
    else{
      digitalWrite(led, in ? HIGH : LOW);
      }
    };
      thing["HEATER"] >> [](pson& out){
      out["heater_state"] = (bool)digitalRead(heater);
    };
      thing["HUMIDIFIER"] >> [](pson& out){
      out["humidifier_state"] = (bool)digitalRead(humidifier);
    };
	
      thing["prefer_temp_min"] << [](pson& in){
    if(in.is_empty()){
      in = prefer_temp_min;
      }
    else{
        if((double)in<=prefer_temp_max)
          prefer_temp_min = in;
        else
        in = prefer_temp_min;
      }
    };
  
      thing["prefer_temp_max"] << [](pson& in){
    if(in.is_empty()){
      in = prefer_temp_max;
      }
    else{
      if((double)in>=prefer_temp_min)
        prefer_temp_max = in;
      else
        in = prefer_temp_max;
      }
    };

      thing["prefer_hum_min"] << [](pson& in){
    if(in.is_empty()){
      in = prefer_hum_min;
      }
    else{
        if((double)in<=prefer_hum_max)
          prefer_hum_min = in;
        else
        in = prefer_hum_min;
      }
    };

      thing["prefer_hum_max"] << [](pson& in){
    if(in.is_empty()){
      in = prefer_hum_max;
      }
    else{
      if((double)in>=prefer_hum_min)
        prefer_hum_max = in;
      else
        in = prefer_hum_max;
      }
    };
     thing["Fans_speed"] >> [](pson& out){
		// Add the values and the corresponding code
		out["fan1_speed"] = fan1_speed;
		out["fan2_speed"] = fan2_speed;
	};
	thing["Mode_Status"] >> [](pson& out){
		// Add the values and the corresponding code
		out["motor_interval"] = motor_interval;
		out["hatching_duration"] = hatching_duration;
		out["working_mode"] = working_mode;
	};
}

void loop() {
   thing.handle();
   h1 = dht1.readHumidity();
   t1 = dht1.readTemperature();
   h2 = dht2.readHumidity();
   t2 = dht2.readTemperature();
 
   inside_temp_check();
   inside_humidity_check();
  Show_default();
  rtc.refresh();
  char key = keypad.getKey (); // Create a variable named key of type char to hold the characters pressed
  if (key=='*') {// if the key variable contains
    digitalWrite(led,LOW);
  }
  if(key=='#'){
    digitalWrite(led,HIGH);
  }
  delay(100); // Delay 1 second.
}

void Show_default(){
     // set cursor to first column, first row
  lcd.setCursor(0, 0);
  lcd.print("DATE:");
  lcd.print(rtc.year());
  lcd.print('/');
  lcd.print(rtc.month());
  lcd.print('/');
  lcd.print(rtc.day());
  lcd.print(" ");
  lcd.print(daysOfTheWeek[rtc.dayOfWeek()-1]);
  lcd.print(" ");

  lcd.setCursor(0, 1);
  lcd.print("TIME:");
  lcd.print(rtc.hour());
  lcd.print(':');
  lcd.print(rtc.minute());
  lcd.print(':');
  lcd.print(rtc.second());
  lcd.setCursor(0, 2);
  // print message
  lcd.print("t: ");
  lcd.print(t1);
  lcd.print(" h: ");
  lcd.print(h1);
  lcd.print(" IN");
  lcd.setCursor(0,3);
  lcd.print("t: ");
  lcd.print(t2);
  lcd.print(" h: ");
  lcd.print(h2);
  lcd.print(" EX");

}
void inside_temp_check(){
    if(t1>prefer_temp_max && t2>40){
      fan1_controll(5);
      digitalWrite(heater, HIGH);
      fan2_speed=0;
      return;
    }
    else if(t1>prefer_temp_max && t2>35){
      fan1_controll(4);
      digitalWrite(heater, HIGH);
      fan2_speed=0;
      return;
    }
    else if(t1>prefer_temp_max && t2>30){
      fan1_controll(3);
      digitalWrite(heater, HIGH);
      fan2_speed=0;
      return;
    }
    else if(t1>prefer_temp_max && t2>25){
      fan1_controll(2);
      digitalWrite(heater, HIGH);
      fan2_speed=0;
      return;
    }
    else if(t1>prefer_temp_max && t2<25){
      fan1_controll(1);
      digitalWrite(heater, HIGH);
      fan2_speed=0;
      return;
    }
    if(t1<prefer_temp_min && t1>30){
    digitalWrite(heater, LOW);
    fan1_speed=0;
    fan2_controll(1);
    return;
  }
  else if (t1<prefer_temp_min&&t1>20){
    digitalWrite(heater, LOW);
    fan2_controll(2);
    fan1_speed=0;
    return;
  }
   else if (t1<prefer_temp_min&&t1>10){
    digitalWrite(heater, LOW);
    fan2_controll(3);
    fan1_speed=0;
    return;
  }
}
void inside_humidity_check(){
      if(h1<prefer_hum_min){
    digitalWrite(humidifier, LOW);
  }
  if(h1>prefer_hum_max){
    digitalWrite(humidifier, HIGH);
  }
}
void fan1_controll(int mode=5){
      digitalWrite(in1, HIGH);
	    digitalWrite(in2, LOW);
switch(mode){
  case 1:
    fan1_speed=65;
    analogWrite(enA, fan1_speed);   
  break;
  case 2:
    fan1_speed=95;
    analogWrite(enA, fan1_speed); 
  break;
  case 3:
    fan1_speed=135;
    analogWrite(enA, fan1_speed); 
  break;
  case 4:
    fan1_speed=195;
    analogWrite(enA, fan1_speed); 
  break;
  case 5:
    fan1_speed=255;
    analogWrite(enA, fan1_speed); 
  break;
  default:
    fan1_speed=0;
    digitalWrite(in1, LOW);
	  digitalWrite(in2, LOW);
    analogWrite(enA, fan1_speed); 

}
}
void fan2_controll(int mode=3){
  //1-->low
  //2-->medium
  //3-->high
      	digitalWrite(in3, HIGH);
	      digitalWrite(in4, LOW);
switch(mode){
  case 3:
 	analogWrite(enB, 45);
    fan2_speed=55;
    break;
  case 2:
    fan2_speed=75;
  	analogWrite(enB, fan2_speed);

    break;
  case 1:
    fan2_speed=165;
  	analogWrite(enB, fan2_speed);

    break;
  default:
  fan2_speed=0;
	digitalWrite(in1, LOW);
	digitalWrite(in2, LOW);
  analogWrite(enB,fan2_speed);
}
}
void start_mode(int w_mode){
	switch(w_mode){
		case 1:
				/*
				 [DUCK]
				 Duration: 28 Days
				 Temperature: 37.2 - 38.3
				 Humidity: 55-60 (first 25 days) & 60-65 (last three days)
				 Turninig  :at least 3-4 times a day ==> 6 times a day is good 
				 Light: No special Light - Light after hatching*/
			working_mode = 1;
			prefer_temp_min = 37;
			prefer_temp_max = 38.3;
			hatching_duration = 28;
			first_n_days = 25;
			second_n_days = 3;
			
			//motor rotate once per 4 hour
			motor_interval = 4 ;
			
		break;
		
		case 2:
				/*
				[CHICKEN]
				Duration: 21 Days
				Temperature: 37.5 - 38.0
				Humidity: 50-55 (first 18 days) & 65-75 (last three days)
				Turninig  : at least 3-4 times a day
				Light: No special Light - Light after hatching*/
			prefer_temp_min = 36.5;
			prefer_temp_max = 38.0;
			hatching_duration = 21;
			first_n_days = 18;
			second_n_days = 3;
			motor_interval = 4 ;
			working_mode = 2;
		break;
		
		case 3:
				/*
				[Quail]
				Duration: 17 Days
				Temperature: 37.5 - 38.0
				Humidity: 50-60 (first 14 days) & 65-75 (last three days)
				Turninig  : at least 3-4 times a day
				Light: No special Light - Light after hatching*/
			prefer_temp_min = 37.0;
			prefer_temp_max = 38.0;
			hatching_duration = 17;
			first_n_days = 14;
			second_n_days = 3;
			motor_interval = 4 ;
			working_mode = 3;
		break;
		
		case 4:				
				/*
				[Partridge]
				Duration: 24 Days
				Temperature: 37.5 - 38.0
				Humidity: 50-55 (first 21 days) & 65-70 (last three days)
				Turninig  : at least 3-4 times a day
				Light: No special Light - Light after hatching*/
			prefer_temp_min = 37.0;
			prefer_temp_max = 38.0;
			hatching_duration = 24;
			first_n_days = 21;
			second_n_days = 3;
			motor_interval = 4 ;
			working_mode = 4;
		break;
		
		case 5:
				/*
				[Pigeon]
				Duration: 18 Days
				Temperature: 37.5 - 38.0
				Humidity: 50-55 (first 15 days) & 65-70 (last three days)
				Turninig  : at least 3-4 times a day
				Light: No special Light - Light after hatching	*/
			prefer_temp_min = 37.0;
			prefer_temp_max = 38.0;
			hatching_duration = 18;
			first_n_days = 15;
			second_n_days = 3;
			motor_interval = 4 ;
			working_mode = 5;
		break;
		case 6:
				/*
				[Pheasent]
				Duration: 24 Days
				Temperature: 37.5 - 38.0
				Humidity: 50-55 (first 21 days) & 65-70 (last three days)
				Turninig  : at least 3-4 times a day
				Light: No special Light - Light after hatching*/
			prefer_temp_min = 37.0;
			prefer_temp_max = 38.0;
			hatching_duration = 24;
			first_n_days = 21;
			second_n_days = 3;
			motor_interval = 4 ;
			working_mode = 6;

		break;
		default:
		//CUSTOM USER DEFINED MODE
		working_mode = 0;
		break;
	}
	
}

void resume_mode(int mode){
	
}

