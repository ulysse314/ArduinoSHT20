#ifndef SHT20_h
#define SHT20_h

#include <Arduino.h>

class SHT20 {
public:
  enum class Waiting {
    NoTrigger,
    Temperature,
    Humidity,
  };

  SHT20(TwoWire *i2cBus = &Wire);

  bool triggerTemperatureMeasure();
  bool triggerHumidityMeasure();
  bool readValue();
  Waiting waitingFor() const { return _waiting; };
  unsigned long long measureDuration() const;
  unsigned long long delayUntilRead() const;
  float getTemperature() const { return _temperature; };
  float getHumidity() const { return _humidity; };

protected:
  enum class Command {
    TriggerTemperatureWithHold     = 0b11100011,
    TriggerHumidityWithHold        = 0b11100101,
    TriggerTemperatureWithoutHold  = 0b11110011,
    TriggerHumidityWithoutHold     = 0b11110101,
    WriteUserRegister              = 0b11100110,
    ReadUserRegister               = 0b11100110,
    SoftReset                      = 0b11111110,
  };

  bool writeRegister(Command command, uint8_t value);
  void setTemperature(uint16_t rawValue);
  void setHumidity(uint16_t rawValue);

  TwoWire *const _i2cBus;
  const uint8_t _i2cAddress;
  Waiting _waiting;
  unsigned long long _triggerDate;
  float _temperature;
  float _humidity;
};

#endif  // SHT20_h
