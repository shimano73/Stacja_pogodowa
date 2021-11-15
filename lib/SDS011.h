/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

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

#ifndef _sds011_h
#define _sds011_h

#include <DHT.h>
#include "SdsDustSensor.h"

#include "therm_hygro_meter.h"

namespace Supla {
namespace Sensor {
class SDS011: public ThermHygroMeter {
  public:
    SDS011(int rxPin, int txPin, int Period ); 
      

    double getTemp();
    double getHumi();   
    void iterateAlways();
    void onInit();	
    
    


    protected:
 	  int _rxPin;
	  int _txPin;
	  int _Period;
	  int _pm25;
	  int _pm10;
      double lastValidTemp;
      double lastValidHumi;
      int8_t retryCountTemp;
      int8_t retryCountHumi;
	  SdsDustSensor sds;
};

};  // namespace Sensor
};  // namespace Supla

#endif
