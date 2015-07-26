#include <EventManager.h>
#include <Time.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
EventManager CdEventManager;

#define MAX_BUF 32
char buff[32];

#define DS3232_I2C_ADDRESS 0x68

int lcd_key     = 0;
int adc_key_in  = 0;

#define btnNONE   0
#define btnRIGHT  1
#define btnUP     2
#define btnDOWN   3
#define btnLEFT   4
#define btnSELECT 5
#define NUM_KEYS 6

#define KEY_PRESS_DURATION 1
#define KEY_PRESS_TIME (KEY_PRESS_DURATION*100)

unsigned long lastToggledKeyPressDuration[NUM_KEYS];
unsigned long lastKeyEvent;

#define W_BOOT 0
#define W_CLOCK 1
#define W_MENU 1
#define W_BRIGHTNESS 3
#define W_STOPPER 4
#define W_ALARM 5
#define W_SET_CLOCK 6
#define W_SET_CLOCK_YEAR 7
#define W_SET_CLOCK_MONTH 8
#define W_SET_CLOCK_DAY 9
#define W_SET_CLOCK_HOUR 10
#define W_SET_CLOCK_MINUTE 11
#define W_SET_CLOCK_SECOND 12
#define W_SET_ALARM 13
#define W_SET_ALARM_YEAR 14
#define W_SET_ALARM_MONTH 15
#define W_SET_ALARM_DAY 16
#define W_SET_ALARM_HOUR 17
#define W_SET_ALARM_MINUTE 18
#define W_SET_ALARM_SECOND 19

byte Window = W_CLOCK;
byte pWindow;
boolean stopper_status;
unsigned long stopper_time;
int backLight = 10;
boolean AlarmIsSet = false;
tmElements_t CurrentTime, ClockSetTime, AlarmSetTime;
boolean bls = true;

void UpdateTime() {
  RTC.write(ClockSetTime);
}

void SetAlarm() {
  tmElements_t ct;
  RTC.read(ct);
 if(AlarmSetTime.Year >= ct.Year
 && AlarmSetTime.Month >= ct.Month
 && AlarmSetTime.Day >= ct.Day
 //&& AlarmSetTime.Hour >= ct.Hour
 //&& AlarmSetTime.Minute > ct.Minute
 ) {
  AlarmIsSet = true; 
 }
}

int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      
 if (adc_key_in > 1000) return btnNONE; 
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;  

 return btnNONE;  // when all others fail, return this...
}

byte getTemperature() {
    byte tempMSB, tempLSB, temp;
    Wire.beginTransmission(DS3232_I2C_ADDRESS);  
    Wire.write(0x11);  // move register pointer to first temperature register
    Wire.endTransmission();
   
    Wire.requestFrom(DS3232_I2C_ADDRESS, 2); 
    tempMSB = Wire.read();  
    tempLSB = Wire.read() >> 6;
    return char(tempMSB + (0.25*tempLSB));
}

class Disp {
  public:
    void ClockSet() {
      if(Window == W_SET_CLOCK) {
        lcd.setCursor(2,0);
        lcd.print("Set time/date");
      } else {
        if(ClockSetTime.Year < 1 ) {
          RTC.read(ClockSetTime);  
        }
  
        lcd.setCursor(3,0);
        lcd.print(ClockSetTime.Year+1970);
        
        if(Window == W_SET_CLOCK_YEAR) {
         lcd.print("_");
        } else {
           lcd.print(".");
        }
       
        if(ClockSetTime.Month < 10) {
         lcd.print("0");
        }
        lcd.print(ClockSetTime.Month);
        
        if(Window == W_SET_CLOCK_MONTH) {
         lcd.print("_");
        } else {
           lcd.print(".");
        }
        
        if(ClockSetTime.Day < 10) {
         lcd.print("0");
        }
        lcd.print(ClockSetTime.Day);
        
        if(Window == W_SET_CLOCK_DAY) {
          lcd.print("_");
        } else {
           lcd.print(".");
        }
       
        lcd.setCursor(4,1);
        if(ClockSetTime.Hour < 10) {
         lcd.print("0");
        }
        lcd.print(ClockSetTime.Hour);
        
        if(Window == W_SET_CLOCK_HOUR) {
           lcd.print("_");
        } else {
           lcd.print(":");
        }

        if(ClockSetTime.Minute < 10) {
         lcd.print("0");
        }
        lcd.print(ClockSetTime.Minute);
        
        if(Window == W_SET_CLOCK_MINUTE) {
            lcd.print("_");
        } else {
           lcd.print(":");
        }
        
        if(ClockSetTime.Second < 10) {
         lcd.print("0");
        }
        lcd.print(ClockSetTime.Second);
        
        if(Window == W_SET_CLOCK_SECOND) {
          //lcd.blink();
          lcd.print("_");
        }      
      }
    }
 
    void Stopper() {
      lcd.setCursor(4,0);
      lcd.print("STOPPER");
    if(stopper_status == false) {
        stopper_time = millis();
    } else {
     long secs = (millis() - stopper_time) / 1000;
     long mins = secs/60;
     long hrs = mins/3600;
     
     if(hrs>=0) {
      mins -= hrs*60;
     } 
     
     if(mins>=0) {
      secs -= mins*60;
     } 

      lcd.setCursor(3,1);
      if(hrs < 10) {
        lcd.print("0");
      }
      lcd.print(hrs);
      lcd.print(":");

      if(mins < 10) {
        lcd.print("0");
      }
      lcd.print(mins);
      lcd.print(":");
      
      if(secs < 10) {
        lcd.print("0");
      }
      lcd.print(secs);
      }
    }
    
    void Clock() {
      tmElements_t tm;
      RTC.read(tm);
   
      snprintf(buff, 32, "%d.%02d.%02d.", tm.Year+1970, tm.Month, tm.Day);
      lcd.setCursor(3,0);
      lcd.print(buff);
      snprintf(buff, 32, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
      lcd.setCursor(2,1);
      lcd.print(buff);
        
      byte temp;
      temp = getTemperature(),
      snprintf(buff, 32, "%dC", temp);
      lcd.setCursor(13,1);
      lcd.print(buff);
    }

    void AlarmSet() {
      if(Window == W_SET_ALARM) {
        lcd.setCursor(2,0);
        lcd.print("Set ALARM");
      } else {
        if(AlarmSetTime.Year < 1 ) {
          RTC.read(AlarmSetTime);  
          AlarmSetTime.Second = 0;
        }

        lcd.setCursor(3,0);
        lcd.print(AlarmSetTime.Year+1970);
        
        if(Window == W_SET_ALARM_YEAR) {
           lcd.print("_");
        } else {
           lcd.print(".");
        }
       
        if(AlarmSetTime.Month < 10) {
         lcd.print("0");
        }
        lcd.print(AlarmSetTime.Month);
        
        if(Window == W_SET_ALARM_MONTH) {
         // lcd.blink();
         lcd.print("_");
        } else {
           lcd.print(".");
        }

        
        if(AlarmSetTime.Day < 10) {
         lcd.print("0");
        }
        lcd.print(AlarmSetTime.Day);
        
        if(Window == W_SET_ALARM_DAY) {
        //  lcd.blink();
        lcd.print("_");
        } else {
           lcd.print(".");
        }
       
        lcd.setCursor(4,1);
        if(AlarmSetTime.Hour < 10) {
         lcd.print("0");
        }
        lcd.print(AlarmSetTime.Hour);
        
        if(Window == W_SET_ALARM_HOUR) {
         // lcd.blink();
         lcd.print("_");
        } else {
           lcd.print(":");
        }

        if(AlarmSetTime.Minute < 10) {
         lcd.print("0");
        }
        lcd.print(AlarmSetTime.Minute);
        
        if(Window == W_SET_ALARM_MINUTE) {
          lcd.print("_");
        } else {
           lcd.print(":");
        }
        
        if(AlarmSetTime.Second < 10) {
         lcd.print("0");
        }
        lcd.print(AlarmSetTime.Second);
        
        if(Window == W_SET_ALARM_SECOND) {
          //lcd.blink();
          lcd.print("_");
        }      
      }
    }

    void Alarm() {
        byte row = random(0,11);
        byte col = random(0,2);
       
        lcd.setCursor(row,col);
        lcd.print("ALARM!");

        pinMode(backLight, OUTPUT);
        if(bls) {
            digitalWrite(backLight, LOW);  
        } else {
            digitalWrite(backLight, HIGH);  
        }
        bls = !bls;
        delay(100);
    }
};
Disp myDisp;

void WindowListener( int event, int param )
{
  if(pWindow!=Window) {
    lcd.clear();
  }
  
  if(Window == W_CLOCK) {
    myDisp.Clock();
    if(AlarmIsSet) {
      lcd.setCursor(0,0);
      lcd.print("A");
    }
    if(stopper_status) {
      lcd.setCursor(15,0);
      lcd.print("S");
    }
    
  } else if(Window == W_MENU) {
    lcd.setCursor(0,1);
    lcd.print("MENUU!");
  } else if(Window == W_BRIGHTNESS) {
    lcd.setCursor(1,0);
    lcd.print("Set backlight");

    lcd.setCursor(12,1);
    lcd.print("ON");

    lcd.setCursor(2,1);
    lcd.print("OFF");
    
  } else if(Window >= W_SET_CLOCK && Window <= W_SET_CLOCK_SECOND) {
    myDisp.ClockSet();
  } else if(Window >= W_SET_ALARM && Window <= W_SET_ALARM_SECOND) {
    myDisp.AlarmSet();
  } else if(Window == W_STOPPER) {
    myDisp.Stopper();
  } else if(Window == W_SET_ALARM) {
    myDisp.AlarmSet();
  } else if(Window == W_ALARM) {
    myDisp.Alarm();
  }
      
  else {
    lcd.setCursor(0,0);
    lcd.print(Window);
    lcd.setCursor(0,1);
    lcd.print("o.O");
  }
  pWindow = Window;
}

void KeyPressListener( int event, int param ){
  if(param == btnNONE) {
    lastToggledKeyPressDuration[btnRIGHT] = 0;
    lastToggledKeyPressDuration[btnUP] = 0;
    lastToggledKeyPressDuration[btnDOWN] = 0;
    lastToggledKeyPressDuration[btnLEFT] = 0;
    lastToggledKeyPressDuration[btnSELECT] = 0;
  } else  {
    lastToggledKeyPressDuration[param] += 1;//millis();
    lastKeyEvent = millis();
  }
}

void NavigateListener( int event, int param ) { //navigation logic
    for(byte i = 1;i < NUM_KEYS;i++) {
      if(lastToggledKeyPressDuration[i] > KEY_PRESS_DURATION) {
          lastToggledKeyPressDuration[i]=0;
             if(Window == W_MENU)//menu
             {
              if(i == btnSELECT) {
                Window = W_CLOCK;
              } else if(i == btnUP) {
                Window = W_BRIGHTNESS;
              } else if(i == btnRIGHT) {
                Window = W_STOPPER;
              } else if(i == btnLEFT) {
                Window = W_SET_CLOCK;
              } else if(i == btnDOWN) {
                Window = W_SET_ALARM;
              }
             }
              else
            if(Window == W_ALARM)
             {
              if(i==btnSELECT) {
                AlarmIsSet = false;
                AlarmSetTime.Year = 0;
                pinMode(backLight, OUTPUT);
                digitalWrite(backLight, HIGH);  
                Window = W_CLOCK;
              }
             }
            else
            if(Window == W_BRIGHTNESS)
             {
              if(i==btnDOWN) {
                Window = W_MENU;
              }
              if(i==btnLEFT) {
                pinMode(backLight, OUTPUT);
                digitalWrite(backLight, LOW);
              }
              if(i==btnRIGHT) {
                pinMode(backLight, OUTPUT);
                digitalWrite(backLight, HIGH);
              }
             }
             else
             if(Window == W_STOPPER)
             {
              if(i == btnLEFT) {//exit
                Window = W_MENU;
              } else if(i==btnRIGHT) {//start
                stopper_status = true;
              } else if(i==btnDOWN) {//pause

              } else if(i==btnUP) {//reset
                stopper_status = false;
              }
             }
             else
             if(Window == W_SET_CLOCK)
             {
              if(i==btnRIGHT) {
                Window = W_MENU;
              }
              if(i==btnSELECT) {
                Window = W_SET_CLOCK_YEAR;
              }
             }
             
             else
              if(Window == W_SET_CLOCK_YEAR)
             {
              if(i==btnSELECT) {
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_MONTH;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_SECOND;
              }
              if(i==btnUP) {
                ClockSetTime.Year += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Year -= 1;
              }
             }
             else
             if(Window == W_SET_CLOCK_MONTH)
             {
              if(i==btnSELECT) {
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_DAY;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_YEAR;
              }
               if(i==btnUP) {
                ClockSetTime.Month += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Month -= 1;
              }
             if(ClockSetTime.Month > 12) {
                ClockSetTime.Month = 1;
              }
              if(ClockSetTime.Month == 0) {
                ClockSetTime.Month = 12;
              }
             }
             else
             if(Window == W_SET_CLOCK_DAY)
             {
              if(i==btnSELECT) {
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_HOUR;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_MONTH;
              }
               if(i==btnUP) {
                ClockSetTime.Day += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Day -= 1;
              }

             if(ClockSetTime.Day > 31) {
                ClockSetTime.Day = 1;
              }
              if(ClockSetTime.Day == 0) {
                ClockSetTime.Day = 31;
              }
             }
              else
             if(Window == W_SET_CLOCK_HOUR)
             {
              if(i==btnSELECT) {
                UpdateTime();
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_MINUTE;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_DAY;
              }
               if(i==btnUP) {
                ClockSetTime.Hour += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Hour -= 1;
              }
              if(ClockSetTime.Hour == 255) {
                ClockSetTime.Hour = 23;
              }
              if(ClockSetTime.Hour > 23) {
                ClockSetTime.Hour = 0;
              }
             }
              else
             if(Window == W_SET_CLOCK_MINUTE)
             {
              if(i==btnSELECT) {
                UpdateTime();
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_SECOND;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_HOUR;
              }
               if(i==btnUP) {
                ClockSetTime.Minute += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Minute -= 1;
              }
              if(ClockSetTime.Minute == 255) {
                ClockSetTime.Minute = 59;
              }
              if(ClockSetTime.Minute > 59) {
                ClockSetTime.Minute = 0;
              }
             }
              else
             if(Window == W_SET_CLOCK_SECOND)
             {
              if(i==btnSELECT) {
                UpdateTime();
                Window = W_SET_CLOCK;
              }
              if(i==btnRIGHT) {
                Window = W_SET_CLOCK_YEAR;
              }
              if(i==btnLEFT) {
                Window = W_SET_CLOCK_MINUTE;
              }
               if(i==btnUP) {
                ClockSetTime.Second += 1;
              }
              if(i==btnDOWN) {
                ClockSetTime.Second -= 1;
              }
              if(ClockSetTime.Second == 255) {
                ClockSetTime.Second = 59;
              }
              
              if(ClockSetTime.Second > 59) {
                ClockSetTime.Second = 0;
              }
             }
            /////////////////////////////////////////////ALARM
            else
              if(Window == W_SET_ALARM)
             {
              if(i==btnUP) {
                Window = W_MENU;
              }
              if(i==btnSELECT) {
                Window = W_SET_ALARM_YEAR;
              }
             }
              else
             if(Window == W_SET_ALARM_YEAR)
             {
              if(i==btnSELECT) {
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_MONTH;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_SECOND;
              }
              if(i==btnUP) {
                AlarmSetTime.Year += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Year -= 1;
              }
             }
             else
             if(Window == W_SET_ALARM_MONTH)
             {
              if(i==btnSELECT) {
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_DAY;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_YEAR;
              }
               if(i==btnUP) {
                AlarmSetTime.Month += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Month -= 1;
              }
             if(AlarmSetTime.Month > 12) {
                AlarmSetTime.Month = 1;
              }
              if(AlarmSetTime.Month == 0) {
                AlarmSetTime.Month = 12;
              }
             }
            else
             if(Window == W_SET_ALARM_DAY)
             {
              if(i==btnSELECT) {
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_HOUR;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_MONTH;
              }
               if(i==btnUP) {
                AlarmSetTime.Day += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Day -= 1;
              }
              else
               if(AlarmSetTime.Day > 31) {
                  AlarmSetTime.Day = 1;
                }
                if(AlarmSetTime.Day == 0) {
                  AlarmSetTime.Day = 31;
                }
             }
              else
             if(Window == W_SET_ALARM_HOUR)
             {
              if(i==btnSELECT) {
                SetAlarm();
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_MINUTE;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_DAY;
              }
               if(i==btnUP) {
                AlarmSetTime.Hour += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Hour -= 1;
              }
              if(AlarmSetTime.Hour == 255) {
                AlarmSetTime.Hour = 23;
              }
              
              if(AlarmSetTime.Hour > 23) {
                AlarmSetTime.Hour = 0;
              }
             }
            else
             if(Window == W_SET_ALARM_MINUTE)
             {
              if(i==btnSELECT) {
                SetAlarm();
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_SECOND;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_HOUR;
              }
               if(i==btnUP) {
                AlarmSetTime.Minute += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Minute -= 1;
              }
              if(AlarmSetTime.Minute == 255) {
                AlarmSetTime.Minute = 59;
              }
              if(AlarmSetTime.Minute > 59) {
                AlarmSetTime.Minute = 0;
              }
             }
            else
             if(Window == W_SET_ALARM_SECOND)
             {
              if(i==btnSELECT) {
                SetAlarm();
                Window = W_SET_ALARM;
              }
              if(i==btnRIGHT) {
                Window = W_SET_ALARM_YEAR;
              }
              if(i==btnLEFT) {
                Window = W_SET_ALARM_MINUTE;
              }
               if(i==btnUP) {
                AlarmSetTime.Second += 1;
              }
              if(i==btnDOWN) {
                AlarmSetTime.Second -= 1;
              }
              if(AlarmSetTime.Second == 255) {
                AlarmSetTime.Second = 59;
              }
              if(AlarmSetTime.Second > 59) {
                AlarmSetTime.Second = 0;
              }
           }
        }
    }
}

void AlarmListener( int event, int param ) {
  if(AlarmIsSet 
	&& CurrentTime.Year == AlarmSetTime.Year
	&& CurrentTime.Month == AlarmSetTime.Month
	&& CurrentTime.Day == AlarmSetTime.Day
	&& CurrentTime.Hour == AlarmSetTime.Hour
	&& CurrentTime.Minute == AlarmSetTime.Minute 
	&& CurrentTime.Second == AlarmSetTime.Second ) {
    Window = W_ALARM;
  }
}

void setup() {
  lcd.begin(16,2); 
  lcd.setCursor(1,0);
  lcd.print("Booting System");

  Wire.begin();  

  CdEventManager.addListener( EventManager::kEventPaint, WindowListener );
  lcd.setCursor(5,1);
  lcd.print(".");
  CdEventManager.addListener( EventManager::kEventMenu0, NavigateListener );
  
  lcd.print(".");
  CdEventManager.addListener( EventManager::kEventKeyPress, KeyPressListener );

  CdEventManager.addListener( EventManager::kEventTimer1, AlarmListener );

  lcd.print(".");
  
  delay(KEY_PRESS_TIME);

  lcd.print(".");

   delay(KEY_PRESS_TIME);

  lcd.print("done");

  delay(KEY_PRESS_TIME/2);
}

void loop() {
  CdEventManager.processEvent();
  addEvents();
}

void addEvents() {
  addAlarmEvent();
  addWindowEvent();
  addKeyEvent();
  addNavigateEvent();
}

void addKeyEvent() {
   if ( millis() - lastKeyEvent > KEY_PRESS_TIME  )  {
        int btnsSts = read_LCD_buttons();
      CdEventManager.queueEvent( EventManager::kEventKeyPress,  btnsSts);
   }
}

void addNavigateEvent() {
  if((millis()%100) == 0) {
    CdEventManager.queueEvent( EventManager::kEventMenu0, 0 );
  }
}

void addWindowEvent() {
  if((millis()%250) == 0) {
    CdEventManager.queueEvent( EventManager::kEventPaint, 0 );
  }
}

void addAlarmEvent() {
  if((millis()%500) == 0) {
    RTC.read(CurrentTime);
    CdEventManager.queueEvent( EventManager::kEventTimer1, 0 );
  }
}

