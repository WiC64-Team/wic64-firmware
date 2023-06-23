#include "esp32-hal-log.h"

#include "wic64.h"
#include "display.h"
#include "connection.h"
#include "client.h"
#include "userport.h"
#include "service.h"
#include "utilities.h"
#include "version.h"

using namespace WiC64;
namespace WiC64 {
    Display *display;
    Connection *connection;
    Client *client;
    Service *service;
    Userport *userport;

    WiC64::WiC64() {
        log_i("Firmware version %s", WIC64_VERSION_STRING);

        display = new Display();
        connection = new Connection();
        client = new Client();
        service = new Service();
        userport = new Userport();

        connection->connect();
        userport->connect();
    }
}