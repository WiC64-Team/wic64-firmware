idf_component_register(
	SRCS "wic64.cpp" "display.cpp" "connection.cpp" "userport.cpp"
	INCLUDE_DIRS "."
	REQUIRES "wic64"
	REQUIRES "arduino"
	REQUIRES "Adafruit-GFX-Library"
	REQUIRES "Adafruit_BusIO"
	REQUIRES "Adafruit_SSD1306"
	REQUIRES "OneButtonComponent")
