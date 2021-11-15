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



#include "SuplaWebPageDownload.h"

#ifdef ARDUINO_ARCH_ESP8266
#include "FS.h"
#elif ARDUINO_ARCH_ESP32
#include "SPIFFS.h"
#endif

void handleDownload() {
  if (!WebServer->isLoggedIn()) {
    return;
  }

  File dataFile = SPIFFS.open(F(CONFIG_FILE_PATH), "r");

  if (!dataFile) {
    return;
  }

  String str = F("attachment; filename=config_");
  str += ConfigManager->get(KEY_HOST_NAME)->getValue();
  str += Supla::Channel::reg_dev.SoftVer;
  str += '_';
  str += F(".dat");

  WebServer->httpServer->sendHeader(F("Content-Disposition"), str);
  WebServer->httpServer->streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
}
