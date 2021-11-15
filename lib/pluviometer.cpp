

/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "Arduino.h"

#include <Time.h>
#include "pluviometer.h"
#define RAIN_NOT_AVAILABLE -1

namespace Supla {
namespace Sensor {
	
unsigned long currentRainMin;
unsigned long lastRainTime;
double cos;


double Pluviometer::getValue(){
	
	return _currentRainCount;
}



void ICACHE_RAM_ATTR serviceInterruptRain()
{
  unsigned long currentTime=(unsigned long) (micros()-lastRainTime);

  lastRainTime=micros();
  if(currentTime>500)   // debounce
  {
     Pluviometer::_currentRainCount += cos;
	Serial.print("cos :"); Serial.print(Pluviometer::_currentRainCount);Serial.println("  +   ");
    if(currentTime<currentRainMin)
    {
     currentRainMin=currentTime;
    }
  } 
}


double Pluviometer::_currentRainCount = 0;

Pluviometer::Pluviometer(int pinRain, double rain_factory) 
{
	channel.setType(SUPLA_CHANNELTYPE_RAINSENSOR);
    channel.setDefault(SUPLA_CHANNELFNC_RAINSENSOR);
    channel.setNewValue(RAIN_NOT_AVAILABLE);

	_pinRain = pinRain;
	_rain_factory = rain_factory;	
	cos = _rain_factory;
}



 void Pluviometer::iterateAlways() {
    if (lastReadTime + 10000 < millis()) {
      lastReadTime = millis();
      channel.setNewValue(getValue());
    }
  }
  
  
  
 

void Pluviometer::setReset(bool value) {
if (value) _currentRainCount = 0;
}


  void Pluviometer::onInit() {
    
	pinMode(_pinRain, INPUT_PULLUP);	   
    digitalWrite(_pinRain, HIGH);   
    attachInterrupt(digitalPinToInterrupt(_pinRain), serviceInterruptRain, FALLING);
    channel.setNewValue(getValue());
	_rain_factory = cos;
  }



};  // namespace Sensor
};  // namespace Supla
