#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "SECRETS.h"

// Citations:
// https://techtutorialsx.com/2016/10/22/esp8266-webserver-getting-query-parameters/
// https://siytek.com/how-to-set-up-esp8266-as-a-wifi-access-point-ap-mode/
// https://www.luisllamas.es/en/esp8266-server-parameters/
// Example URL: http://website.com/path?parameter1=value1&parameter2=value2

#define STATUS_TIMEOUT 30000
#define INITIALIZATION_DELAY 60000
#define TWENTY_FOUR_HOURS 86400000 /*60*60*24*1000*/

ESP8266WebServer server(80); // Create a server object listening on port 80

long midnightMillis = 0, alarmMillis = 0 /* includes midnightMillis */, now = 0, lastStim = 0, statusTimeout = STATUS_TIMEOUT, dailyOffset = 0, systemTime = 0;
int timeHour = 0, timeMinute = 0, alarmHour = 0, alarmMinute = 0, alarmLength = 300 /*in seconds*/;
bool isAlarmSet = false, isTimeSet = false;

const int led = 2,
          relay1 = 5;
          
void setup()
{
  // Setup serial port
  Serial.begin(115200);
  Serial.println();

  // Begin Access Point
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(SECRET_SSID, SECRET_PASS);

  // Initialize pins
  pinMode(led, OUTPUT); //set up LED
  digitalWrite(led, HIGH);
  pinMode(relay1, OUTPUT); // set up vibrator
  digitalWrite(relay1,HIGH);

  // Print access point IP
  Serial.print("IP address for network ");
  Serial.print(SECRET_SSID);
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());

  ServerInit();
  
  Serial.println("\nServer started");
}

void loop()
{

  server.handleClient();         //Handling of incoming requests
  now = millis();

  handleSystemTime();
  handleAlarm();
  handleStatus();
}

void handleSystemTime() {
  systemTime = now - midnightMillis;  
}

void handleStatus() {
  if (now - statusTimeout > 0) {
    statusTimeout += STATUS_TIMEOUT;
    Serial.print("\nCurrent System Millis: " + String(now));
    Serial.print("\nCurrent Time   Millis: " + String(systemTime));
    Serial.print("\nCurrent Alarm  Millis: " + String(alarmMillis));
    Serial.print("\nCurrent Alarm  Millis & Last Stim: " + String(alarmMillis+dailyOffset));
  }
}

void handleAlarm() {
  if ( (systemTime - (alarmMillis + dailyOffset) > 0) && now > INITIALIZATION_DELAY ) {
    dailyOffset += TWENTY_FOUR_HOURS;
    shake();
  }
}

bool shakeStatus = false,
     shakeState = false; //false = off, true = on

void shake() {
  int shakeON = 5, // in secs
      shakeOFF = 10;
  while (millis() - now < alarmLength*1000) {
    shakeOn();
    delay(shakeON * 1000);
    shakeOff();
    delay(shakeOFF * 1000);
  }
}

void shakeOn() {
  //if (!shakeState) {
    pinMode(relay1,OUTPUT);
    digitalWrite(relay1,LOW); // On
    shakeState = true;
    Serial.print("\nShake On ");
  //}
}

void shakeOff() {
  //if (shakeState) {
    pinMode(relay1,INPUT); //Let pullup bring gate up to 5V (Our HIGH is only 3.3)
    shakeState = false;
    Serial.print("\nShake Off ");
  //}
}

void ServerInit() {
  server.on("/", handleRootPath);
  server.on("/clock", handleTimeSet );
  server.on("/alarm", handleAlarmSet);
  server.on("/status", handleStatushtml);
  server.begin(); // Start the server
}

void handleStatushtml() {
  String message = "";
  message += "Current Time: "; message += String(int(floor(systemTime/(1000*60*60)))); message += ":"; message += String(int(floor(systemTime/(1000*60))) % 60);
  message += "\nAlarm   Time: "; message += String(int(floor((alarmMillis+dailyOffset)/(1000*60*60)))); message += ":"; message += String(int(floor((alarmMillis+dailyOffset)/(1000*60))) % 60);
  message += "\nCurrent System Millis: "; message += String(now);
  message += "\nCurrent Time   Millis: "; message += String(systemTime);
  message += "\nCurrent Alarm  Millis: "; message += String(alarmMillis);
  message += "\nCurrent Alarm  Millis & Last Stim: "; message += String(alarmMillis+dailyOffset);

  server.send(200, "text/plain", message);          //Returns the HTTP response
}

void handleRootPath() {
 
   server.send(200, "text/plain", "Hello world");
 
}

void handleTimeSet() {

  String message = "";

  if (server.arg("hour")== ""){     //Parameter not found
    message = "Hour Argument not found";
  }
  else{     //Parameter found
    message = "Hour Argument = ";
    message +=server.arg("hour");
    timeHour= server.arg("hour").toInt();     //Gets the value of the query parameter
    Serial.println("Time: Hour set - " + String(timeHour));
  }

  if (server.arg("minute")== ""){     //Parameter not found
    message = "Minute Argument not found";
  }
  else{     //Parameter found
    message += "\nMinute Argument = ";
    message +=server.arg("minute");
    timeMinute= server.arg("minute").toInt();     //Gets the value of the query parameter
    Serial.println("Time: Minute set - " + String(timeMinute));
  }

  //    Hours to mins    extra mins    millis/min
  // (((24 - 13) * 60) + (60 - 35)) * (60 * 1000)
  while ( timeHour > 23 ) { timeHour -= 24; Serial.println("Time: Hour set - " + String(timeHour)); }
  midnightMillis = (((23 - timeHour) * 60) + (60 - timeMinute)) * (60 * 1000);
  Serial.println("Time: MidnightMillis set - " + String(midnightMillis));
  
  message += "\nmidnightMillis = " + String(midnightMillis);
  message += "\nisTimeSet = true";
  server.send(200, "text/plain", message);          //Returns the HTTP response
  isTimeSet = true;
}

void handleAlarmSet() {

  String message = "";

  if (server.arg("hour")== ""){     //Parameter not found
    message += "\nHour Argument not found";
  }
  else{     //Parameter found
    message = "Hour Argument = ";
    message +=server.arg("hour");
    alarmHour= server.arg("hour").toInt();     //Gets the value of the query parameter
    Serial.println("Alarm: Hour set - " + String(alarmHour));
  }

  if (server.arg("minute")== ""){     //Parameter not found
    message += "\nMinute Argument not found";
  }
  else{     //Parameter found
    message += "\nMinute Argument = ";
    message +=server.arg("minute");
    alarmMinute= server.arg("minute").toInt();     //Gets the value of the query parameter
    Serial.println("Alarm: Minute set - " + String(alarmMinute));
  }
  if (server.arg("length")== ""){     //Parameter not found
    message += "\nLength Argument not found.  Old value retained.";
  }
  else{     //Parameter found
    message += "\nAlarm Length Argument = ";
    message +=server.arg("length");
    alarmLength= server.arg("length").toInt();     //Gets the value of the query parameter
    Serial.println("Alarm: Length set - " + String(alarmLength));
  }
  if (server.arg("repetition")== ""){     //Parameter not found
    message += "\nRepetition Argument not found.  Old value retained.";
  }
  else{     //Parameter found
    message += "\nAlarm Repetition Argument = ";
    message +=server.arg("repetition");
    String alarmRep = server.arg("repetition"); //Gets the value of the query parameter
    Serial.println("Alarm: repetition set - " + alarmRep + " - DUMMY");
  }
  
  if (isTimeSet) {
    isAlarmSet = true;
    alarmMillis = (((alarmMinute * 60 * 1000) + (alarmHour * 60 * 60 * 1000)));
    if ( alarmMillis - TWENTY_FOUR_HOURS > systemTime ) { alarmMillis -= TWENTY_FOUR_HOURS; }
    //if (alarmHour > timeHour || (alarmHour == timeHour && alarmMinute > timeMinute)) { alarmMillis -= midnightMillis; } // Current day, subtract midnight millis because it's in the future for both
    //else { alarmMillis += midnightMillis; } // add midnight millis if the alarm time is the day after.
    message += "\nAlarm Millis = " + String(alarmMillis);
    Serial.println("Alarm: Millis set - " + String(alarmMillis));
  }
  else {
    message += "Error.  Time is not set.";
  }
  
  server.send(200, "text/plain", message);          //Returns the HTTP response

}
