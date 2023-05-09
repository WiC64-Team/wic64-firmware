bool displayattached = false;
bool inputmode = true;
bool transferdata = false;
bool ex = false;                          // Flag ob ESP gerade ein Kommand abarbeitet
bool pending = false;
bool checkprefs=false;
bool httpInitResult;
bool payloadswap=false;
int swap=0;
int total=0;
int templen=0;
String temp;
unsigned long crashcounter = 0;           // Zähler für die Anzahl der Commandos - Nur für Debugging
unsigned long payloadsize;                // Größe der zu transferierenden Daten
unsigned long pay2size;                   // Größe des zweiten Buffers bei Übertragungen >64kb
unsigned long timeout;                    // Timeout Counter in Millis
unsigned long count = 0;
unsigned long sizel;                      // Counter anzahl der bytes vom c64 low byte
unsigned long sizeh;                      // Counter anzahl der bytes vom c64 high byte
unsigned long transsize;                  // Anzahl der bytes die vom C64 kommen sollen

String defaultserver = "http://www.wic64.de/prg/"; // Zum Test des WiC64 Kernals Load ! 
String defaultsaveserver = "www.wic64.de:80:/prg/up.php:"; // Zum Test des WiC64 Kernals Save !
// String defaultsaveserver = "sk.sx-64.de:80:/up/up.php:"; // Zum Test des WiC64 Kernals Save !  
String firmwareversion;
int databyte = 0;                         // Das Byte was gelesen oder geschrieben wird am Bus
int buttontimeA = 0;                      // Zähler für das Drücken von Button Boot0 auf dem ESP32
int buttontimeB = 0;                      // Zähler für das Drücken von SpecialButton auf dem ESP32
int buttontimeBcount = 0;
char ssid[50];                            // SSID für's WLAN
char password[50];                        // PASSWORT für's WLAN
char buff[65535] = { 0 };                 // Buffer für große Dateien >64kb
String ssiddata;                          // Konvertier-Speicher
String passworddata;                      // Konvertier-Speicher
bool killswitch = false;                  // WiC64 deaktivieren vom Userport oder aktivieren
bool killled = false;                     // LED an oder aus
bool displayrotate = false;               // Rotate display 180 degrees
String payload;                           // Datenpuffer für die Übertragung
String payloadB;                          // Datenpuffer für Übertragung >64kb
String messagetoc64;                      // Message an C64 z.b. Daten oder Ladefehler 
String act;                               // Display Anzeige was gerade passiert ...
String input;                             // Eingabesting vom C64
String lastinput;                         // Puffer Eingabestring
String setserver;                         // Stringpuffer für SETS
String setsaveserver;                     // Stringpuffer für SETS save server
String httpstring;                        // HTTP Adresse von der geladen wird
String wic64hostname;                     // WiC64 Hostname bestehend aus WiC64+MAC des ESP
String localip;                           // IP des C64 WiC
String localssid;                         // WLAN zu dem Verbunden wurde
String prefname;                          // Liesst oder speichert Daten im Eeprom
String prefdata;                          // Liesst oder speichert Daten im Eeprom
String prefstring1;                        // Zum Suchen & Ersetzen in http kommandos
String prefstring2;  
String prefstring3;
String prefstring1data;
String prefstring2data;
String prefstring3data;
String prefanswer;
String sectoken;                          // Securitytoken 
String sectokenname;                      // Securitytokenname  
char sep = 1;                             // Byte zum separieren der einzelnen Strings (Trennbyte)
int udpport = 8080;                       // UDP message port
int tcpport = 8081;                       // TCP message port

int port1;
String server1;
String serverpath1;
String filename;
String udpmsg;
String udpmsgr;
String remotedata;
uint8_t buffer[50];
IPAddress ip;
WiFiUDP udp;
Preferences preferences;
WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80
HTTPClient http;         // HTTP Client create
WiFiClient client;       // WiFi Client create
WiFiClientSecure clientsec; // WiFi Client https create
char tcpanswer[64];


const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = 3600;
int   daylightOffset_sec = 3600;
String acttime;
        
#define I2C_SDA 13       // Display I2C Bus Pins festlegen Pin SDA
#define I2C_SCL 15       // Display I2C Bus Pins festlegen Pin SCL
#define OLED_RESET -1    // Kein RESET am Display verwendet
#define SCREEN_WIDTH 128 // Display Breite in Pixeln
#define SCREEN_HEIGHT 64 // Display Höhe in Pixeln
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Display config definieren.
void displaystuff(String act);    // Display Daten anzeigen
void disablewic();      // WiC64 komplett deaktiveren
void notFound();        // Sendet "Not Found"-Seite
void handleRoot();      // Sendet / root vom Webserver
void handleUpdate();    // Sendet Update Seite
void handleDeveloper(); // Sendet Update 2 Seite (Developer only)
void handleDeveloper2();// Sendet geheime Update-Seite für 2. Testchannel
void handleDowngrade(); // Sendet downgrade Firmware zum WiC64
void handleNotFound();  // Unbekannte Seite aufgerufen - 404
void handshake_flag2();
void IRAM_ATTR PA2irq();    
void IRAM_ATTR PC2irq();

void WiC64connected(WiFiEvent_t event, WiFiEventInfo_t info) { displaystuff("WLAN reconnect");}
void WiC64disconnected(WiFiEvent_t event, WiFiEventInfo_t info) { displaystuff("WLAN lost !");}
void WiC64ipconfig(WiFiEvent_t event, WiFiEventInfo_t info) { displaystuff("got new IP");}
