/* 
 *  MCP23017Encoders Example
 *  djrm, modified April 14th 2022 to enable use of latest Adafruit libraries and custom encoder board hardware
 */


#include <Wire.h>
// Adafruit_BusIO is needed by latest Adafruit_MCP23017 library (v2.1.0), both are needed by the MCP23017Encoders library
// Note: small syntax changes needed since original MCP23017Encoders library release
// Note: I2C address now passed to MPC23017.begin function, needed if alternative hardware used.
#include <MCP23017Encoders.h>

MCP23017Encoders myMCP23017Encoders(3);         // Parameter is arduino interrupt pin that the MCP23017 is connected to

//////////////////////////////////////////////
void setup()
{

  Serial.begin(115200); 
  Serial.println("MPC23017 TestBed");
  myMCP23017Encoders.begin(0x27);
  
  myMCP23017Encoders.setAccel(0, 10.0);
  myMCP23017Encoders.setAccel(1, 10.0);
  myMCP23017Encoders.setAccel(2, 10.0);
  myMCP23017Encoders.setAccel(3, 10.0);

}

//////////////////////////////////////////////
void loop()
{

  delay(100);

  for (int i = 0; i < MCP_ENCODERS; i++)
  {
    Serial.print(myMCP23017Encoders.read(i)); Serial.print(" ,"); 
  }

  for (int i = 0; i < MCP_ENCODERS; i++)
  {
    Serial.print("button=");
    Serial.print(myMCP23017Encoders.button_read(i)); Serial.print(" "); 
    Serial.print("change=");
    Serial.print(myMCP23017Encoders.button_change(i)); Serial.print(" "); 
    myMCP23017Encoders.button_clear(i);
  }



  
  Serial.println();


}
