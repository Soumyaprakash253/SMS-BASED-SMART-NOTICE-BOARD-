#include "Arduino.h"
#include "uRTCLib.h"  // RTC library
#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

// Configuration for DMD and RTC
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// Interrupt handler for TimerOne-driven DMD refresh scanning
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

// Initialize RTC
uRTCLib rtc(0x68);

String displayMessage = "";  // The message to display
int messageDisplayCount = 0;  // Counter for the number of times the message has been displayed
int maxDisplayCount = 0;  // Maximum number of times to display the message
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup() {
  // Initialize TimerOne and DMD
  Timer1.initialize(5000);
  Timer1.attachInterrupt(ScanDMD);
  dmd.clearScreen(true);
   dmd.selectFont(SystemFont5x7);
  dmd.drawString(2, 4, "SETTING UP", strlen("Setting up"), GRAPHICS_NORMAL);
   
  
  
  Serial.begin(9600);
  delay(20000);
  Serial.println("AT");
  delay(1000);
  Serial.println("AT+CMGD=1,4");  // Configure TEXT mode
  delay(1000);
  Serial.println("AT+CNMI=2,2,0,0,0");  // Handle new SMS messages
  delay(1000);
  

  // Begin serial communication for GSM module and Arduino IDE
  

  // Initialize RTC
  
  URTCLIB_WIRE.begin();
  //rtc.set(30, 6, 17, 4, 8, 5, 24);

  // Send initialization commands to GSM module
  
  
  
}

void loop() {
  rtc.refresh();  // Refresh RTC data

  int currentSecond = rtc.second();
  int currentMinute = rtc.minute();
  int currentHour = rtc.hour();
  int currentDate=rtc.day();
  int currentMonth=rtc.month();
  int currentYear=rtc.year();
  String currentday=daysOfTheWeek[rtc.dayOfWeek()-1];
  int currentTemperature = rtc.temp();

 

  // Process incoming GSM messages
  if (Serial.available()) {
    if (Serial.find("+CMT:")) {  // Check if a message is received
      Serial.readStringUntil('"');  // Skip sender's number
      Serial.readStringUntil('"');  // Skip another "
      Serial.readStringUntil('"');  // Skip timestamp
      Serial.readStringUntil('\n');
      String receivedMessage = Serial.readStringUntil('\r');  // Read message content

      if (receivedMessage.startsWith("***##")) {
        // Extract the number of times the message should be displayed
        int separatorIndex = receivedMessage.indexOf(" ", 6);
        if (separatorIndex != -1) {
          String displayCountStr = receivedMessage.substring(6, separatorIndex);
          maxDisplayCount = displayCountStr.toInt();  // Set the maximum display count
          displayMessage = receivedMessage.substring(separatorIndex + 1);  // Message content
          messageDisplayCount = 0;  // Reset the display count
        }
      }
    }
  }

  // Display the message if the current second is 10 or 40 and if it hasn't reached max count
  if ((currentSecond == 10 ) && messageDisplayCount < maxDisplayCount) {

    displayMsg(displayMessage);
    messageDisplayCount++;  // Increment the display count
  } else if (currentSecond == 10) {
    // If the message has been displayed enough times, display "WELCOME"
    
    dmd.clearScreen(true);
    const char *greeting;
    if (currentHour >= 0 && currentHour < 12) {
      greeting = "Good Morning";
    } else if (currentHour >= 12 && currentHour < 18) {
     greeting = "Good Afternoon";
    } else {
      greeting = "Good Evening";
    }
    
     displayMsg(greeting);
     
      
  } else {
    // Otherwise, display the current time
    displayTime(currentHour, currentMinute, currentSecond, currentTemperature);
    if (currentSecond == 40){
     displayDay(currentDate,currentMonth,currentYear,currentday);
      }
    
  }
}

// Function to display a message as a scrolling marquee
void displayMsg(String str) {
  int slen = str.length() + 1;
  char msgBuffer[250];
  str.toCharArray(msgBuffer, slen);
  dmd.clearScreen(true);

  dmd.selectFont(Arial_Black_16);  // Choose font for display

  // Draw scrolling marquee with the message
  dmd.drawMarquee(msgBuffer, slen, (32 * DISPLAYS_ACROSS), 1);

  unsigned long startTime = millis();
  boolean done = false;

  while (!done) {
    if ((startTime + 40) < millis()) {
      done = dmd.stepMarquee(-1, 0);
      startTime = millis();
    }
  }
}

// Function to display the current time
void displayTime(int hour, int minute, int second,int temperature) {
  // Format the time as HH:MM:SS
   
 char hr[3];
  char mn[3];
  char sc[3];
  char t[3];// Buffer to hold formatted time
  sprintf(hr, "%02d:", hour);
  sprintf(mn,"%02d",minute);
  sprintf(sc,"%02d",second);
  
  
  
  sprintf(t,"%02d",temperature);
  

    // Clear the display
  dmd.selectFont(Arial_Black_16);  // Choose font
  
  dmd.drawString(0, 1, hr, strlen(hr), GRAPHICS_NORMAL);
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(23, 0, mn, strlen(mn), GRAPHICS_NORMAL);
  dmd.drawString(23, 8, sc, strlen(sc), GRAPHICS_NORMAL);
  dmd.drawString(37, 0, "TEMP", 4, GRAPHICS_NORMAL);
  dmd.drawString(37, 8, t, 2, GRAPHICS_NORMAL);
  dmd.drawString(49, 8, "'C", 2, GRAPHICS_NORMAL);
  
}
void displayDay(int day,int month,int year,String dayOfWeek){
  char d[3];
  char m[3];
  char y[4];
  

  sprintf(d,"%02d",day);
  sprintf(m,"%02d",month);
  sprintf(y,"%02d",year);
  
  String Str = String(day)+"/"+String(month)+"/"+String(year)+"-"+dayOfWeek;
  displayMsg(Str);
  
  }
