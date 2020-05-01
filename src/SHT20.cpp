#include "SHT20.h"

#define SHIFTED_DIVISOR                       0x988000

enum class TemperatureMeasureDuration {
  Bits14 = 85,
  Bits13 = 43,
  Bits12 = 22,
  Bits11 = 11,
};

enum class HumidityMeasureDuration {
  Bits12 = 29,
  Bits11 = 15,
  Bits10 = 9,
  Bits8 = 4,
};

static byte checkCRC(uint16_t message_from_sensor, uint8_t check_value_from_sensor)
{
    uint32_t remainder = (uint32_t)message_from_sensor << 8;
    remainder |= check_value_from_sensor;
    uint32_t divsor = (uint32_t)SHIFTED_DIVISOR;
    for(int i = 0 ; i < 16 ; i++){
        if(remainder & (uint32_t)1 << (23 - i)){
            remainder ^= divsor;
        }
        divsor >>= 1;
    }
    return (byte)remainder;
}

SHT20::SHT20(TwoWire *i2cBus) :
    _i2cBus(i2cBus),
    _i2cAddress(0x40),
    _waiting(Waiting::NoTrigger),
    _triggerDate(0),
    _temperature(0),
    _humidity(0) {
}

bool SHT20::triggerTemperatureMeasure() {
  if (!writeRegister(Command::TriggerTemperatureWithoutHold, 0)) {
    return false;
  }
  _waiting = Waiting::Temperature;
  _triggerDate = millis();
  return true;
}

bool SHT20::triggerHumidityMeasure() {
  if (!writeRegister(Command::TriggerHumidityWithoutHold, 0)) {
    return false;
  }
  _waiting = Waiting::Humidity;
  _triggerDate = millis();
  return true;
}

bool SHT20::readValue() {
  uint8_t toRead = _i2cBus->requestFrom(_i2cAddress, 3);
  if (toRead != 3) {
    return false;
  }
  byte msb, lsb, checksum;
  msb = _i2cBus->read();
  lsb = _i2cBus->read();
  checksum = _i2cBus->read();
  uint16_t rawValue = ((uint16_t) msb << 8) | (uint16_t) lsb;
  bool validValue = checkCRC(rawValue, checksum) == 0;
  switch (_waiting) {
  case Waiting::NoTrigger:
    return false;
  case Waiting::Temperature:
    setTemperature(rawValue);
    break;
  case Waiting::Humidity:
    setHumidity(rawValue);
    break;
  }
  _waiting = Waiting::NoTrigger;
  return validValue;
}

unsigned long long SHT20::measureDuration() const {
  switch (_waiting) {
  case Waiting::NoTrigger:
    break;
  case Waiting::Temperature:
    return static_cast<unsigned long long>(TemperatureMeasureDuration::Bits14);
  case Waiting::Humidity:
    return static_cast<unsigned long long>(HumidityMeasureDuration::Bits12);
  }
  return 0;
}

unsigned long long SHT20::delayUntilRead() const {
  unsigned long long duration = measureDuration();
  unsigned long long waitedTime = (unsigned long long)(millis() - _triggerDate);
  if (waitedTime >= duration) {
    return 0;
  }
  return duration - waitedTime;
}

bool SHT20::writeRegister(Command command, uint8_t value) {
  _i2cBus->beginTransmission(_i2cAddress);
  _i2cBus->write((uint8_t)command);
  return _i2cBus->endTransmission() == 0;
}

void SHT20::setTemperature(uint16_t rawValue) {
  float tempTemperature = rawValue * (175.72 / 65536.0);
  _temperature = tempTemperature - 46.85;
}

void SHT20::setHumidity(uint16_t rawValue) {
  float tempRH = rawValue * (125.0 / 65536.0);
  _humidity = tempRH - 6.0;
}
