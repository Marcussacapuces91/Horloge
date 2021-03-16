
///> @see https://github.com/arduino-libraries/NTPClient
#include <NTPClient.h>

///> @see https://arduino-esp8266.readthedocs.io/en/latest/index.html
#include <ESP8266WiFi.h>

#include <WiFiUdp.h>

///> @see https://github.com/adafruit/Adafruit_LED_Backpack
#include "Adafruit_LEDBackpack.h"

#include "secrets.h"

template<
  const long BAUDRATE = 115200,
  const unsigned I2C_ADDR = 0x70
> class App {
public:
  App() :
    alpha4(Adafruit_AlphaNum4()),
    url(F("pool.ntp.org")),
    ntpUDP(),
    timeClient(ntpUDP, url.c_str(), utcOffsetInSeconds)
  {}

  inline
  void setup() {
    Serial.begin(BAUDRATE);
  
    alpha4.begin(I2C_ADDR);
  
    alpha4.clear();
    alpha4.setBrightness(0);
    
    alpha4.writeDigitRaw(0, 0x0008);
    alpha4.writeDigitRaw(3, 0x0008);
    alpha4.writeDisplay();
    for (byte i = 0; i < 4; ++i) {
      alpha4.setBrightness(i);
      delay(200);
    }
    alpha4.writeDigitRaw(0, 0x00C0);
    alpha4.writeDigitRaw(3, 0x00C0);
    alpha4.writeDisplay();
    for (byte i = 4; i < 8; ++i) {
      alpha4.setBrightness(i);
      delay(200);
    }
    alpha4.writeDigitRaw(0, 0x00E2);
    alpha4.writeDigitRaw(3, 0x00E2);
    alpha4.writeDisplay();
    for (byte i = 8; i < 12; ++i) {
      alpha4.setBrightness(i);
      delay(200);
    }
    alpha4.writeDigitRaw(0, 0x00E3);
    alpha4.writeDigitRaw(3, 0x00E3);
    alpha4.writeDisplay();
    for (byte i = 12; i <= 15; ++i) {
      alpha4.setBrightness(i);
      delay(200);
    }
    alpha4.writeDigitRaw(1, 0x0008);
    alpha4.writeDigitRaw(2, 0x0008);
    alpha4.writeDisplay();
    delay(200);
    
    alpha4.writeDigitRaw(3, 0x07E3);
    alpha4.writeDisplay();
    delay(200);
  
    alpha4.writeDigitRaw(3, 0x00E3);
    alpha4.writeDisplay();
    delay(1000);
  
    display(F("HORLOGE 1,0 (c) Marc SIBERT"));
    delay(1000);
    display(String(F("COMPILE ")) + __DATE__ + F(" - ") + __TIME__);
    delay(1000);
  
  
    setupWiFi(WIFI_SSID, WIFI_PASSWORD);
    timeClient.update();
    displayDate(timeClient.getEpochTime() / 86400L);
  }

/**  
 * Méthode appelée perpétuellement.
 */
  inline
  void loop() {
    for (byte i = 0; i < 10; ++i) {
      timeClient.update();
      const byte hh = timeClient.getHours();
      alpha4.writeDigitAscii(0, hh < 10 ? ' ' : ('0' + hh / 10));
      alpha4.writeDigitAscii(1, '0' + hh % 10, !(i/5));
      const byte mn = timeClient.getMinutes();
      alpha4.writeDigitAscii(2, '0' + mn / 10);
      alpha4.writeDigitAscii(3, '0' + mn % 10);
      alpha4.writeDisplay();
      delay(100);
    }
  }
  
protected:
/**
 * Affiche les chaînes de caractères sur l'écran.
 * @param str La chaîne de caractère.
 */
  void display(const String& str) {
    const byte digits = 4;
    if (str.length() <= digits) {
      for (byte i = 0; i < digits; ++i) {
        alpha4.writeDigitAscii(i, i < str.length() ? str[i] : ' ');
      }
    } else {
      for (unsigned i = 0; i < str.length() - (digits - 1); ++i) {
        display(str.substring(i, i + digits));
      }  
    }
    alpha4.writeDisplay();
    delay(200);
  }

  void setupWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid, password);
//    WiFi.begin();
    display(String(F("WIFI - SSID ")) + ssid);
    auto s = WiFi.status();
    do {
      switch (s) {
        case WL_IDLE_STATUS : 
          display(F("Status change..."));
          break;
        case WL_NO_SSID_AVAIL :
          display(F("SSID not reachable!"));
          break;
        case WL_CONNECTED :
          display(F("Connected."));
          break;
        case WL_CONNECT_FAILED :
          display(F("Connection failed!"));
          break;
        case WL_CONNECTION_LOST :
          display(F("Connection lost..."));
          break;
        case WL_DISCONNECTED :
          display(F("Disconnected..."));
          break;
        default :
          Serial.print("WL_IDLE_STATUS      ="); Serial.println(WL_IDLE_STATUS);
          Serial.print("WL_NO_SSID_AVAIL    ="); Serial.println(WL_NO_SSID_AVAIL);
          Serial.print("WL_SCAN_COMPLETED   ="); Serial.println(WL_SCAN_COMPLETED);
          Serial.print("WL_CONNECTED        ="); Serial.println(WL_CONNECTED);
          Serial.print("WL_CONNECT_FAILED   ="); Serial.println(WL_CONNECT_FAILED);
          Serial.print("WL_CONNECTION_LOST  ="); Serial.println(WL_CONNECTION_LOST);
          Serial.print("WL_DISCONNECTED     ="); Serial.println(WL_DISCONNECTED);

          display(String(F("Other error #")) + String(s));
          Serial.print("Erreur wl_status_t : ");
          Serial.println(s);
          break;
      }
      delay(100);
      s = WiFi.status();
    } while (s != WL_CONNECTED);
    display(String(F("IP address ")) + WiFi.localIP().toString());
    delay(200);
  }

/**
 * @see http://howardhinnant.github.io/date_algorithms.html
 */
  void displayDate(const unsigned long& day) {
    const unsigned long z = day + 719468;
    const unsigned long era = z / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    const byte mp = (5*doy + 2)/153;                                   // [0, 11]
    const byte d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
    const byte m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
    const unsigned y = static_cast<unsigned>(yoe) + era * 400 + (m <= 2);
    
    static const String jours[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
    static const String mois[] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};
  
    display(String(jours[(day+4) % 7]) + " " + String(d) + " " + mois[mp] + " " + String(y));
    delay(1000);  
  }
  
private:
///> Accès à l'afficheur.
  Adafruit_AlphaNum4 alpha4;

///> Outils WiFi & ntp.
  String url;
  const long utcOffsetInSeconds = 3600;
  WiFiUDP ntpUDP;
  NTPClient timeClient;

};

// constexpr char url[] = "europe.pool.ntp.org";
App<> app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}
