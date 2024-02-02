#ifndef WIC64_MAIN_H
#define WIC64_MAIN_H

#include "wic64.h"
#include "webserver.h"
#include "buttons.h"
#include "display.h"

namespace WiC64 {
    extern Webserver *webserver;
    extern Buttons   *buttons;
    extern Display   *display;
}

#endif // WIC64_MAIN_H
