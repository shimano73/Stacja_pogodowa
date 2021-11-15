/*
  Copyright (C) krycha88 & shimano73
U23ZU2daU2
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
#include "SuplaDeviceGUI.h"

#ifdef SUPLA_PZEM_V_3
#include <supla/sensor/PzemV3.h>
#include <supla/sensor/three_phase_PzemV3.h>
#endif
#ifdef SUPLA_DEEP_SLEEP
#include <supla/control/deepSleep.h>
#endif
#include <supla/clock/clock.h>
#include <supla/sensor/percentage.h>
#include <supla/sensor/bh1750.h>
#include <supla/sensor/Anemometr.h>
#include <supla/sensor/pluviometer.h>
#include <supla/sensor/SDS011.h>
#include <ArduinoJson.h> // 6.9.0 or later

#ifdef ARDUINO_ARCH_ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


#include <supla/clock/clock.h>
Supla::Clock suplaClock;



bool only_once = 1;
unsigned long pm_lastmillis =0;

//#define DRD_TIMEOUT 5  // Number of seconds after reset during which a subseqent reset will be considered a double reset.
//#define DRD_ADDRESS 0  // RTC Memory Address for the DoubleResetDetector to use
// DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
//Luftdaten ******************************************************************************************************

bool LUFTDATEN_ON = false;

const char *luftdatenAPIHOST = "api.luftdaten.info";
const char *luftdatenAPIURL = "/v1/push-sensor-data/";
const uint16_t luftdatenAPIPort = 80;

const char *madavideAPIHOST = "api-rrd.madavi.de";
const char *madavideAPIURL = "/data.php";
const uint16_t madavideAPIPort = 80;
String  luftdatenChipId;


// AirMonitor ****************************************************************************************************
//wSArlHeQTOM5ayOsfoXRPrRdr8Vh3NtNQ54OBlmSnJ
//char *airMonitorServerName = "api.airmonitor.pl";
const uint16_t airMonitorPort = 5000;
//char LATITUDE[10]  = "50.084449";
//char LONGITUDE[10] = "18.832895";


// aqieco**********************************************************************************************************

bool DEBUG = true;
String aqiEcoChipId;
const char SOFTWAREVERSION[] = "2.7.3 build " __TIME__ " " __DATE__;
const char HARDWAREVERSION[] = "1.0 - ESP8266";
char THP_MODEL[255] = "BME280";
char DUST_MODEL[255] = "SDS011/21";
bool FREQUENTMEASUREMENT;

struct _send_data {
  double bme_temp;
  double bme_humi;
  double bme_press;
  double heca_temp;
  double heca_humi;
  int pm25;
  int pm10;
};

struct _send_data send_data;

#ifdef SUPLA_WEATHER_STATION 
  Supla::Sensor::Pluviometer *pluvio;
#endif

void sendDataToAqiEco(double currentTemperature, double currentPressure, double currentHumidity, int heater_temp, int heater_humi, int averagePM1, int averagePM25, int averagePM4, int averagePM10) {
  if (!(ConfigManager->get(KEY_AQI_ECO_STATE)->getValueInt())) {
    return;
  }
  Serial.print("Typ czujnika smogu : ");
  Serial.println(DUST_MODEL); 
  Serial.print(" Typ czujnika THP : ");
  Serial.println(THP_MODEL);
  StaticJsonDocument<768> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
  json["esp8266id"] = aqiEcoChipId;
  json["software_version"] = "Smogomierz_" + String(SOFTWAREVERSION);
  JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");

  if (!strcmp(DUST_MODEL, "PMS7003")) {
    JsonObject P0 = sensordatavalues.createNestedObject();
    P0["value_type"] = "PMS_P0";
    P0["value"] = averagePM1;
    JsonObject P1 = sensordatavalues.createNestedObject();
    P1["value_type"] = "PMS_P1";
    P1["value"] = averagePM10;
    JsonObject P2 = sensordatavalues.createNestedObject();
    P2["value_type"] = "PMS_P2";
    P2["value"] = averagePM25;
  } else if (!strcmp(DUST_MODEL, "SDS011/21")) {
    JsonObject P1 = sensordatavalues.createNestedObject();
    P1["value_type"] = "SDS_P1";
    P1["value"] = averagePM10;
    JsonObject P2 = sensordatavalues.createNestedObject();
    P2["value_type"] = "SDS_P2";
    P2["value"] = averagePM25;
    Serial.print("Przygotowanie danych SDS011");
  } else if (!strcmp(DUST_MODEL, "HPMA115S0")) {
    JsonObject P1 = sensordatavalues.createNestedObject();
    P1["value_type"] = "HPM_P1";
    P1["value"] = averagePM10;
    JsonObject P2 = sensordatavalues.createNestedObject();
    P2["value_type"] = "HPM_P2";
    P2["value"] = averagePM25;
  }

  if (!strcmp(THP_MODEL, "BME280")) {
    JsonObject temperature = sensordatavalues.createNestedObject();
    temperature["value_type"] = "BME280_temperature";
    temperature["value"] = String(currentTemperature);
    JsonObject humidity = sensordatavalues.createNestedObject();
    humidity["value_type"] = "BME280_humidity";
    humidity["value"] = String(currentHumidity);
    JsonObject pressure = sensordatavalues.createNestedObject();
    pressure["value_type"] = "BME280_pressure";
    pressure["value"] = String(currentPressure * 100); //hPa -> Pa
     Serial.print("Przygotowanie danych BME280");
   
  } else if (!strcmp(THP_MODEL, "BMP280")) {
    JsonObject temperature = sensordatavalues.createNestedObject();
    temperature["value_type"] = "BMP280_temperature";
    temperature["value"] = String(currentTemperature);
    JsonObject pressure = sensordatavalues.createNestedObject();
    pressure["value_type"] = "BMP280_pressure";
    pressure["value"] = String(currentPressure * 100); //hPa -> Pa
  } else if (!strcmp(THP_MODEL, "HTU21")) {
    JsonObject temperature = sensordatavalues.createNestedObject();
    temperature["value_type"] = "HTU21_temperature";
    temperature["value"] = String(currentTemperature);
    JsonObject humidity = sensordatavalues.createNestedObject();
    humidity["value_type"] = "HTU21_humidity";
    humidity["value"] = String(currentHumidity);
  } else if (!strcmp(THP_MODEL, "DHT22")) {
    JsonObject temperature = sensordatavalues.createNestedObject();
    temperature["value_type"] = "DHT22_temperature";
    temperature["value"] = String(currentTemperature);
    JsonObject humidity = sensordatavalues.createNestedObject();
    humidity["value_type"] = "DHT22_humidity";
    humidity["value"] = String(currentHumidity);
  } else if (!strcmp(THP_MODEL, "SHT1x")) {
    JsonObject temperature = sensordatavalues.createNestedObject();
    temperature["value_type"] = "SHT1x_temperature";
    temperature["value"] = String(currentTemperature);
    JsonObject humidity = sensordatavalues.createNestedObject();
    humidity["value_type"] = "SHT1x_humidity";
    humidity["value"] = String(currentHumidity);
  }

#ifdef SUPLA_SHT3x 
    JsonObject heater_temperature = sensordatavalues.createNestedObject();
    heater_temperature["value_type"] = "heater_temperature";
    heater_temperature["value"] = heater_temp;
    JsonObject heater_humidity = sensordatavalues.createNestedObject();
    heater_humidity["value_type"] = "heater_humidity";
    heater_humidity["value"] = heater_humi;
#endif  
  WiFiClient client;

  Serial.print("\nconnecting to ");
  Serial.println(ConfigManager->get(KEY_AQI_ECO_HOST)->getValue());

  if (!client.connect(ConfigManager->get(KEY_AQI_ECO_HOST)->getValue(), 80)) {
    Serial.println("connection failed");
  
    ConfigManager->set(KEY_AQI_ECO_STATUS, "Not connected");
    delay(1000);
    return;
  }
  else     ConfigManager->set(KEY_AQI_ECO_STATUS, "Connected");
  delay(100);

  client.print("POST ");
  client.print(ConfigManager->get(KEY_AQI_ECO_PATH)->getValue());
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(ConfigManager->get(KEY_AQI_ECO_HOST)->getValue());
  client.println("Content-Type: application/json");
  client.println("X-PIN: 1");
  client.print("X-Sensor: smogomierz-");
  client.println(aqiEcoChipId);
  client.print("Content-Length: ");
  client.println(measureJson(json));
  client.println("Connection: close");
  client.println();
  serializeJson(json, client);

  String line = client.readStringUntil('\r');
  Serial.println(line);
  // TODO: Support wrong error (!= 200)

  if (DEBUG) {
    Serial.println();
    Serial.print("POST ");
    Serial.print(ConfigManager->get(KEY_AQI_ECO_PATH)->getValue());
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(ConfigManager->get(KEY_AQI_ECO_HOST)->getValue());
    Serial.println("Content-Type: application/json");
    Serial.println("X-PIN: 1");
    Serial.print("X-Sensor: smogomierz-");
    Serial.println(aqiEcoChipId);
    Serial.print("Content-Length: ");
    Serial.println(measureJson(json));
    Serial.println("Connection: close");
    Serial.println();
    serializeJsonPretty(json, Serial);
    Serial.println("\n");
    Serial.println(line);
  }
  client.stop();
}


void sendJson(JsonObject& json) {
    WiFiClient client;
    Serial.print("\nconnecting to ");
    Serial.print(ConfigManager->get(KEY_AIR_MONITOR_SERVER)->getValue());
    Serial.print(" port = ");
    Serial.println(airMonitorPort);
    if (!client.connect(ConfigManager->get(KEY_AIR_MONITOR_SERVER)->getValue(), airMonitorPort)) {
        ConfigManager->set(KEY_AIR_MONITOR_STATUS,"Not connected");
        Serial.println("connection failed");
        Serial.println("wait ...\n");
        delay(1000);
        //return;
    } else ConfigManager->set(KEY_AIR_MONITOR_STATUS,"Connected");

    delay(100); 

    client.println("POST /api HTTP/1.1");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(measureJson(json));
    client.println();
    serializeJson(json, client);

    String line = client.readStringUntil('\r');
    // TODO: Support wrong error (!= 200)

    if (DEBUG) {
        Serial.print("Length:");
    Serial.println(measureJson(json));
    serializeJsonPretty(json, Serial);
        Serial.println(line);
    }
    client.stop();
}

void sendDUSTData(int averagePM1, int averagePM25, int averagePM10) {
  if (strcmp(DUST_MODEL, "Non")) {
  StaticJsonDocument<400> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
    json["lat"] = String(ConfigManager->get(KEY_AIR_MONITOR_LATITUDE)->getValue());
    json["long"] = String(ConfigManager->get(KEY_AIR_MONITOR_LONGITUDE)->getValue());
    json["pm1"] = averagePM1;
    json["pm25"] = averagePM25;
    json["pm10"] = averagePM10;
  if (!strcmp(DUST_MODEL, "PMS7003")) {
    json["sensor"] = "PMS7003";
  }
  if (!strcmp(DUST_MODEL, "HPMA115S0")) {
    json["sensor"] = "HPMA115S0";
  }
  if (!strcmp(DUST_MODEL, "SDS011/21")) {
    json["sensor"] = "SDS011";
  }
  if (!strcmp(DUST_MODEL, "SPS30")) {
    json["sensor"] = "SPS30";
  }
    sendJson(json);
  }
}

void sendTHPData(double currentTemperature, double currentPressure, double currentHumidity) {
  if (strcmp(THP_MODEL, "Non")) {
  StaticJsonDocument<400> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
    json["lat"] = String(ConfigManager->get(KEY_AIR_MONITOR_LATITUDE)->getValue());
    json["long"] = String(ConfigManager->get(KEY_AIR_MONITOR_LONGITUDE)->getValue());
  if (!strcmp(THP_MODEL, "BME280")) {
      json["pressure"] = currentPressure;
      json["temperature"] = currentTemperature;
      json["humidity"] = currentHumidity;
    json["sensor"] = "BME280";
  } else if (!strcmp(THP_MODEL, "BMP280")) {
      json["pressure"] = currentPressure;
      json["temperature"] = currentTemperature;
    json["sensor"] = "BMP280";
  } else if (!strcmp(THP_MODEL, "HTU21")) {
      json["temperature"] = currentTemperature;
      json["humidity"] = currentHumidity;
    json["sensor"] = "HTU21";
  } else if (!strcmp(THP_MODEL, "DHT22")) {
      json["temperature"] = currentTemperature;
      json["humidity"] = currentHumidity;
    json["sensor"] = "DHT22";
  } else if (!strcmp(THP_MODEL, "SHT1x")) {
      json["temperature"] = currentTemperature;
      json["humidity"] = currentHumidity;
    json["sensor"] = "SHT1x";
  }
    sendJson(json);
  }
}

void sendDataToAirMonitor(float currentTemperature, float currentPressure, float currentHumidity, int averagePM1, int averagePM25, int averagePM4, int averagePM10) {
    if (!(ConfigManager->get(KEY_AIR_MONITOR_STATE)->getValueInt())) {
        return;
    }

    sendDUSTData(averagePM1, averagePM25, averagePM10);
    sendTHPData(currentTemperature, currentPressure, currentHumidity);
}



/*
void sendDUSTLuftdatenJson(JsonObject& json) {
  WiFiClient client;
  Serial.print("\nconnecting to ");
  Serial.println(luftdatenAPIHOST);

  if (!client.connect(luftdatenAPIHOST, luftdatenAPIPort)) {
    Serial.println("connection failed");
    delay(1000);
    return;
  }
  delay(100);
  client.println("POST " + String(luftdatenAPIURL) + " HTTP/1.1");
  client.println("Host: " + String(luftdatenAPIHOST));
  client.println("Content-Type: application/json");
  client.println("X-PIN: 1");
  client.println("X-Sensor: smogomierz-" + luftdatenChipId);
  client.print("Content-Length: ");
  client.println(measureJson(json));
  client.println("Connection: close");
  client.println();
  serializeJson(json, client);

  String line = client.readStringUntil('\r');
  // TODO: Support wrong error (!= 200)

  if (DEBUG) {
    Serial.println();
    Serial.println("POST " + String(luftdatenAPIURL) + " HTTP/1.1");
    Serial.println("Host: " + String(luftdatenAPIHOST));
    Serial.println("Content-Type: application/json");
    Serial.println("X-PIN: 1");
    Serial.println("X-Sensor: smogomierz-" + luftdatenChipId);
    Serial.print("Content-Length: ");
    Serial.println(measureJson(json));
    Serial.println("Connection: close");
    Serial.println();
    serializeJsonPretty(json, Serial);
    Serial.println("\n");
    Serial.println(line);
  }
  client.stop();
}

void sendTHPLuftdatenJson(JsonObject& json) {
  WiFiClient client;
  Serial.print("\nconnecting to ");
  Serial.println(luftdatenAPIHOST);

  if (!client.connect(luftdatenAPIHOST, luftdatenAPIPort)) {
    Serial.println("connection failed");
    delay(1000);
    return;
  }
  delay(100);
  client.println("POST " + String(luftdatenAPIURL) + " HTTP/1.1");
  client.println("Host: " + String(luftdatenAPIHOST));
  client.println("Content-Type: application/json");
  if (!strcmp(THP_MODEL, "BME280")) {
    client.println("X-PIN: 11");
  } else if (!strcmp(THP_MODEL, "BMP280")) {
    client.println("X-PIN: 3");
  } else if (!strcmp(THP_MODEL, "HTU21")) {
    client.println("X-PIN: 7");
  } else if (!strcmp(THP_MODEL, "DHT22")) {
    client.println("X-PIN: 7");
  } else if (!strcmp(THP_MODEL, "SHT1x")) {
    client.println("X-PIN: 12");
  }
  client.println("X-Sensor: smogomierz-" + luftdatenChipId);
  client.print("Content-Length: ");
  client.println(measureJson(json));
  client.println("Connection: close");
  client.println();

  serializeJson(json, client);

  String line = client.readStringUntil('\r');
  // TODO: Support wrong error (!= 200)

  if (DEBUG) {
    Serial.println();
    Serial.println("POST " + String(luftdatenAPIURL) + " HTTP/1.1");
    Serial.println("Host: " + String(luftdatenAPIHOST));
    Serial.println("Content-Type: application/json");
    if (!strcmp(THP_MODEL, "BME280")) {
      Serial.println("X-PIN: 11");
    } else if (!strcmp(THP_MODEL, "BMP280")) {
      Serial.println("X-PIN: 3");
    } else if (!strcmp(THP_MODEL, "HTU21")) {
      Serial.println("X-PIN: 7");
    } else if (!strcmp(THP_MODEL, "DHT22")) {
      Serial.println("X-PIN: 7");
    } else if (!strcmp(THP_MODEL, "SHT1x")) {
      Serial.println("X-PIN: 12");
    }
    Serial.println("X-Sensor: smogomierz-" + luftdatenChipId);
    Serial.print("Content-Length: ");
    Serial.println(measureJson(json));
    Serial.println("Connection: close");
    Serial.println();
    serializeJsonPretty(json, Serial);
    Serial.println("\n");
    Serial.println(line);
  }
  client.stop();
}

void sendDUSTDatatoLuftdaten(int averagePM1, int averagePM25, int averagePM10) {
  if (strcmp(DUST_MODEL, "Non")) {
    StaticJsonDocument<600> jsonBuffer;
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["software_version"] = "Smogomierz_" + String(SOFTWAREVERSION);
    JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");
    if (!strcmp(DUST_MODEL, "PMS7003")) {
      JsonObject P0 = sensordatavalues.createNestedObject();
      P0["value_type"] = "P0";
      P0["value"] = averagePM1;
      JsonObject P1 = sensordatavalues.createNestedObject();
      P1["value_type"] = "P1";
      P1["value"] = averagePM10;
      JsonObject P2 = sensordatavalues.createNestedObject();
      P2["value_type"] = "P2";
      P2["value"] = averagePM25;
    } else {
      JsonObject P1 = sensordatavalues.createNestedObject();
      P1["value_type"] = "P1";
      P1["value"] = averagePM10;
      JsonObject P2 = sensordatavalues.createNestedObject();
      P2["value_type"] = "P2";
      P2["value"] = averagePM25;
    }
    sendDUSTLuftdatenJson(json);
  }
}

void sendTHPDatatoLuftdaten(double currentTemperature, double currentPressure, double currentHumidity) {
  if (strcmp(THP_MODEL, "Non")) {
    StaticJsonDocument<600> jsonBuffer;
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["software_version"] = "Smogomierz_" + String(SOFTWAREVERSION);
    JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");
    if (!strcmp(THP_MODEL, "BME280")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
      JsonObject pressure = sensordatavalues.createNestedObject();
      pressure["value_type"] = "pressure";
      pressure["value"] = String(currentPressure * 100); //hPa -> Pa
    } else if (!strcmp(THP_MODEL, "BMP280")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject pressure = sensordatavalues.createNestedObject();
      pressure["value_type"] = "pressure";
      pressure["value"] = String(currentPressure * 100); //hPa -> Pa
    } else if (!strcmp(THP_MODEL, "HTU21")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    } else if (!strcmp(THP_MODEL, "DHT22")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    } else if (!strcmp(THP_MODEL, "SHT1x")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    }
    sendTHPLuftdatenJson(json);
  }
}


void sendDUSTMadavideJson(JsonObject& json) {
  WiFiClient client;
  Serial.print("\nconnecting to ");
  Serial.println(madavideAPIHOST);

  if (!client.connect(madavideAPIHOST, madavideAPIPort)) {
    Serial.println("connection failed");
    delay(1000);
    return;
  }
  delay(100);
  client.println("POST " + String(madavideAPIURL) + " HTTP/1.1");
  client.println("Host: " + String(madavideAPIHOST));
  client.println("Content-Type: application/json");
  client.println("X-PIN: 1");
  client.println("X-Sensor: smogomierz-" + luftdatenChipId);
  client.print("Content-Length: ");
  client.println(measureJson(json));
  client.println("Connection: close");
  client.println();

  serializeJson(json, client);

  String line = client.readStringUntil('\r');
  // TODO: Support wrong error (!= 200)

  if (DEBUG) {
    Serial.println();
    Serial.println("POST " + String(madavideAPIURL) + " HTTP/1.1");
    Serial.println("Host: " + String(madavideAPIHOST));
    Serial.println("Content-Type: application/json");
    Serial.println("X-PIN: 1");
    Serial.println("X-Sensor: smogomierz-" + luftdatenChipId);
    Serial.print("Content-Length: ");
    Serial.println(measureJson(json));
    Serial.println("Connection: close");
    Serial.println();
    serializeJsonPretty(json, Serial);
    Serial.println("\n");
    Serial.println(line);
  }
  client.stop();
}

void sendTHPMadavideJson(JsonObject& json) {
  WiFiClient client;
  Serial.print("\nconnecting to ");
  Serial.println(madavideAPIHOST);

  if (!client.connect(madavideAPIHOST, madavideAPIPort)) {
    Serial.println("connection failed");
    delay(1000);
    return;
  }
  delay(100);
  client.println("POST " + String(madavideAPIURL) + " HTTP/1.1");
  client.println("Host: " + String(madavideAPIHOST));
  client.println("Content-Type: application/json");
  if (!strcmp(THP_MODEL, "BME280")) {
    client.println("X-PIN: 11");
  } else if (!strcmp(THP_MODEL, "BMP280")) {
    client.println("X-PIN: 3");
  } else if (!strcmp(THP_MODEL, "HTU21")) {
    client.println("X-PIN: 7");
  } else if (!strcmp(THP_MODEL, "DHT22")) {
    client.println("X-PIN: 7");
  } else if (!strcmp(THP_MODEL, "SHT1x")) {
    client.println("X-PIN: 12");
  }
  client.println("X-Sensor: smogomierz-" + luftdatenChipId);
  client.print("Content-Length: ");
  client.println(measureJson(json));
  client.println("Connection: close");
  client.println();

  serializeJson(json, client);

  String line = client.readStringUntil('\r');
  // TODO: Support wrong error (!= 200)

  if (DEBUG) {
    Serial.println();
    Serial.println("POST " + String(madavideAPIURL) + " HTTP/1.1");
    Serial.println("Host: " + String(madavideAPIHOST));
    Serial.println("Content-Type: application/json");
    if (!strcmp(THP_MODEL, "BME280")) {
      Serial.println("X-PIN: 11");
    } else if (!strcmp(THP_MODEL, "BMP280")) {
      Serial.println("X-PIN: 3");
    } else if (!strcmp(THP_MODEL, "HTU21")) {
      Serial.println("X-PIN: 7");
    } else if (!strcmp(THP_MODEL, "DHT22")) {
      Serial.println("X-PIN: 7");
    } else if (!strcmp(THP_MODEL, "SHT1x")) {
      Serial.println("X-PIN: 12");
    }
    Serial.println("X-Sensor: smogomierz-" + luftdatenChipId);
    Serial.print("Content-Length: ");
    Serial.println(measureJson(json));
    Serial.println("Connection: close");
    Serial.println();
    serializeJsonPretty(json, Serial);
    Serial.println("\n");
    Serial.println(line);
  }
  client.stop();
}

void sendDUSTDatatoMadavide(int averagePM1, int averagePM25, int averagePM10) {
  if (strcmp(DUST_MODEL, "Non")) {
    StaticJsonDocument<600> jsonBuffer;
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["software_version"] = "Smogomierz_" + String(SOFTWAREVERSION);
    JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");
    if (!strcmp(DUST_MODEL, "PMS7003")) {
      JsonObject P0 = sensordatavalues.createNestedObject();
      P0["value_type"] = "PMS_P0";
      P0["value"] = averagePM1;
      JsonObject P1 = sensordatavalues.createNestedObject();
      P1["value_type"] = "PMS_P1";
      P1["value"] = averagePM10;
      JsonObject P2 = sensordatavalues.createNestedObject();
      P2["value_type"] = "PMS_P2";
      P2["value"] = averagePM25;
    } else if (!strcmp(DUST_MODEL, "SDS011/21")) {
      JsonObject P1 = sensordatavalues.createNestedObject();
      P1["value_type"] = "SDS_P1";
      P1["value"] = averagePM10;
      JsonObject P2 = sensordatavalues.createNestedObject();
      P2["value_type"] = "SDS_P2";
      P2["value"] = averagePM25;
    } else if (!strcmp(DUST_MODEL, "HPMA115S0")) {
      JsonObject P1 = sensordatavalues.createNestedObject();
      P1["value_type"] = "HPM_P1";
      P1["value"] = averagePM10;
      JsonObject P2 = sensordatavalues.createNestedObject();
      P2["value_type"] = "HPM_P2";
      P2["value"] = averagePM25;
    }
    sendDUSTMadavideJson(json);
  }
}

void sendTHPDatatoMadavide(float currentTemperature, float currentPressure, float currentHumidity) {
  if (strcmp(THP_MODEL, "Non")) {
    StaticJsonDocument<600> jsonBuffer;
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["software_version"] = "Smogomierz_" + String(SOFTWAREVERSION);
    JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");
    if (!strcmp(THP_MODEL, "BME280")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
      JsonObject pressure = sensordatavalues.createNestedObject();
      pressure["value_type"] = "pressure";
      pressure["value"] = String(currentPressure * 100); //hPa -> Pa
    } else if (!strcmp(THP_MODEL, "BMP280")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject pressure = sensordatavalues.createNestedObject();
      pressure["value_type"] = "pressure";
      pressure["value"] = String(currentPressure * 100); //hPa -> Pa
    } else if (!strcmp(THP_MODEL, "HTU21")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    } else if (!strcmp(THP_MODEL, "DHT22")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    } else if (!strcmp(THP_MODEL, "SHT1x")) {
      JsonObject temperature = sensordatavalues.createNestedObject();
      temperature["value_type"] = "temperature";
      temperature["value"] = String(currentTemperature);
      JsonObject humidity = sensordatavalues.createNestedObject();
      humidity["value_type"] = "humidity";
      humidity["value"] = String(currentHumidity);
    }
    sendTHPMadavideJson(json);
  }
}



void sendDataToLuftdaten(float currentTemperature, float currentPressure, float currentHumidity, int heater_temp, int heater_humi, int averagePM1, int averagePM25, int averagePM4, int averagePM10) {
  if (!(LUFTDATEN_ON)) {
    return;
  }  
  sendDUSTDatatoLuftdaten(averagePM1, averagePM25, averagePM10);
  sendTHPDatatoLuftdaten(currentTemperature, currentPressure, currentHumidity);
  delay(10);
  sendDUSTDatatoMadavide(averagePM1, averagePM25, averagePM10);
  sendTHPDatatoMadavide(currentTemperature, currentPressure, currentHumidity);
}

*/
void odczyt(){
  
    for (auto element = Supla::Element::begin(); element != nullptr; element = element->next()) { // iterujemy po wszystkich elementach dodanych do programu
    if (element->getChannel()) { // sprawdzamy, czy do elementu jest przypisany kanał. Niektóre elementy (np. button) nie posiadają kanału.
      auto channel = element->getChannel();
      switch (channel->getChannelNumber()) {
        //Serial.print("ChannelNumber :");Serial.println(channel->getChannelNumber());
        case 0 : 
              send_data.bme_temp = channel->getValueDoubleFirst();
              Serial.print("Wartość BME temp:");Serial.println(send_data.bme_temp);
              send_data.bme_humi = channel->getValueDoubleSecond();
              Serial.print("Wartość BME humi:");Serial.println(send_data.bme_humi);
        break;
        case 1:
              send_data.bme_press = channel->getValueDouble();
              Serial.print("Wartość BME press:");Serial.println(send_data.bme_press);
        break;
        case 2:
              send_data.heca_temp = channel->getValueDoubleFirst();
              Serial.print("Wartość HECA temp:");Serial.println(send_data.heca_temp);
              send_data.heca_humi = channel->getValueDoubleSecond();
              Serial.print("Wartość HECA humi:");Serial.println(send_data.heca_humi);
        break;
        case 6:
              send_data.pm25 = channel->getValueDoubleFirst();
              Serial.print("Wartość PM2.5:");Serial.println(send_data.pm25);
              send_data.pm10 = channel->getValueDoubleSecond();
              Serial.print("Wartość PM10:");Serial.println(send_data.pm10);
        
        break;
        }
        
      }
      if (element->getSecondaryChannel()) {
        auto channel = element->getSecondaryChannel();
        if (channel->getChannelNumber() == 1) {      
         send_data.bme_press = channel->getValueDouble();
         Serial.print("Wartość BME press:");Serial.println(send_data.bme_press);        
      }
    }
    }
  
  
}



void setup() {
  uint8_t nr, gpio;
  DEBUG = true;
  Serial.begin(74880);

#ifdef ARDUINO_ARCH_ESP8266
  ESP.wdtDisable();
#endif

  delay(1000);

  ConfigManager = new SuplaConfigManager();
  ConfigESP = new SuplaConfigESP();

  // if (drd.detectDoubleReset()) {
  //   drd.stop();
  //   ConfigESP->factoryReset();
  // }

#ifdef SUPLA_WEATHER_STATION
     
#endif
 

#if defined(SUPLA_RELAY) || defined(SUPLA_ROLLERSHUTTER)
  uint8_t rollershutters = ConfigManager->get(KEY_MAX_ROLLERSHUTTER)->getValueInt();

  for (nr = 1; nr <= ConfigManager->get(KEY_MAX_RELAY)->getValueInt(); nr++) {
    if (ConfigESP->getGpio(nr, FUNCTION_RELAY) != OFF_GPIO) {
      if (rollershutters > 0) {
#ifdef SUPLA_ROLLERSHUTTER
        Supla::GUI::addRolleShutter(nr);
#endif
        rollershutters--;
        nr++;
      }
      else {
#ifdef SUPLA_RELAY
        Supla::GUI::addRelayButton(nr);
#endif
      }
    }
  }
#endif

#ifdef SUPLA_LIMIT_SWITCH
  for (nr = 1; nr <= ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt(); nr++) {
    if (ConfigESP->getGpio(nr, FUNCTION_LIMIT_SWITCH) != OFF_GPIO) {
      new Supla::Sensor::Binary(ConfigESP->getGpio(nr, FUNCTION_LIMIT_SWITCH), true);
    }
  }
#endif

#ifdef SUPLA_CONFIG
  Supla::GUI::addConfigESP(ConfigESP->getGpio(FUNCTION_CFG_BUTTON), ConfigESP->getGpio(FUNCTION_CFG_LED));
#endif

#ifdef SUPLA_DS18B20
  if (ConfigESP->getGpio(FUNCTION_DS18B20) != OFF_GPIO) {
    Supla::GUI::addDS18B20MultiThermometer(ConfigESP->getGpio(FUNCTION_DS18B20));
  }
#endif

#ifdef SUPLA_DHT11
  for (nr = 1; nr <= ConfigManager->get(KEY_MAX_DHT11)->getValueInt(); nr++) {
    if (ConfigESP->getGpio(nr, FUNCTION_DHT11) != OFF_GPIO) {
      auto dht11 = new Supla::Sensor::DHT(ConfigESP->getGpio(nr, FUNCTION_DHT11), DHT11);

      if (nr == 1) {
        Supla::GUI::addConditionsTurnON(SENSOR_DHT11, dht11);
        Supla::GUI::addConditionsTurnOFF(SENSOR_DHT11, dht11);
      }
    }
  }
#endif

#ifdef SUPLA_DHT22
  for (nr = 1; nr <= ConfigManager->get(KEY_MAX_DHT22)->getValueInt(); nr++) {
    if (ConfigESP->getGpio(nr, FUNCTION_DHT22) != OFF_GPIO) {
      auto dht22 = new Supla::Sensor::DHT(ConfigESP->getGpio(nr, FUNCTION_DHT22), DHT22);

      if (nr == 1) {
        Supla::GUI::addConditionsTurnON(SENSOR_DHT22, dht22);
        Supla::GUI::addConditionsTurnOFF(SENSOR_DHT22, dht22);
      }
    }
  }
#endif

#ifdef SUPLA_SI7021_SONOFF
  if (ConfigESP->getGpio(FUNCTION_SI7021_SONOFF) != OFF_GPIO) {
    auto si7021sonoff = new Supla::Sensor::Si7021Sonoff(ConfigESP->getGpio(FUNCTION_SI7021_SONOFF));
    Supla::GUI::addConditionsTurnON(SENSOR_SI7021_SONOFF, si7021sonoff);
    Supla::GUI::addConditionsTurnOFF(SENSOR_SI7021_SONOFF, si7021sonoff);
  }
#endif

#ifdef SUPLA_HC_SR04
  if (ConfigESP->getGpio(FUNCTION_TRIG) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_ECHO) != OFF_GPIO) {
    Supla::Sensor::HC_SR04 *hcsr04;
    if (ConfigManager->get(KEY_HC_SR04_MAX_SENSOR_READ)->getValueInt() > 0) {
      hcsr04 = new Supla::Sensor::HC_SR04(ConfigESP->getGpio(FUNCTION_TRIG), ConfigESP->getGpio(FUNCTION_ECHO), 0,
                                          ConfigManager->get(KEY_HC_SR04_MAX_SENSOR_READ)->getValueInt(),
                                          ConfigManager->get(KEY_HC_SR04_MAX_SENSOR_READ)->getValueInt(), 0);
    }
    else {
      hcsr04 = new Supla::Sensor::HC_SR04(ConfigESP->getGpio(FUNCTION_TRIG), ConfigESP->getGpio(FUNCTION_ECHO));
    }

    Supla::GUI::addConditionsTurnON(SENSOR_HC_SR04, hcsr04);
    Supla::GUI::addConditionsTurnOFF(SENSOR_HC_SR04, hcsr04);
  }
#endif

#ifdef SUPLA_MAX6675
  if (ConfigESP->getGpio(FUNCTION_CLK) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_CS) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_D0) != OFF_GPIO) {
    auto thermocouple =
        new Supla::Sensor::MAXThermocouple(ConfigESP->getGpio(FUNCTION_CLK), ConfigESP->getGpio(FUNCTION_CS), ConfigESP->getGpio(FUNCTION_D0));
    Supla::GUI::addConditionsTurnON(SENSOR_MAX6675, thermocouple);
    Supla::GUI::addConditionsTurnOFF(SENSOR_MAX6675, thermocouple);
  }
#endif

#ifdef SUPLA_NTC_10K
  if (ConfigESP->getGpio(FUNCTION_NTC_10K) != OFF_GPIO) {
    auto ntc10k = new Supla::Sensor::NTC10K(ConfigESP->getGpio(FUNCTION_NTC_10K));
    Supla::GUI::addConditionsTurnON(SENSOR_NTC_10K, ntc10k);
    Supla::GUI::addConditionsTurnOFF(SENSOR_NTC_10K, ntc10k);
  }
#endif

#ifdef SUPLA_MPX_5XXX
  if (ConfigESP->getGpio(FUNCTION_MPX_5XXX) != OFF_GPIO) {
    Supla::GUI::mpx = new Supla::Sensor::MPX_5XXX(ConfigESP->getGpio(FUNCTION_MPX_5XXX));
    Supla::GUI::addConditionsTurnON(SENSOR_MPX_5XXX, Supla::GUI::mpx);
    Supla::GUI::addConditionsTurnOFF(SENSOR_MPX_5XXX, Supla::GUI::mpx);
  }
#endif

#ifdef SUPLA_ANALOG_READING_MAP
  if (ConfigESP->getGpio(FUNCTION_ANALOG_READING) != OFF_GPIO) {
    Supla::GUI::analog = new Supla::Sensor::AnalogRedingMap(ConfigESP->getGpio(FUNCTION_ANALOG_READING));
    Supla::GUI::addConditionsTurnON(SENSOR_ANALOG_READING_MAP, Supla::GUI::analog);
    Supla::GUI::addConditionsTurnOFF(SENSOR_ANALOG_READING_MAP, Supla::GUI::analog);
  }
#endif

#ifdef SUPLA_RGBW
  for (nr = 1; nr <= ConfigManager->get(KEY_MAX_RGBW)->getValueInt(); nr++) {
    Supla::GUI::addRGBWLeds(nr);
  }
#endif

#ifdef SUPLA_IMPULSE_COUNTER
  if (ConfigManager->get(KEY_MAX_IMPULSE_COUNTER)->getValueInt() > 0) {
    for (nr = 1; nr <= ConfigManager->get(KEY_MAX_IMPULSE_COUNTER)->getValueInt(); nr++) {
      gpio = ConfigESP->getGpio(nr, FUNCTION_IMPULSE_COUNTER);
      if (gpio != OFF_GPIO) {
        Supla::GUI::addImpulseCounter(gpio, ConfigESP->getLevel(gpio), ConfigESP->getMemory(gpio),
                                      ConfigManager->get(KEY_IMPULSE_COUNTER_DEBOUNCE_TIMEOUT)->getValueInt());
      }
    }
  }

#endif

#ifdef SUPLA_HLW8012
  if (ConfigESP->getGpio(FUNCTION_CF) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_CF1) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_SEL) != OFF_GPIO) {
    Supla::GUI::addHLW8012(ConfigESP->getGpio(FUNCTION_CF), ConfigESP->getGpio(FUNCTION_CF1), ConfigESP->getGpio(FUNCTION_SEL));
  }
#endif

#ifdef SUPLA_PZEM_V_3
  int8_t pinRX1 = ConfigESP->getGpio(1, FUNCTION_PZEM_RX);
  int8_t pinTX1 = ConfigESP->getGpio(1, FUNCTION_PZEM_TX);
  int8_t pinRX2 = ConfigESP->getGpio(2, FUNCTION_PZEM_RX);
  int8_t pinTX2 = ConfigESP->getGpio(2, FUNCTION_PZEM_TX);
  int8_t pinRX3 = ConfigESP->getGpio(3, FUNCTION_PZEM_RX);
  int8_t pinTX3 = ConfigESP->getGpio(3, FUNCTION_PZEM_TX);

  if (pinRX1 != OFF_GPIO && pinTX1 != OFF_GPIO && pinRX2 != OFF_GPIO && pinTX2 != OFF_GPIO && pinRX3 != OFF_GPIO && pinTX3 != OFF_GPIO) {
    new Supla::Sensor::ThreePhasePZEMv3(pinRX1, pinTX1, pinRX2, pinTX2, pinRX3, pinTX3);
  }
  else if (pinRX1 != OFF_GPIO && pinTX1 != OFF_GPIO && pinTX2 != OFF_GPIO && pinTX3 != OFF_GPIO) {
    new Supla::Sensor::ThreePhasePZEMv3(pinRX1, pinTX1, pinRX1, pinTX2, pinRX1, pinTX3);
  }
  else if (pinRX1 != OFF_GPIO && pinTX1 != OFF_GPIO) {
    new Supla::Sensor::PZEMv3(pinRX1, pinTX1);
  }
#endif

#ifdef SUPLA_CSE7766
  if (ConfigESP->getGpio(FUNCTION_CSE7766_RX) != OFF_GPIO) {
    Supla::GUI::addCSE7766(ConfigESP->getGpio(FUNCTION_CSE7766_RX));
  }
#endif

#ifdef GUI_SENSOR_I2C
  if (ConfigESP->getGpio(FUNCTION_SDA) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_SCL) != OFF_GPIO) {
    Wire.begin(ConfigESP->getGpio(FUNCTION_SDA), ConfigESP->getGpio(FUNCTION_SCL));

#ifdef SUPLA_BME280
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_BME280).toInt()) {
      Supla::Sensor::BME280 *bme280;
      switch (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_BME280).toInt()) {
        case BMx280_ADDRESS_0X76:
          bme280 = new Supla::Sensor::BME280(0x76, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());

          Supla::GUI::addConditionsTurnON(SENSOR_BME280, bme280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BME280, bme280);
          break;
        case BMx280_ADDRESS_0X77:
          bme280 = new Supla::Sensor::BME280(0x77, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          Supla::GUI::addConditionsTurnON(SENSOR_BME280, bme280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BME280, bme280);
          break;
        case BMx280_ADDRESS_0X76_AND_0X77:
          bme280 = new Supla::Sensor::BME280(0x76, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          new Supla::Sensor::BME280(0x77, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());

          Supla::GUI::addConditionsTurnON(SENSOR_BME280, bme280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BME280, bme280);
          break;
      }
    }
#endif

#ifdef SUPLA_BMP280
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_BMP280).toInt()) {
      Supla::Sensor::BMP280 *bmp280;
      switch (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_BMP280).toInt()) {
        case BMx280_ADDRESS_0X76:
          bmp280 = new Supla::Sensor::BMP280(0x76, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          Supla::GUI::addConditionsTurnON(SENSOR_BMP280, bmp280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BMP280, bmp280);
          break;
        case BMx280_ADDRESS_0X77:
          bmp280 = new Supla::Sensor::BMP280(0x77, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          Supla::GUI::addConditionsTurnON(SENSOR_BMP280, bmp280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BMP280, bmp280);
          break;
        case BMx280_ADDRESS_0X76_AND_0X77:
          bmp280 = new Supla::Sensor::BMP280(0x76, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          new Supla::Sensor::BMP280(0x77, ConfigManager->get(KEY_ALTITUDE_BMX280)->getValueInt());
          Supla::GUI::addConditionsTurnON(SENSOR_BMP280, bmp280);
          Supla::GUI::addConditionsTurnOFF(SENSOR_BMP280, bmp280);
          break;
      }
    }
#endif

#ifdef SUPLA_SHT3x

    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_SHT3x).toInt()) {
      Supla::Sensor::SHT3x *sht3x;
      switch (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_SHT3x).toInt()) {
        case SHT3x_ADDRESS_0X44:
          sht3x = new Supla::Sensor::SHT3x(0x44);

          Supla::GUI::addConditionsTurnON(SENSOR_SHT3x, sht3x);
          Supla::GUI::addConditionsTurnOFF(SENSOR_SHT3x, sht3x);
          break;
        case SHT3x_ADDRESS_0X45:
          sht3x = new Supla::Sensor::SHT3x(0x45);

          Supla::GUI::addConditionsTurnON(SENSOR_SHT3x, sht3x);
          Supla::GUI::addConditionsTurnOFF(SENSOR_SHT3x, sht3x);
          break;
        case SHT3x_ADDRESS_0X44_AND_0X45:
          sht3x = new Supla::Sensor::SHT3x(0x44);
          new Supla::Sensor::SHT3x(0x45);

          Supla::GUI::addConditionsTurnON(SENSOR_SHT3x, sht3x);
          Supla::GUI::addConditionsTurnOFF(SENSOR_SHT3x, sht3x);
          break;
      }
    }
#endif

#ifdef SUPLA_BH1750
 if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_BH1750).toInt()) {
      Supla::Sensor::Sensor_BH1750 *bh1750;
      bh1750 = new Supla::Sensor::Sensor_BH1750();
      Supla::GUI::addConditionsTurnON(SENSOR_BH1750, bh1750);
      Supla::GUI::addConditionsTurnOFF(SENSOR_BH1750, bh1750);
 }
#endif

#ifdef SUPLA_SI7021
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_SI7021).toInt()) {
      auto si7021 = new Supla::Sensor::Si7021();
      Supla::GUI::addConditionsTurnON(SENSOR_SI7021, si7021);
      Supla::GUI::addConditionsTurnOFF(SENSOR_SI7021, si7021);
    }
#endif

#ifdef SUPLA_VL53L0X
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_VL53L0X).toInt()) {
      auto vl53l0x = new Supla::Sensor::VL_53L0X();
      Supla::GUI::addConditionsTurnON(SENSOR_VL53L0X, vl53l0x);
      Supla::GUI::addConditionsTurnOFF(SENSOR_VL53L0X, vl53l0x);
    }
#endif

#ifdef SUPLA_OLED
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_OLED).toInt()) {
      SuplaOled *oled = new SuplaOled();
      oled->addButtonOled(ConfigESP->getGpio(FUNCTION_CFG_BUTTON));
    }
#endif

#ifdef SUPLA_WEATHER_STATION


  if (ConfigESP->getGpio(FUNCTION_ANEMOMETR) != OFF_GPIO && ConfigManager->get(KEY_ANEMOMETR_FACTORY)->getValueDouble() > 0) {
       new Supla::Sensor::Anemometr(ConfigESP->getGpio(FUNCTION_ANEMOMETR),ConfigManager->get(KEY_ANEMOMETR_FACTORY)->getValueDouble(),1);
      
  }

  if (ConfigESP->getGpio(FUNCTION_PLUVIOMETER) != OFF_GPIO && ConfigManager->get(KEY_PLUVIOMETER_FACTORY)->getValueDouble() > 0) { 
   pluvio = new Supla::Sensor::Pluviometer(ConfigESP->getGpio(FUNCTION_PLUVIOMETER),ConfigManager->get(KEY_PLUVIOMETER_FACTORY)->getValueDouble());  
     
  }
  

  if  (ConfigESP->getGpio(FUNCTION_SMOG_SENSOR_RX) != OFF_GPIO && ConfigESP->getGpio(FUNCTION_SMOG_SENSOR_TX) != OFF_GPIO && ConfigManager->get(KEY_SMOG_SENSOR_PERIOD)->getValueInt() > 0) { //dupa4
      new Supla::Sensor::SDS011(ConfigESP->getGpio(FUNCTION_SMOG_SENSOR_RX),ConfigESP->getGpio(FUNCTION_SMOG_SENSOR_TX),ConfigManager->get(KEY_SMOG_SENSOR_PERIOD)->getValueInt());  
      aqiEcoChipId = String(ESP.getChipId());
      
  }  

  if (ConfigManager->get(KEY_AQI_ECO_STATE)->getValueInt() != 0) {
                // sendDataToAqiEco(bme_channel.temp,bme_channel.pressure,bme_channel.humidity,0,pomiar25_int,0,pomiar10_int);

  }

#endif  //SUPLA_WEATHER_STATION

#ifdef SUPLA_MCP23017
    if (ConfigManager->get(KEY_ACTIVE_SENSOR)->getElement(SENSOR_I2C_MCP23017).toInt()) {
      Supla::Control::MCP_23017 *mcp = new Supla::Control::MCP_23017();

      for (nr = 1; nr <= ConfigManager->get(KEY_MAX_BUTTON)->getValueInt(); nr++) {
        gpio = ConfigESP->getGpio(nr, FUNCTION_BUTTON);
        if (gpio != OFF_GPIO)
          mcp->setPullup(gpio, ConfigESP->getPullUp(gpio), false);
      }

#ifdef ARDUINO_ARCH_ESP8266
      Wire.setClock(400000);
#elif ARDUINO_ARCH_ESP32
      Wire.setClock(100000);
#endif
    }
#endif
  }
#endif

#ifdef DEBUG_MODE
  new Supla::Sensor::EspFreeHeap();
#endif

#ifdef SUPLA_DEEP_SLEEP
  if (ConfigManager->get(KEY_DEEP_SLEEP_TIME)->getValueInt() > 0) {
    new Supla::Control::DeepSleep(ConfigManager->get(KEY_DEEP_SLEEP_TIME)->getValueInt() * 60, 30);
  }
#endif

  Supla::GUI::begin();

#ifdef SUPLA_MPX_5XXX
  if (ConfigESP->getGpio(FUNCTION_MPX_5XXX) != OFF_GPIO) {
    Supla::Sensor::Percentage *mpxPercent;

    if (Supla::GUI::mpx->getThankHeight() != 0) {
      mpxPercent = new Supla::Sensor::Percentage(Supla::GUI::mpx, 0, Supla::GUI::mpx->getThankHeight() * 0.01);
    }
    else {
      mpxPercent = new Supla::Sensor::Percentage(Supla::GUI::mpx, 0, 100);
    }

    Supla::GUI::addConditionsTurnON(SENSOR_MPX_5XXX_PERCENT, mpxPercent);
    Supla::GUI::addConditionsTurnOFF(SENSOR_MPX_5XXX_PERCENT, mpxPercent);
  }
#endif

  Supla::GUI::addCorrectionSensor();

#ifdef ARDUINO_ARCH_ESP8266
  // https://github.com/esp8266/Arduino/issues/2070#issuecomment-258660760
  wifi_set_sleep_type(NONE_SLEEP_T);

  ESP.wdtEnable(WDTO_120MS);
#endif

 SuplaDevice.addClock(new Supla::Clock); 
}

void loop() {
  SuplaDevice.iterate();
  
 
  delay(25);
 #ifdef SUPLA_WEATHER_STATION 
      if ((ConfigManager->get(KEY_PLUVIOMETER_TIME_H)->getValueInt() == suplaClock.getHour()) and (suplaClock.getMin() == 0) and only_once) {
          pluvio->setReset(1);
          only_once = 0;
      }   
      if ((ConfigManager->get(KEY_PLUVIOMETER_TIME_H)->getValueInt() == suplaClock.getHour()) and (suplaClock.getMin() == 1)) {
        only_once = 1;
      }
      
      if (millis() - pm_lastmillis >= 60000) {
        pm_lastmillis = millis();  
        odczyt();
        if (ConfigManager->get(KEY_AQI_ECO_STATE)->getValueInt() != 0)
          sendDataToAqiEco(send_data.bme_temp,send_data.bme_press,send_data.bme_humi,send_data.heca_temp,send_data.heca_humi, 0,send_data.pm25,0,send_data.pm10);
       
      //  if (ConfigManager->get(KEY_AIR_MONITOR_STATE)->getValue() != 0)
      //    sendDataToAirMonitor(send_data.bme_temp,send_data.bme_press,send_data.bme_humi,0,send_data.pm25,0,send_data.pm10);
      //  sendDataToAirMonitor(bme_channel.temp, bme_channel.pressure,bme_channel.humidity,0,pomiar25_int,0,pomiar10_int);
      //  sendDataToLuftdaten(send_data.bme_temp,send_data.bme_press,send_data.bme_humi,send_data.heca_temp,send_data.heca_humi, 0,send_data.pm25,0,send_data.pm10);
 }
 #endif
  // drd.loop();
}
