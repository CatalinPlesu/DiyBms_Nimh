#ifndef Steinhart_H // include guard
#define Steinhart_H

#include <Arduino.h>

class Steinhart {
   public:
      static int16_t ThermistorToCelcius(uint16_t BCOEFFICIENT, uint16_t RawADC, float ADCScaleMax);
      static uint16_t ThermistorToCelciusFloat(uint16_t BCOEFFICIENT, uint16_t RawADC, float ADCScaleMax);
      static uint16_t ThermistorToCelciusFloat2(uint16_t BCOEFFICIENT, uint16_t RawADC, float ADCScaleMax);
      static uint8_t TemperatureToByte(int16_t TempInCelcius);


};
#endif
