#include "LedControl.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DNSServer.h>            
#include <ESP8266WebServer.h>     
#include <WiFiManager.h>
#include <ArduinoJson.h>          

// EasyESP or NodeMCU Pin D8 to DIN, D7 to Clk, D6 to LOAD, no.of devices is 1
LedControl lc = LedControl(D8,D7,D6,1);
ESP8266WiFiMulti WiFiMulti;
WiFiManager wifiManager;

//const uint8_t fingerprint[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool world = false;
String country = "germany";

void setup()
{
  Serial.begin(115200);
  
  wifiManager.autoConnect("CoronaTickerWifiSetup");
  Serial.println("[Setup] Successfull connected!");
  Serial.print("[Setup] IP Address is: ");
  Serial.println(WiFi.localIP());

  lc.shutdown(0,false);   // Enable display
  lc.setIntensity(0,2);   // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register
  delay(1000);
}

void loop()
{ 
  int infected = 0;
  while (infected == 0)
  {
    if (world) {
      infected = getCurrentInfectedWorld();
    } else {
      infected = getCurrentInfectedInCountry(country);
    }
  }

  setDisplay(infected);
  String region = world ? "world" : country;
  Serial.println("[RESULT] Active cases COVID-19 " + region + ": " + String(infected));
  Serial.println("[TIMER] 1 hour until  next request");
  delay(3600000);
}

int getCurrentInfectedWorld(){
  return getCurrentInfectedInCountry("");
}

int getCurrentInfectedInCountry(String country){
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  //client->setFingerprint(fingerprint);
  client->setInsecure();

  HTTPClient https;

  String url;
  if (country.isEmpty()){
    url = "https://corona.lmao.ninja/v2/all?yesterday";
  } else {
    url = "https://corona.lmao.ninja/v2/countries/" + country + "?yesterday&strict&query%20";
  }

  if (https.begin(*client, url)) {  // HTTPS

    Serial.print("[HTTPS] GET Request\n");
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
        DynamicJsonDocument doc(1024);      
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();

        String result = obj["active"];
        return result.toInt();
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      return 0;
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    return 0;
  }
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
