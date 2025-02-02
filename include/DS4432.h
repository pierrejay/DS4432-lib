#include <Arduino.h>
#include <Wire.h>

namespace DS4432 {

static constexpr uint8_t ADDR = 0x48; // 0x90 in 7 bits (Arduino Wire)
static constexpr uint8_t OUT0 = 0xF8;
static constexpr uint8_t OUT1 = 0xF9;
static constexpr int8_t ERROR = -128; // DS4432::get() returns this value if there is an error (-127...127 are valid)

/* @brief Convert an int8_t (-128..+127) to a DS4432 byte (bit7=sign, bits6..0=magnitude).
 * @param cmd: the "analog" command to convert (-128..+127)
 * @return The DS4432 command byte
 */
inline uint8_t cmdToByte(int8_t cmd) {
    // Saturate -128 to -127 if needed:
    if (cmd == -128) {
        cmd = -127;
    }
    // Sign = 1 if cmd >= 0 (source), 0 otherwise (sink)
    bool signBit = (cmd >= 0);
    // Absolute value on 7 bits (0..127)
    uint8_t magnitude = (uint8_t)((cmd < 0) ? -cmd : cmd); 
    // Bit7 = sign, bits6..0 = magnitude
    return (signBit ? 0x80 : 0x00) | (magnitude & 0x7F);
}

/* @brief Convert a DS4432 byte to int8_t (-127..+127)
 * @param val: the DS4432 byte
 * @return The "analog" command -127..+127
 */
inline int8_t byteToCmd(uint8_t val) {
    // Bit7 => sign
    bool signBit = (val & 0x80) != 0;
    // Bits6..0 => magnitude 0..127
    uint8_t mag = val & 0x7F;
    // Apply the sign
    int8_t cmd = (int8_t)mag;
    if (!signBit) {
        cmd = -cmd;  // sink
    }
    return cmd;
}

/* @brief Set the command to the DS4432
 * @param i2c: the I2C object
 * @param output: the output to set (0 for OUT0, 1 for OUT1)
 * @param cmd: the "analog" command to set [-127..+127]
 * @param ack: if true, use a read after the write to check that the value has been written
 * @return true if the command has been set, false otherwise
 */
inline bool set(TwoWire &i2c, uint8_t output, int8_t cmd, bool ack = false) {
    // Check we only handle 0 or 1
    if (output > 1) return false;
    // Select the register
    uint8_t reg = (output == 0) ? OUT0 : OUT1;
    // Compute the byte to send (bit7=signe, bits6..0=magnitude)
    uint8_t dataByte = cmdToByte(cmd);

    // I2C write
    i2c.beginTransmission(ADDR);  // DS4432 address in 7 bits
    i2c.write(reg);               // internal memory to write (F8 or F9)
    i2c.write(dataByte);          // the command
    if (i2c.endTransmission() != 0) return false; // !=0 => I2C error

    // Check the value if requested
    if (ack) {
        int8_t readback = get(i2c, output);
        return (readback != ERROR) && (readback == cmd);
    }
    
    return true;
}


/* @brief Read the command from the DS4432
 * @param i2c: the I2C object
 * @param output: the output to read (0 for OUT0, 1 for OUT1)
 * @return Current value -127..+127, or ERROR (-128) if there is an error
 */
inline int8_t get(TwoWire &i2c, uint8_t output) {
    if (output > 1) return ERROR;
    uint8_t reg = (output == 0) ? OUT0 : OUT1;

    // First point to the register to read
    i2c.beginTransmission(ADDR);
    i2c.write(reg);
    // End transmission without STOP (→ repeated START)
    i2c.endTransmission(false);

    // Read 1 byte
    i2c.requestFrom(ADDR, 1);
    if (!i2c.available()) {
        return ERROR; // read error
    }
    uint8_t val = i2c.read();

    // Convert the DS4432 byte to -127..+127
    return byteToCmd(val);
}

} // namespace DS4432
