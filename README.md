# MCP23017Encoders

 This library is dependent upon the adafruit MCP23017 library and it must be installed and included.

 The current version of the Adafruit MCP23017 also depends on the Adafruit BusIO library

 The encoder library now has an option to build for the custom 4 encoder board described here: [clicky](https://github.com/MBilge/4x-Rotary-Encoder-I2C-Board-v1.0) by adding macro definition CUSTOM_4X_ENCODER_BOARD in MPC23017Encoders.h This board also needs to have the default I2C address 0x27 passed to the board begin function. see example for details.

