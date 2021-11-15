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

#ifndef _Anemometr_h
#define _Anemometr_h

#include "supla/channel.h"
#include "supla/element.h"



#define WIND_NOT_AVAILABLE -1
#define SAMPLETIME 5.0
extern "C" void serviceInterruptAnem(void)  __attribute__ ((signal));

namespace Supla {
namespace Sensor {
class Anemometr: public Element {
 public:
  Anemometr(int pinAnem, double wind_factory, int selectedMode);  

  float get_wind_gust();
  void reset_wind_gust();
  static unsigned long _shortestWindTime;
  static long _currentWindCount;     
  friend void serviceInterruptAnem();
  virtual double getValue() ;
  void iterateAlways();
  void onInit();	


 protected:
	Channel *getChannel() {
		return &channel;
	}
	Channel channel;
	unsigned long lastReadTime;
	double _wind_factory;
	int _pinAnem;   
	int _selectedMode;
	unsigned long _startSampleTime;    
	float _currentWindSpeed;    
	void startWindSample(float sampleTime);
	float get_current_wind_speed_when_sampling();
	float _sampleTime;
};

};  // namespace Sensor
};  // namespace Supla

#endif
