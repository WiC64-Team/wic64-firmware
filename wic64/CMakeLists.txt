idf_component_register(
    INCLUDE_DIRS "."

    SRCS "main.cpp"
    SRCS "wic64.cpp"
    SRCS "connection.cpp"
    SRCS "userport.cpp"
    SRCS "service.cpp"
    SRCS "data.cpp"
    SRCS "request.cpp"
    SRCS "command.cpp"
    SRCS "commands/echo.cpp"
    SRCS "display.cpp"
    SRCS "utilities.cpp"
    SRCS "version.h"

    REQUIRES "wic64"
    REQUIRES "arduino"
    REQUIRES "Adafruit-GFX-Library"
    REQUIRES "Adafruit_BusIO"
    REQUIRES "Adafruit_SSD1306"
    REQUIRES "OneButtonComponent"
)
