




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
      server80.sendHeader("Location", String(F("https://")) + WiFi.localIP().toString());
      server80.send(302);
    });
    server80.begin();

    server443.onNotFound([](){
      server443.send(404, "text/plain", "Page not found!");
    });
    server443.begin();    

    timeClient.update();
    displayDate(timeClient.getEpochTime() / 86400L);
#else
    server80.onNotFound([](){
      server80.send(404, F("text/plain"), F("Page not found!"));      
    });

    server80.on("/", [](){
      auto f = LittleFS.open(F("/index.htm"), "r");
      if (!f) {
        server80.send(500, F("text/plain"), F("Can't find /index.htm file in SPDIF!"));
        return;
      }
      server80.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server80.send(200, F("text/html"), "");
      static const size_t BUFFER_LEN = 1000;
      char buffer[BUFFER_LEN]; 
      while (f.available()) {
        const auto len = f.readBytes(buffer, BUFFER_LEN);
        server80.sendContent(buffer, len);
        yield();
      }
      f.close();
      
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
  static ESP8266WebServer server80;

#ifdef SECURE
  static ESP8266WebServerSecure  server443;
#endif
  
};

ESP8266WebServer Web::server80(80);

#ifdef SECURE
ESP8266WebServerSecure Web::server443(443);
#endif
