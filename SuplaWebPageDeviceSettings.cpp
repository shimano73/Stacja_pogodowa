/*
  Copyright (C) krycha88
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



#include "SuplaWebPageDeviceSettings.h"

void createWebPageDeviceSettings() {
  WebServer->httpServer->on(getURL(PATH_DEVICE_SETTINGS), [&]() {
    if (!WebServer->isLoggedIn()) {
      return;
    }

    if (WebServer->httpServer->method() == HTTP_GET)
      handleDeviceSettings();
    else
      handleDeviceSettingsSave();
  });
}

void handleDeviceSettings(int save) {
  WebServer->sendHeaderStart();

  webContentBuffer += SuplaSaveResult(save);
  webContentBuffer += SuplaJavaScript(PATH_DEVICE_SETTINGS);

#if (DEFAULT_TEMPLATE_BOARD == BOARD_OFF)
  addForm(webContentBuffer, F("post"), PATH_DEVICE_SETTINGS);
  addFormHeader(webContentBuffer, S_TEMPLATE_BOARD);
  uint8_t selected = ConfigManager->get(KEY_BOARD)->getValueInt();
  addListBox(webContentBuffer, INPUT_BOARD, S_TYPE, BOARD_P, MAX_MODULE, selected);
  addFormHeaderEnd(webContentBuffer);
  addButtonSubmit(webContentBuffer, S_SAVE);
  addFormEnd(webContentBuffer);
#else
  addFormHeader(webContentBuffer, S_DEFAULT_TEMPLATE_BOARD);
  addLabel(webContentBuffer, FPSTR(BOARD_P[DEFAULT_TEMPLATE_BOARD]));
  addFormHeaderEnd(webContentBuffer);
#endif

  addFormHeader(webContentBuffer, S_DEVICE_SETTINGS);
#if defined(SUPLA_RELAY)
  addButton(webContentBuffer, S_RELAYS, PATH_RELAY);
#endif

#ifdef SUPLA_BUTTON
  addButton(webContentBuffer, S_BUTTONS, PATH_CONTROL);
#endif

#ifdef SUPLA_LIMIT_SWITCH
  addButton(webContentBuffer, S_LIMIT_SWITCHES, PATH_SWITCH);
#endif

#ifdef GUI_SENSOR_1WIRE
  addButton(webContentBuffer, S_SENSORS_1WIRE, PATH_1WIRE);
#endif

#ifdef GUI_SENSOR_I2C
  addButton(webContentBuffer, S_SENSORS_I2C, PATH_I2C);
#endif

#ifdef GUI_SENSOR_SPI
  addButton(webContentBuffer, S_SENSORS_SPI, PATH_SPI);
#endif

#ifdef GUI_SENSOR_ANALOG
  addButton(webContentBuffer, F("ANALOG"), PATH_ANALOG);
#endif

#if defined(GUI_OTHER) || defined(SUPLA_WEATHER_STATION)
  addButton(webContentBuffer, S_SENSORS_OTHER, PATH_OTHER);
#endif

#if defined(GUI_SENSOR_1WIRE) || defined(GUI_SENSOR_I2C) || defined(GUI_SENSOR_SPI)
  addButton(webContentBuffer, S_CORRECTION, PATH_CORRECTION);
#endif

#ifdef SUPLA_CONFIG
  addButton(webContentBuffer, S_LED_BUTTON_CFG, PATH_CONFIG);
#endif
  addFormHeaderEnd(webContentBuffer);
  addButton(webContentBuffer, S_RETURN, "");

  WebServer->sendHeaderEnd();
}

void handleDeviceSettingsSave() {
  String input = INPUT_BOARD;

  if (strcmp(WebServer->httpServer->arg(input).c_str(), "") != 0) {
    saveChooseTemplateBoard(WebServer->httpServer->arg(input).toInt());
    Supla::Storage::ScheduleSave(2000);
  }

  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      handleDeviceSettings(1);
      break;
    case E_CONFIG_FILE_OPEN:
      handleDeviceSettings(2);
      break;
  }
}
