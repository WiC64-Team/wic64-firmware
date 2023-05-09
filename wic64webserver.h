/////// Subs Webserver - Das erklärt sich ganz von selbst

void handleRoot() {

  String compilerdate = __DATE__ " " __TIME__;
  String compilervars = ((__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, esp_get_idf_version() );

  String HTML = "<html><head><title>WiC64 - by KiWi</title></head><body>"
                  "<center><h1><strong>WiC64</strong></h1></center>\n"
                  "<hr />\n"
                  "<p>WiC64 core :" + compilerdate + compilervars + " - <a href=\"/update\"><b>Firmware update</b></a>  -                        <a href=\"/developer\"><b>Switch to DEV-Channel</b></a>  -                        <a href=\"/downgrade\"><b>Downgrade firmware to previous version</b></a>"
                  "<hr />\n"
                  "<br><br><br><br>" + wic64hostname + "<br>" + localip + "<br><br><br>"
                  "<hr />\n"
                  "<center> WiC64 - by GMP - KiWi - Lazy Jones - YPS</center><hr />\n"
                  "</body></html>";
                  server.send(200, "text/html", HTML);
                  log_i("--> Root webpage requested");
}

void handleUpdate() {
   if (pending == true) { handshake_flag2(); pending = false; }
   displaystuff("FW update 1WWW");
   detachInterrupt(digitalPinToInterrupt(PA2));       // Interrupts killen - sonst crashed httpupdate
   detachInterrupt(digitalPinToInterrupt(PC2));



  String HTML =   "<html><head><title>WiC64 - by KiWi</title><meta http-equiv=\"refresh\" content=\"30;url=/\" /></head><body>"
                  "<center><h1><strong>WiC64</strong></h1></center>\n"
                  "<hr />\n"
                  "<p><b>WiC64 Firmware update is running - Please wait ....</b>"
                  "<hr />\n"
                  "<br><br><br><br><br><br><br><br>"
                  "<hr />\n"
                  "<center> WiC64 - by GMP - KiWi - Lazy Jones - YPS</center><hr />\n"
                  "</body></html>";
                  server.send(200, "text/html", HTML);

   log_i("--> Standard firmware is updating now !");
   WiFiClient client;


   t_httpUpdate_return ret = httpUpdate.update(client, "http://sk.sx-64.de:80/wic64/wic64.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                log_d("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                log_d("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                log_d("HTTP_UPDATE_OK");
                break;
        }

      attachInterrupt(digitalPinToInterrupt(PA2), PA2irq, CHANGE);      // Interrupts wiederherstellen wenn es kein Firmwareupdate gab - Ansonsten macht der ESP einen RESET
      attachInterrupt(digitalPinToInterrupt(PC2), PC2irq, HIGH);
}

void handleDeveloper() {
   if (pending == true) { handshake_flag2(); pending = false; }
   displaystuff("FW update 2WWW");
   detachInterrupt(digitalPinToInterrupt(PA2));       // Interrupts killen - sonst crashed httpupdate
   detachInterrupt(digitalPinToInterrupt(PC2));



  String HTML =   "<html><head><title>WiC64 - by KiWi</title><meta http-equiv=\"refresh\" content=\"30;url=/\" /></head><body>"
                  "<center><h1><strong>WiC64</strong></h1></center>\n"
                  "<hr />\n"
                  "<p><b>WiC64 DEVELOPER Firmware update is running - Please wait ....</b>"
                  "<hr />\n"
                  "<br><br><br><br><br><br><br><br>"
                  "<hr />\n"
                  "<center> WiC64 - by GMP - KiWi - Lazy Jones - YPS</center><hr />\n"
                  "</body></html>";
                  server.send(200, "text/html", HTML);

   log_i("--> Developer firmware is updating now !");
   WiFiClient client;


   t_httpUpdate_return ret = httpUpdate.update(client, "http://sk.sx-64.de:80/wic64-d1/wic64.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                log_d("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                log_d("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                log_d("HTTP_UPDATE_OK");
                break;
        }

      attachInterrupt(digitalPinToInterrupt(PA2), PA2irq, CHANGE);      // Interrupts wiederherstellen wenn es kein Firmwareupdate gab - Ansonsten macht der ESP einen RESET
      attachInterrupt(digitalPinToInterrupt(PC2), PC2irq, HIGH);
}

void handleDeveloper2() {
   if (pending == true) { handshake_flag2(); pending = false; }
   displaystuff("FW update 3WWW");
   detachInterrupt(digitalPinToInterrupt(PA2));       // Interrupts killen - sonst crashed httpupdate
   detachInterrupt(digitalPinToInterrupt(PC2));



  String HTML =   "<html><head><title>WiC64 - by KiWi</title><meta http-equiv=\"refresh\" content=\"30;url=/\" /></head><body>"
                  "<center><h1><strong>WiC64</strong></h1></center>\n"
                  "<hr />\n"
                  "<p><b>SWiC64 DEVELOPER II Firmware update is running - Please wait ....</b>"
                  "<hr />\n"
                  "<br><br><br><br><br><br><br><br>"
                  "<hr />\n"
                  "<center> WiC64 - by GMP - KiWi - Lazy Jones - YPS</center><hr />\n"
                  "</body></html>";
                  server.send(200, "text/html", HTML);

   log_i("--> Developer II firmware is updating now !");
   WiFiClient client;

   String tempmac = WiFi.macAddress();
   t_httpUpdate_return ret = httpUpdate.update(client, "http://sk.sx-64.de:80/wic64-d2/wic64.bin?" + tempmac );

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                log_d("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                log_d("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                log_d("HTTP_UPDATE_OK");
                break;
        }

      attachInterrupt(digitalPinToInterrupt(PA2), PA2irq, CHANGE);      // Interrupts wiederherstellen wenn es kein Firmwareupdate gab - Ansonsten macht der ESP einen RESET
      attachInterrupt(digitalPinToInterrupt(PC2), PC2irq, HIGH);
}

void handleDowngrade() {
   if (pending == true) { handshake_flag2(); pending = false; }
   displaystuff("FW downgrade");
   detachInterrupt(digitalPinToInterrupt(PA2));       // Interrupts killen - sonst crashed httpupdate
   detachInterrupt(digitalPinToInterrupt(PC2));



  String HTML =   "<html><head><title>WiC64 - by KiWi</title><meta http-equiv=\"refresh\" content=\"30;url=/\" /></head><body>"
                  "<center><h1><strong>WiC64</strong></h1></center>\n"
                  "<hr />\n"
                  "<p><b>SWiC64 downgrade is running - Please wait ....</b>"
                  "<hr />\n"
                  "<br><br><br><br><br><br><br><br>"
                  "<hr />\n"
                  "<center> WiC64 - by GMP - KiWi - Lazy Jones - YPS</center><hr />\n"
                  "</body></html>";
                  server.send(200, "text/html", HTML);

   log_i("--> Firmware downgrade is updating now !");
   WiFiClient client;


   t_httpUpdate_return ret = httpUpdate.update(client, "http://sk.sx-64.de:80/wic64/wic64-old.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                log_d("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                log_d("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                log_d("HTTP_UPDATE_OK");
                break;
        }

      attachInterrupt(digitalPinToInterrupt(PA2), PA2irq, CHANGE);      // Interrupts wiederherstellen wenn es kein Firmwareupdate gab - Ansonsten macht der ESP einen RESET
      attachInterrupt(digitalPinToInterrupt(PC2), PC2irq, HIGH);
}

void notFound(){
  String HTML = F("<html><head><title>404 Not Found</title></head><body>"
                  "<h1>Not Found</h1>"
                  "<p>The requested URL was not found on this webserver.</p>"
                  "</body></html>");
            server.send(404, "text/html", HTML);
            log_i("--> 404 not found");
}
