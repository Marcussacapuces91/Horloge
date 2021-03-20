/**
 * Copyright 2021 Marc SIBERT
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  
 *  @brief 
 *  
 *  @author Marc SIBERT
 *  Contact: marc@sibert.fr
 */

///> @see https://arduino-esp8266.readthedocs.io/en/latest/index.html
#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#ifdef SECURE
#include <ESP8266WebServerSecure.h>
#endif

/**
 * Classe instanciant les fonctions web de réglage des paramètres.
 */
class Web {
public:

/**
 * Réglage et démarrage du/des serveurs.
 */
  void setup() {
#ifdef SECURE
    server80.onNotFound([](){
      server80.sendHeader(F("Location"), String(F("https://")) + WiFi.localIP().toString());
      server80.send(302);
    });
    server80.begin();

    server443.onNotFound([](){
      server443.send(404, F("text/plain"), F("Page not found!"));
    });
    server443.begin();    

    timeClient.update();
    displayDate(timeClient.getEpochTime() / 86400L);
#else
    server80.onNotFound([](){
      server80.send(404, F("text/plain"), F("Page not found!"));      
    });

///> index.htm or root
    server80.on(F("/"), [](){
      auto f = LittleFS.open(F("/index.htm"), "r");
      if (!f) {
        server80.send(500, F("text/plain"), F("Can't find /index.htm file in SPDIF!"));
        return;
      }
      server80.setContentLength(f.size());
      server80.sendHeader(F("Cache-Control"), F("immutable"));
      server80.send(200, F("text/html; charset=utf-8"), "");
      static const size_t BUFFER_LEN = 1000;
      char buffer[BUFFER_LEN];
      while (f.available()) {
        const auto len = f.readBytes(buffer, BUFFER_LEN);
        server80.sendContent(buffer, len);
        yield();
      }
      f.close();
    });

///> w3.css
    server80.on(F("/w3.css"), [](){
      auto f = LittleFS.open(F("/w3.css"), "r");
      if (!f) {
        server80.send(500, F("text/plain"), F("Can't find /w3.css file in SPDIF!"));
        return;
      }
      server80.setContentLength(f.size());
      server80.sendHeader(F("Cache-Control"), F("immutable"));
      server80.send(200, F("text/css; charset=utf-8"), "");
      static const size_t BUFFER_LEN = 1000;
      char buffer[BUFFER_LEN]; 
      while (f.available()) {
        const auto len = f.readBytes(buffer, BUFFER_LEN);
        server80.sendContent(buffer, len);
        yield();
      }
      f.close();
    });

    server80.on(F("/wifi/ssid"), [](){
        const auto s = '"' + String(WiFi.SSID()) + '"';
        server80.sendHeader(F("Cache-Control"), F("no-cache"));
        server80.send(200, String(F("application/json; charset=utf-8")), s);
    });

    server80.on(F("/wifi/level"), [](){
        const auto s = String(WiFi.RSSI());
        server80.sendHeader(F("Cache-Control"), F("no-cache"));
        server80.send(200, String(F("application/json; charset=utf-8")), s); 
    });

    server80.on(F("/wifi/localip"), [](){
        const auto s = '"' + String(WiFi.localIP().toString() + '"');
        server80.sendHeader(F("Cache-Control"), F("no-cache"));
        server80.send(200, String(F("application/json; charset=utf-8")), s); 
    });

    server80.on(F("/time/ntp"), [](){
        const auto s = '"' + String(WiFi.localIP().toString() + '"');
        server80.sendHeader(F("Cache-Control"), F("no-cache"));
//        server80.send(200, String(F("application/json; charset=utf-8")), s);
    });

    server80.on(F("/time/offset"), [](){
        const auto s = '"' + String(WiFi.localIP().toString() + '"');
        server80.sendHeader(F("Cache-Control"), F("no-cache"));


//        server80.send(200, String(F("application/json; charset=utf-8")), s);
    });

    server80.begin();
#endif
  }

/**
 * Méthode appelée régulièrement par la boucle répétivive.
 */
  void handleClient() {
    server80.handleClient();
#ifdef SECURE
    server443.handleClient();
#endif
  }

protected:
  void sendFile(const String& path) {
    auto f = LittleFS.open(path, "r");
    if (!f) {
      server80.send(500, F("text/plain"), String(F("Can't find ")) + path + F(" file in SPDIF!"));
      return;
    }
    server80.setContentLength(f.size());
    server80.sendHeader(F("Cache-Control"), F("immutable"));
    server80.send(200, F("text/css; charset=utf-8"), "");
    static const size_t BUFFER_LEN = 1000;
    char buffer[BUFFER_LEN]; 
    while (f.available()) {
      const auto len = f.readBytes(buffer, BUFFER_LEN);
      server80.sendContent(buffer, len);
      yield();
    }
    f.close();
  
  }


private:
  static ESP8266WebServer server80;
#ifdef SECURE
  static ESP8266WebServerSecure  server443;
#endif
};

ESP8266WebServer Web::server80(80);
#ifdef SECURE
ESP8266WebServerSecure Web::server443(443);
#endif
