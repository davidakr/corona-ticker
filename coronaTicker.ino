#include "LedControl.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// EasyESP or NodeMCU Pin D8 to DIN, D7 to Clk, D6 to LOAD, no.of devices is 1
LedControl lc=LedControl(D8,D7,D6,1);
ESP8266WiFiMulti WiFiMulti;

const char* ssid     = "ssid";
const char* password = "password";
const char* host = "worldometers.info";

void setup()
{
  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ssid", "password");
  
  lc.shutdown(0,false);   // Enable display
  lc.setIntensity(0,15);  // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register
}

void loop()
{
 // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://www.worldometers.info/coronavirus/country/germany/")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          // parse for currently infected patients
          int back = payload.indexOf("Currently Infected Patients");
          int front = payload.indexOf("<div class=\"number-table-main\">", back - 80 );
          String result = payload.substring(front + 31, back - 37);
          result.replace(",", "");
          setDisplay(result.toInt());
          Serial.printf("Currently Infected Patientse: %d\n", result);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 15min before next round...");
  delay(900000);
}
void setDisplay(int number)
{
  if(number != 0) {
    lc.clearDisplay(0);
    
    int ones             = (number%10);
    int tens             = ((number/10)%10);
    int hundreds         = ((number/100)%10);
    int thousands        = ((number/1000)%10);
    int tenThousands     = ((number/10000)%10);
    int hundredThousands = ((number/100000)%10);
    int millions         = ((number/1000000)%10);
    int tenMillions      = ((number/10000000)%10);
    
    lc.setDigit(0,7,(byte)tenMillions,false);
    lc.setDigit(0,6,(byte)millions,false);
    lc.setDigit(0,5,(byte)hundredThousands,false);
    lc.setDigit(0,4,(byte)tenThousands,false); 
    lc.setDigit(0,3,(byte)thousands,false); 
    lc.setDigit(0,2,(byte)hundreds,false);
    lc.setDigit(0,1,(byte)tens,false); 
    lc.setDigit(0,0,(byte)ones,false);   
  }
}
