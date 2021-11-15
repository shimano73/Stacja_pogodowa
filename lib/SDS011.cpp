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
#include "SDS011.h"
#include <DHT.h>
#include "SdsDustSensor.h"
//#include "therm_hygro_meter.h"

namespace Supla {
namespace Sensor {

SDS011::SDS011(int rxPin, int txPin, int Period ) : sds(rxPin, txPin){

	_rxPin = rxPin;
	_txPin = txPin;
	
	channel.setType(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR);
    channel.setDefault(SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
	sds.begin();
	Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
    Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
    Serial.println(sds.setCustomWorkingPeriod(_Period).toString()); // sensor sends data every 3 minutes
	//PmResult pm = sds.readPm();
}

double SDS011::getTemp() {
  double value = TEMPERATURE_NOT_AVAILABLE;
  value = _pm25;
  if (isnan(value)) {
	value = TEMPERATURE_NOT_AVAILABLE;
  }

  if (value == TEMPERATURE_NOT_AVAILABLE) {
	retryCountTemp++;
	if (retryCountTemp > 3) {
	  retryCountTemp = 0;
	} else {
	  value = lastValidTemp;
	}
  } else {
	retryCountTemp = 0;
  }
  lastValidTemp = value;

  return value;
}

double SDS011::getHumi() {
  double value = HUMIDITY_NOT_AVAILABLE;
  value = _pm10;

  if (isnan(value)) {
	value = HUMIDITY_NOT_AVAILABLE;
  }

  if (value == HUMIDITY_NOT_AVAILABLE) {
	retryCountHumi++;
	if (retryCountHumi > 3) {
	  retryCountHumi = 0;
	} else {
	  value = lastValidHumi;
	}
  } else {
	retryCountHumi = 0;
  }
  lastValidHumi = value;

  return value;
}

void SDS011::iterateAlways() {
  if (lastReadTime + 10000 < millis()) {
	lastReadTime = millis();
	channel.setNewValue(getTemp(), getHumi());
	
	PmResult pm = sds.readPm();
	if (pm.isOk()) {
		_pm25 = pm.pm25;
		_pm10 = pm.pm10;
	}
	
  }
}

void SDS011::onInit() {
 
  channel.setNewValue(getTemp(), getHumi());
  
}


};  // namespace Sensor
};  // namespace Supla
