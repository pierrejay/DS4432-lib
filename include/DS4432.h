#include <Arduino.h>
#include <Wire.h>

namespace DS4432 {

static constexpr uint8_t ADDR = 0x48; // 0x90 en 7 bits (Arduino Wire)
static constexpr uint8_t OUT0 = 0xF8;
static constexpr uint8_t OUT1 = 0xF9;
static constexpr int8_t ERROR = -128; // DS4432::get() returns this value if there is an error (-127...127 are valid)

// Convertit un int8_t (-128..+127) en octet DS4432 (bit7=signe, bits6..0=magnitude).
inline uint8_t cmdToByte(int8_t cmd) {
    // Sature -128 en -127 si besoin :
    if (cmd == -128) {
        cmd = -127;
    }
    // Sign = 1 si cmd >= 0 (source), 0 sinon (sink)
    bool signBit = (cmd >= 0);
    // Valeur absolue sur 7 bits (0..127)
    uint8_t magnitude = (uint8_t)((cmd < 0) ? -cmd : cmd); 
    // Bit7 = sign, bits6..0 = magnitude
    return (signBit ? 0x80 : 0x00) | (magnitude & 0x7F);
}

// Convertit un octet DS4432 en int8_t (-127..+127)
inline int8_t byteToCmd(uint8_t val) {
    // Bit7 => signe
    bool signBit = (val & 0x80) != 0;
    // Bits6..0 => magnitude 0..127
    uint8_t mag = val & 0x7F;
    // Applique le signe
    int8_t cmd = (int8_t)mag;
    if (!signBit) {
        cmd = -cmd;  // sink
    }
    return cmd;
}

// output = 0 pour OUT0 (0xF8), 1 pour OUT1 (0xF9)
// cmd    = -127..+127 ; négatif => sink, positif => source, 0 => 0mA
inline bool set(TwoWire &i2c, uint8_t output, int8_t cmd) {
    // Vérifie qu’on ne gère que 0 ou 1
    if (output > 1) return false;
    // Sélectionne le registre
    uint8_t reg = (output == 0) ? OUT0 : OUT1;
    // Calcule l’octet à envoyer (bit7=signe, bits6..0=magnitude)
    uint8_t dataByte = cmdToByte(cmd);

    // Écriture I2C
    i2c.beginTransmission(ADDR);  // adresse DS4432 en 7 bits
    i2c.write(reg);               // mémoire interne à écrire (F8 ou F9)
    i2c.write(dataByte);          // la consigne
    return (i2c.endTransmission() == 0); // 0 => succès
}

// output = 0 pour OUT0 (0xF8), 1 pour OUT1 (0xF9)
// renvoie la consigne -127..+127
inline int8_t get(TwoWire &i2c, uint8_t output) {
    if (output > 1) return ERROR; // ou gérer autrement l’erreur
    uint8_t reg = (output == 0) ? OUT0 : OUT1;

    // On pointe d’abord le registre à lire
    i2c.beginTransmission(ADDR);
    i2c.write(reg);
    // Fin de transmission sans STOP (→ repeated START)
    i2c.endTransmission(false);

    // On lit 1 octet
    i2c.requestFrom(ADDR, 1);
    if (!i2c.available()) {
        return ERROR; // erreur de lecture
    }
    uint8_t val = i2c.read();

    // Convertit l’octet DS4432 en -127..+127
    return byteToCmd(val);
}

} // namespace DS4432
