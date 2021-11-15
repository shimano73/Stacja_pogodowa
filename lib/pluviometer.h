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

#ifndef _Pluviometer_h
#define _Pluviometer_h

#include "supla/channel.h"
#include "supla/element.h"

extern "C" void serviceInterruptRain(void)  __attribute__ ((signal));

namespace Supla {
namespace Sensor {
class Pluviometer: public Element {
 public:
  Pluviometer(int pinRain, double rain_factory);  

  static double _currentRainCount;   
  friend void serviceInterruptRain();
  virtual double getValue() ;
  void setReset(bool value);
  void iterateAlways();
  void onInit();	


 protected:
	Channel *getChannel() {
		return &channel;
	}
	Channel channel;
	int _pinRain; 	
	bool reset;
	double _rain_factory;
	unsigned long lastReadTime;
};

};  // namespace Sensor
};  // namespace Supla

#endif

