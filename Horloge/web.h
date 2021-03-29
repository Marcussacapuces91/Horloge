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

#pragma once

///> @see https://arduino-esp8266.readthedocs.io/en/latest/index.html
#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#ifdef SECURE
#include <ESP8266WebServerSecure.h>
#endif

#include <LittleFS.h>
#include <ArduinoJson.h>

class Horloge;

/**
 * Classe instanciant les fonctions web de réglage des paramètres.
 */
class Web {
public:

  Web(Horloge*const apHorloge);

/**
 * Réglage et démarrage du/des serveurs.
 */
  void setup();

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
  static
  void sendFile(const String& path, const String& contentType) {
    auto f = LittleFS.open(path, "r");
    if (!f) {
      server80.send(500, F("text/plain"), String(F("Can't find ")) + path + F(" file in SPDIF!"));
      return;
    }
    server80.setContentLength(f.size());
    server80.sendHeader(F("Cache-Control"), F("immutable"));
    server80.streamFile(f, contentType);
//    const auto l = server80.streamFile(f, contentType);
//    if (f != f.size()) { }
    f.close();
  }


private:
  static Horloge* pHorloge;

  static ESP8266WebServer server80;
#ifdef SECURE
  static ESP8266WebServerSecure  server443;
#endif
};
