/*
  MCP23017Encoders.cpp -- multiplex 8 interrupt driven rotary encoders on an MCP23017 chip over I2C
  Created by Curiosity Workshop / Michael Gerlicher, December 2020.
  
  The use of this library is at your own risk.  Don't trust it with anything that can cause damage to ANYTHING including
  persons, property, the destruction of Earth or other celestial bodies.
  
  This library is dependent upon the adafruit MCP23017 library and it must be installed and included.

  To report problems, download updates and examples, suggest enhancements or get technical support, please visit my patreon page:

     www.patreon.com/curiosityworkshop
     www.github.com/curiosity-workshop

*/

//#include "WProgram.h"

#include <Arduino.h>
#include "MCP23017Encoders.h"

int                 _MCP23017intPin;
Adafruit_MCP23X17   _mcp;

volatile struct mcpEncoder
{
    int tickValue;
    uint16_t previousBitMask;
    unsigned long msSinceLastRead;
    int currentValue;
    float accel;                        // accelerator value 
    int button_value;
    int button_change;
} mcpEncoders[MCP_ENCODERS];

MCP23017Encoders::MCP23017Encoders(int intPin)
{
    _MCP23017intPin = intPin;

    

}

void MCP23017Encoders::begin(uint8_t i2c_addr)
{
    int i;

    _mcp.begin_I2C(i2c_addr);      // use default address 0

    pinMode(_MCP23017intPin, INPUT);

    _mcp.setupInterrupts(MCP_INT_MIRROR, MCP_INT_ODR, LOW); // The mcp output interrupt pin.
    for (i = 0; i < 16; i++)
    {
        _mcp.pinMode(i, INPUT_PULLUP);
        _mcp.setupInterruptPin(i, CHANGE);
        button_clear(i);
      
    }

    _mcp.readGPIOAB(); // Initialize for interrupts.

    attachInterrupt(digitalPinToInterrupt(_MCP23017intPin), MCP23017EncoderISR, FALLING);

    for (i = 0; i < MCP_ENCODERS; i++)
    {
        mcpEncoders[i].accel = 1.0;
        mcpEncoders[i].msSinceLastRead = millis();
    }

}


int MCP23017Encoders::read(int encoder)
{
    
    float w;

    if (encoder < 0 || encoder >= MCP_ENCODERS) return 0;
   
    w = (200*abs(mcpEncoders[encoder].tickValue)) / (millis() - mcpEncoders[encoder].msSinceLastRead) ;
/*    
    Serial.print(" t="); 
    Serial.print((millis() - mcpEncoders[encoder].msSinceLastRead)); 
    Serial.print(", "); 

    Serial.print(" w="); 
    Serial.print(w); 
    Serial.print(", "); 
*/    
    if (w > 11.0) 
    {
    /*
       Serial.print(" w="); 
       Serial.print(mcpEncoders[encoder].msSinceLastRead); 
       Serial.print(", "); 
       
       Serial.print("Acc, "); 
     */
       mcpEncoders[encoder].currentValue += mcpEncoders[encoder].tickValue * mcpEncoders[encoder].accel;
    }
    else  
    {
       mcpEncoders[encoder].currentValue += mcpEncoders[encoder].tickValue;
    }
    mcpEncoders[encoder].tickValue = 0;
    mcpEncoders[encoder].msSinceLastRead = millis();

 
    return mcpEncoders[encoder].currentValue;
}

void MCP23017Encoders::write(int encoder, int value)
{
    if (encoder < 0 || encoder >= MCP_ENCODERS) return 0;

    mcpEncoders[encoder].currentValue = value;
}

void MCP23017Encoders::setAccel(int encoder, float value)
{
    mcpEncoders[encoder].accel = value;
}

int MCP23017Encoders::button_read(int button)
{
   return mcpEncoders[button].button_value;
}

int MCP23017Encoders::button_change(int button)
{
   return mcpEncoders[button].button_change;
}

int MCP23017Encoders::button_clear(int button)
{
   mcpEncoders[button].button_change = 0;
}


/*
  Interrupt service
*/
static void MCP23017EncoderISR(void)
{
    uint8_t p;
    uint16_t v;


    noInterrupts();

    // Stop interrupts from external pin to avoid issues with i2c
    detachInterrupt(digitalPinToInterrupt(_MCP23017intPin));
    interrupts(); // re-start interrupts for mcp     

    p = _mcp.getLastInterruptPin();

    v = _mcp.readGPIOAB();
    

    switch (p)
    {
#ifdef CUSTOM_4X_ENCODER_BOARD

   case 7:
      mcpEncoders[0].button_change = 1;
      mcpEncoders[0].button_value = v & 0x0080 ? 0:1;
      break;

   case 4:
      mcpEncoders[1].button_change = 2;
      mcpEncoders[1].button_value = v & 0x0010 ? 0:2;
      break;

   case 8:
      mcpEncoders[2].button_change = 4;
      mcpEncoders[2].button_value = v & 0x0100 ? 0:4;
      break;

   case 11:
      mcpEncoders[3].button_change = 8;
      mcpEncoders[3].button_value = v & 0x0800 ? 0:8;
      break;


    case 5:  case 6:
        v = (v & 0x0060) >> 3;
        v = v | mcpEncoders[0].previousBitMask;
        mcpEncoders[0].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[0].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[0].tickValue--;
            break;
            
        default:
            break;            
        }
        break;

    case 2:  case 3:
        v = (v & 0x000C);
        v = v | mcpEncoders[1].previousBitMask;
        mcpEncoders[1].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[1].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[1].tickValue--;
            break;
                     
        default:
            break;            
        }
        break;

    case 9:   case 10:

        v = (v & 0x0600) >> 7;
        v = v | mcpEncoders[2].previousBitMask;
        mcpEncoders[2].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[2].tickValue--;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[2].tickValue++;
            break;
                       
        default:
            break;            
        }
        break;

    case 12:   case 13:
        v = (v & 0x3000) >> 10;
        v = v | mcpEncoders[3].previousBitMask;
        mcpEncoders[3].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[3].tickValue--;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[3].tickValue++;
            break;
            
        default:
            break;            
        }
        break;

   default:
      break;
      
#else

    case 0:  case 1:
        v = (v & 0x0003) << 2;
        v = v | mcpEncoders[0].previousBitMask;
        mcpEncoders[0].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[0].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[0].tickValue--;
            break;
            /*      case 3: case 12:          // ToDo:  implement this but only on rising edges of one pin
                    mcpEncoders[0].tickValue += 2;
                    break;
                  default:
                    mcpEncoders[0].tickValue -= 2;
                    break;
              */
        }


        break;


    case 2:  case 3:
        v = (v & 0x000C);
        v = v | mcpEncoders[1].previousBitMask;
        mcpEncoders[1].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[1].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[1].tickValue--;
            break;
         
        }

        break;


    case 4:  case 5:

        v = (v & 0x0030) >> 2;
        v = v | mcpEncoders[2].previousBitMask;
        mcpEncoders[2].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[2].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[2].tickValue--;
            break;
         
        }

        break;

    case 6:   case 7:
        v = (v & 0x00C0) >> 4;
        v = v | mcpEncoders[3].previousBitMask;
        mcpEncoders[3].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[3].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[3].tickValue--;
            break;
           
        }

        break;

    case 8:    case 9:
        v = (v & 0x0300) >> 6;
        v = v | mcpEncoders[4].previousBitMask;
        mcpEncoders[4].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[4].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[4].tickValue--;
            break;
          
        }

        break;

    case 10:    case 11:
        v = (v & 0x0C00) >> 8;
        v = v | mcpEncoders[5].previousBitMask;
        mcpEncoders[5].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[5].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[5].tickValue--;
            break;
        
        }

        break;

    case 12:   case 13:
        v = (v & 0x3000) >> 10;
        v = v | mcpEncoders[6].previousBitMask;
        mcpEncoders[6].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[6].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[6].tickValue--;
            break;
         
        }

        break;

    case 14:   case 15:
        v = (v & 0xC000) >> 12;
        v = v | mcpEncoders[7].previousBitMask;
        mcpEncoders[7].previousBitMask = v >> 2;

        switch (v)
        {
        case 0: case 5: case 10: case 15:
            break;

        case 1: case 7: case 8: case 14:
            mcpEncoders[7].tickValue++;
            break;

        case 2: case 4: case 11: case 13:
            mcpEncoders[7].tickValue--;
            break;
         
        }

        break;
#endif
    }

 

    EIFR = 0x01;            // Clear


    attachInterrupt(digitalPinToInterrupt(_MCP23017intPin), MCP23017EncoderISR, FALLING);
    
}

