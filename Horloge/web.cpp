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

#include "web.h"
#include "horloge.h"

Web::Web(Horloge*const aHorloge) { pHorloge = aHorloge; }

static const char MIME_TYPE_JSON[] PROGMEM = "application/json; charset=utf-8";
static const char MIME_TYPE_TEXT[] PROGMEM = "text/plain; charset=utf-8";

static const char CACHE_CONTROL[] PROGMEM = "Cache-Control";
static const char NO_CACHE[] PROGMEM = "no-cache";

void Web::setup() {
#ifdef SECURE
  server80.onNotFound([](){
    server80.sendHeader(F("Location"), String(F("https://")) + WiFi.localIP().toString());
    server80.send(302);
  });
  server80.begin();

  server443.onNotFound([](){
    server443.send(404, FPSTR(MIME_TYPE_TEXT), F("Page not found!"));
  });
  server443.begin();    
#else
  server80.onNotFound([](){
    server80.send(404, FPSTR(MIME_TYPE_TEXT), F("Page not found!"));      
  });

///> index.htm or root
  const auto indexPath = String(F("/index.htm"));
  server80.serveStatic(indexPath.c_str(), LittleFS, indexPath.c_str());
  server80.serveStatic("/", LittleFS, indexPath.c_str());

///> w3.css
  const auto w3Path = String(F("/w3.css"));
  server80.serveStatic(w3Path.c_str(), LittleFS, w3Path.c_str());

///> favicon
  const auto faviconPath = String(F("/favicon.ico"));
  server80.serveStatic(faviconPath.c_str(), LittleFS, faviconPath.c_str());

  server80.on(F("/wifi/ssid"), HTTP_GET, [](){
      server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
      server80.send(200, FPSTR(MIME_TYPE_JSON), String('"') + WiFi.SSID() + '"');
  });

  server80.on(F("/wifi/level"), HTTP_GET, [](){
      const auto s = String(WiFi.RSSI());
      server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
      server80.send(200, FPSTR(MIME_TYPE_JSON), s); 
  });

  server80.on(F("/wifi/localip"), HTTP_GET, [](){
      const auto s = String('"') + WiFi.localIP().toString() + '"';
      server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
      server80.send(200, FPSTR(MIME_TYPE_JSON), s); 
  });

  server80.on(F("/time/ntp"), HTTP_GET, [](){
      const auto s = String('"') + Web::pHorloge->url + '"';
      server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
      server80.send(200, FPSTR(MIME_TYPE_JSON), s);
  });

  server80.on(F("/time/ntp"), HTTP_POST, [](){
    StaticJsonDocument<128> doc;
    const auto err = deserializeJson(doc, server80.arg(F("plain")));
    if (err) {
      server80.send(500, FPSTR(MIME_TYPE_TEXT), String(F("Error deserialize JSON: ")) + err.c_str());      
      return;
    }
    Web::pHorloge->updateURL(doc.as<const char*>());
    server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
    server80.send(204, FPSTR(MIME_TYPE_TEXT), F("No Content"));
  });

  server80.on(F("/time/offset"), HTTP_GET, [](){
      const auto offset = Web::pHorloge->utcOffsetInSeconds;
      char s[10];
      snprintf_P(s, sizeof(s), PSTR("\"%+03d:%02d\""), offset / 3600, abs(offset % 3600) / 60);
      server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
      server80.send(200, FPSTR(MIME_TYPE_JSON), s);
  });

  server80.on(F("/time/offset"), HTTP_POST, [](){
    StaticJsonDocument<128> doc;
    const auto err = deserializeJson(doc, server80.arg(F("plain")));
    if (err) {
      server80.send(500, FPSTR(MIME_TYPE_TEXT), String(F("Error deserialize JSON: ")) + err.c_str());      
      return;
    }
    int h,m = 0;
    if (sscanf(doc.as<const char*>(), "%d:%d", &h, &m) != 2) {
      server80.send(500, FPSTR(MIME_TYPE_TEXT), F("Invalid answer!"));      
      return;
    }
    Web::pHorloge->updateUTCOffsetInSeconds((h * 60 + abs(m)) * 60L);
    server80.sendHeader(FPSTR(CACHE_CONTROL), FPSTR(NO_CACHE));
    server80.send(204, FPSTR(MIME_TYPE_TEXT), F("No Content"));
  });

  server80.begin();
#endif
}

Horloge* Web::pHorloge = nullptr;

ESP8266WebServer Web::server80;
#ifdef SECURE
ESP8266WebServerSecure Web::server443;
#endif
