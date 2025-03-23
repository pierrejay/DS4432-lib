# DS4432 Arduino Library

A simple Arduino library to control the DS4432 Dual-Channel I2C Current DAC.

## Overview

The DS4432 is a dual-channel, I2C programmable current DAC capable of both sinking and sourcing current up to 200μA. Each DAC output has 127 sink and 127 source settings that are programmed using the I2C interface.

This library provides a clean API to interface with the DS4432 from any Arduino-compatible platform.

## Features

- Simple control of both DAC channels
- Source mode (+1 to +127) and sink mode (-1 to -127) support
- Error checking and validation
- Compact code size

## Hardware Connection

Connect your DS4432 to your Arduino/ESP8266/ESP32:

- VCC → 3.3V or 5V (DS4432 works from 2.7V to 5.5V)
- GND → GND
- SDA → SDA or any GPIO pin (when using Wire.begin(SDA_PIN, SCL_PIN))
- SCL → SCL or any GPIO pin (when using Wire.begin(SDA_PIN, SCL_PIN))
- Connect appropriate resistors to FS0 and FS1 to set the full-scale current

## Usage

```cpp
#include <Wire.h>
#include "DS4432.h"

void setup() {
  Wire.begin();  // Initialize I2C
  
  // Set channel 0 to source 50 (about 39% of full scale)
  DS4432::set(Wire, 0, 50);
  
  // Set channel 1 to sink 75 (about 59% of full scale)
  DS4432::set(Wire, 1, -75);
  
  // Read back the values
  int8_t ch0 = DS4432::get(Wire, 0);  // Should return 50
  int8_t ch1 = DS4432::get(Wire, 1);  // Should return -75
}
```

## Applications

The DS4432 is particularly useful for creating adjustable regulators (e.g. Buck) by injecting current into the feedback node (voltage divider setting the reference voltage in most cases). See the Maxim datasheet for more implementation details.

## License

MIT License.

## References

For more information, see the [DS4432 datasheet](https://datasheets.maximintegrated.com/en/ds/DS4432.pdf).