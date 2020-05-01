#include "SHT20.h"

SHT20 *_sht20;

double temperature = 0;
double humidity = 0;
int counter = 0;

void setup() {
  Wire.begin();
  _sht20 = new SHT20();
  Particle.variable("temp", temperature);
  Particle.variable("humi", humidity);
  Particle.variable("counter", counter);
}

unsigned long long timer = 0;

void loop() {
  if ((unsigned long long)(millis() - timer) > 1000) {
    counter++;
    temperature = _sht20->getTemperature();
    humidity = _sht20->getHumidity();
    switch(_sht20->waitingFor()) {
    case SHT20::Waiting::NoTrigger:
      _sht20->triggerHumidityMeasure();
      timer = millis();
      break;
    case SHT20::Waiting::Temperature:
      if (_sht20->delayUntilRead() == 0) {
        _sht20->readValue();
        Particle.publish("Temp", String((double)_sht20->getTemperature()), PRIVATE);
        timer = millis();
      }
      break;
    case SHT20::Waiting::Humidity:
      if (_sht20->delayUntilRead() == 0) {
        _sht20->readValue();
        Particle.publish("Humi", String((double)_sht20->getHumidity()), PRIVATE);
        _sht20->triggerTemperatureMeasure();
        timer = millis();
      }
      break;
    }
  }
}
