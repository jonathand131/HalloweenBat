#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "private_wifi_settings.h"
#include "private_blynk_settings.h"

// #define BLYNK_DEBUG // Optional, this enables lots of prints
// #define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>


#define BLYNK_VIRTUAL_MODE V0
#define BLYNK_VIRTUAL_BRIGHTNESS V1

typedef enum __OPERATING_MODE {
  MODE_OFF=1,
  MODE_FUZZY,
  MODE_MANUAL,
  MODE_BLINK,
  MODE_BLINK_LR,
  MODE_EOL,
} operating_mode;

const char * modeToString(operating_mode om) {
  switch(om) {
    case MODE_OFF:
      return "Off";
    case MODE_FUZZY:
      return "Fuzzy";
    case MODE_MANUAL:
      return "Manual";
    case MODE_BLINK:
      return "Blink";
    case MODE_BLINK_LR:
      return "Blink LR";
  }
};

/* Hardware related constants */
const int LED_1 = 14;
const int LED_2 = 13;
const int LED_OFF = 0;
const int LED_ON = 1;

/* Prototypes */
void setupWiFi();
void populateModeMenu();

/* Globals */
int last_val;
operating_mode mode;
int max_brightness;
bool blink_state;

/* Blynk handlers */
BLYNK_CONNECTED() {
  Serial.println("Blynk connected, populate mode menu items...");
  populateModeMenu();
  Blynk.syncAll();
}

BLYNK_READ(BLYNK_VIRTUAL_MODE) {
  Blynk.virtualWrite(BLYNK_VIRTUAL_MODE, mode);
}

BLYNK_WRITE(BLYNK_VIRTUAL_MODE) {
  mode = (operating_mode)param.asInt();
  Serial.printf("New mode: %d\n", mode);
  if(mode == MODE_OFF) {
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  } else {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  }
}

BLYNK_READ(BLYNK_VIRTUAL_BRIGHTNESS) {
  Blynk.virtualWrite(BLYNK_VIRTUAL_BRIGHTNESS, max_brightness);
}

BLYNK_WRITE(BLYNK_VIRTUAL_BRIGHTNESS) {
  max_brightness = param.asInt();
  Serial.printf("New brightness: %d\n", max_brightness);
}


void setup() {
  Serial.begin(74880);
  Serial.println("Booting");

  // Init globals
  Serial.printf("PWMRANGE: %d\n", PWMRANGE);
  max_brightness = PWMRANGE;
  last_val = max_brightness;
  mode = MODE_FUZZY;
  blink_state = false;

  // Init LEDs
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_1, LED_ON);
  digitalWrite(LED_2, LED_OFF);

  // Initialize random seed
  int seed = analogRead(A0);
  Serial.printf("Seed: %d\n", seed);
  randomSeed(seed * 32);

  // Configure WiFi, OTA and connectivity services
  setupWiFi();
  Blynk.config(BLYNK_AUTH);
}

void loop() {
  ArduinoOTA.handle();
  Blynk.run();

  switch(mode) {
    case MODE_OFF:
    {
      analogWrite(LED_1, 0);
      analogWrite(LED_2, 0);
      delay(500);
      break;
    }

    case MODE_FUZZY:
    {
      last_val += random((-1 * max_brightness)/2, max_brightness/2);
      if(last_val < 0) {
        last_val = 0;
      } else if(last_val > max_brightness) {
        last_val = max_brightness;
      }
      analogWrite(LED_1, last_val);
      analogWrite(LED_2, last_val);
      delay(100);
      break;
    }

    case MODE_MANUAL:
    {
      last_val = max_brightness;
      analogWrite(LED_1, last_val);
      analogWrite(LED_2, last_val);
      delay(100);
      break;
    }

    case MODE_BLINK_LR:
    {
      last_val = max_brightness;
      if(blink_state) {
        analogWrite(LED_1, last_val);
        analogWrite(LED_2, 0);
      } else {
        analogWrite(LED_1, 0);
        analogWrite(LED_2, last_val);
      }
      blink_state = !blink_state;
      delay(500);
      break;
    }

    case MODE_BLINK:
    {
      last_val = max_brightness;
      if(blink_state) {
        analogWrite(LED_1, last_val);
        analogWrite(LED_2, last_val);
      } else {
        analogWrite(LED_1, 0);
        analogWrite(LED_2, 0);
      }
      blink_state = !blink_state;
      delay(500);
      break;
    }

    default:
    {
      Serial.printf("Bad mode: %d\n", mode);
      delay(500);
      break;
    }
  }
}

void setupWiFi() {
  WiFi.printDiag(Serial);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("nodemcu1");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void populateModeMenu() {
  BlynkParamAllocated items(128); // list length, in bytes
  for(operating_mode om = (operating_mode)1; om < MODE_EOL; om = (operating_mode)(om + 1)) {
    items.add(modeToString(om));
  }
  Blynk.setProperty(BLYNK_VIRTUAL_MODE, "labels", items);
}
