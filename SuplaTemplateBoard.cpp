#include "SuplaTemplateBoard.h"
#include "SuplaDeviceGUI.h"

void addButton(uint8_t gpio, uint8_t event, uint8_t action, bool pullUp, bool invertLogic) {
  uint8_t nr = ConfigManager->get(KEY_MAX_BUTTON)->getValueInt();
  nr++;

  ConfigESP->setEvent(gpio, event);
  ConfigESP->setAction(gpio, action);
  ConfigESP->setPullUp(gpio, pullUp);
  ConfigESP->setInversed(gpio, invertLogic);

  ConfigESP->setGpio(gpio, nr, FUNCTION_BUTTON);
  ConfigManager->set(KEY_MAX_BUTTON, nr++);
}

void addRelay(uint8_t gpio, uint8_t level) {
  uint8_t nr = ConfigManager->get(KEY_MAX_RELAY)->getValueInt();
  nr++;
  ConfigESP->setLevel(gpio, level);
  ConfigESP->setMemory(gpio, MEMORY_RELAY_RESTORE);
  ConfigESP->setGpio(gpio, nr, FUNCTION_RELAY);
  ConfigManager->set(KEY_MAX_RELAY, nr++);
}

void addLimitSwitch(uint8_t gpio) {
  uint8_t nr = ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt();
  nr++;
  ConfigESP->setGpio(gpio, nr, FUNCTION_LIMIT_SWITCH);
  ConfigManager->set(KEY_MAX_LIMIT_SWITCH, nr++);
}

void addLedCFG(uint8_t gpio, uint8_t level) {
  ConfigESP->setLevel(gpio, level);
  ConfigESP->setGpio(gpio, FUNCTION_CFG_LED);
}

void addLed(uint8_t gpio) {
  ConfigESP->setGpio(gpio, FUNCTION_LED);
}

void addButtonCFG(uint8_t gpio) {
  ConfigESP->setGpio(gpio, FUNCTION_CFG_BUTTON);
}

#ifdef SUPLA_HLW8012
void addHLW8012(int8_t pinCF, int8_t pinCF1, int8_t pinSEL) {
  ConfigESP->setGpio(pinCF, FUNCTION_CF);
  ConfigESP->setGpio(pinCF1, FUNCTION_CF1);
  ConfigESP->setGpio(pinSEL, FUNCTION_SEL);
  // Supla::GUI::addHLW8012(ConfigESP->getGpio(FUNCTION_CF), ConfigESP->getGpio(FUNCTION_CF1), ConfigESP->getGpio(FUNCTION_SEL));
}
#endif

void addRGBW(int8_t redPin, int8_t greenPin, int8_t bluePin, int8_t brightnessPin) {
  uint8_t nr = ConfigManager->get(KEY_MAX_RGBW)->getValueInt();
  nr++;
  ConfigESP->setGpio(redPin, nr, FUNCTION_RGBW_RED);
  ConfigESP->setGpio(greenPin, nr, FUNCTION_RGBW_GREEN);
  ConfigESP->setGpio(bluePin, nr, FUNCTION_RGBW_BLUE);
  ConfigESP->setGpio(brightnessPin, nr, FUNCTION_RGBW_BRIGHTNESS);
  ConfigManager->set(KEY_MAX_RGBW, nr++);
}

void addDimmer(int8_t brightnessPin) {
  addRGBW(OFF_GPIO, OFF_GPIO, OFF_GPIO, brightnessPin);
}

void saveChooseTemplateBoard(int8_t board) {
  ConfigManager->set(KEY_BOARD, board);
  chooseTemplateBoard(board);
}

void chooseTemplateBoard(uint8_t board) {
  int8_t nr, key;
  for (nr = 0; nr <= OFF_GPIO; nr++) {
    key = KEY_GPIO + nr;
    ConfigManager->set(key, "");
  }

  ConfigManager->set(KEY_MAX_BUTTON, "0");
  ConfigManager->set(KEY_MAX_RELAY, "0");
  ConfigManager->set(KEY_MAX_LIMIT_SWITCH, "0");
  ConfigManager->set(KEY_MAX_RGBW, "0");

  switch (board) {
    case BOARD_ELECTRODRAGON:
      addLedCFG(16);
      addButtonCFG(0);
      addButton(0);
      addButton(2);
      addRelay(12);
      addRelay(13);
      break;
    case BOARD_INCAN3:
      addLedCFG(2, LOW);
      addButtonCFG(0);
      addButton(14, Supla::Event::ON_CHANGE);
      addButton(12, Supla::Event::ON_CHANGE);
      addRelay(5);
      addRelay(13);
      addLimitSwitch(4);
      addLimitSwitch(16);
      break;
    case BOARD_INCAN4:
      addLedCFG(12);
      addButtonCFG(0);
      addButton(2, Supla::Event::ON_CHANGE);
      addButton(10, Supla::Event::ON_CHANGE);
      addRelay(4);
      addRelay(14);
      addLimitSwitch(4);
      addLimitSwitch(16);
      break;
    case BOARD_MELINK:
      addLedCFG(12);
      addButtonCFG(5);
      addButton(5);
      addRelay(4);
      break;
    case BOARD_NEO_COOLCAM:
      addLedCFG(4);
      addButtonCFG(13);
      addButton(13);
      addRelay(12);
      break;
    case BOARD_SHELLY1:
      addButtonCFG(5);
      addButton(5, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, false, true);
      addRelay(4);
      break;
    case BOARD_SHELLY2:
      addLedCFG(16);
      addButtonCFG(12);
      addButton(12, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, false, true);
      addButton(14, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, false, true);
      addRelay(4);
      addRelay(5);
      break;
    case BOARD_SONOFF_BASIC:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addRelay(12);
      break;
    case BOARD_SONOFF_MINI:
      addLedCFG(13);
      addButtonCFG(4);
      addButton(4, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addRelay(12);
      break;
    case BOARD_SONOFF_DUAL_R2:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addButton(9);
      addRelay(12);
      addRelay(5);
      break;
    case BOARD_SONOFF_S2X:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addRelay(12);
      break;
    case BOARD_SONOFF_SV:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addRelay(12);
      break;
    case BOARD_SONOFF_TH:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addRelay(12);
      ConfigESP->setGpio(14, FUNCTION_SI7021_SONOFF);
      break;
    case BOARD_SONOFF_TOUCH:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addRelay(12);
      break;
    case BOARD_SONOFF_TOUCH_2CH:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addButton(9);
      addRelay(12);
      addRelay(5);
      break;
    case BOARD_SONOFF_TOUCH_3CH:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addButton(9);
      addButton(10);
      addRelay(12);
      addRelay(5);
      addRelay(4);
      break;
    case BOARD_SONOFF_4CH:
      addLedCFG(13);
      addButtonCFG(0);
      addButton(0);
      addButton(9);
      addButton(10);
      addButton(14);
      addRelay(12);
      addRelay(5);
      addRelay(4);
      addRelay(15);
      break;
    case BOARD_YUNSHAN:
      addLedCFG(2, LOW);
      addButtonCFG(0);
      addButton(3);
      addRelay(4);
      break;

    case BOARD_YUNTONG_SMART:
      addLedCFG(15);
      addButtonCFG(12);
      addButton(12);
      addRelay(4);
      break;

    case BOARD_GOSUND_SP111:
      addLedCFG(2, LOW);
      addButtonCFG(13);
      addButton(13);
      addRelay(15);
      addLed(0);
#ifdef SUPLA_HLW8012
      addHLW8012(5, 4, 12);
      // Supla::GUI::counterHLW8012->setCurrentMultiplier(18388);
      // Supla::GUI::counterHLW8012->setVoltageMultiplier(247704);
      // Supla::GUI::counterHLW8012->setPowerMultiplier(2586583);
      // Supla::GUI::counterHLW8012->setMode(LOW);
#endif
      break;
    case BOARD_DIMMER_LUKASZH:
      addLedCFG(15);
      addButtonCFG(0);
      addDimmer(14);
      addDimmer(12);
      addDimmer(13);
      addButton(5);
      addButton(4);
      addButton(16);
      // ConfigESP->setGpio(GPIO_ANALOG_A0_ESP8266, FUNCTION_NTC_10K);
      break;
    case BOARD_H801:
      addLedCFG(1);
      addButtonCFG(0);
      addRGBW(15, 13, 12, 4);
      break;
    case BOARD_SHELLY_PLUG_S:
      addLedCFG(2, LOW);
      addButtonCFG(13);
      addButton(13);
      addRelay(15);
      addLed(0);
#ifdef SUPLA_HLW8012
      addHLW8012(5, 14, 12);
      // Supla::GUI::counterHLW8012->setCurrentMultiplier(18388);
      // Supla::GUI::counterHLW8012->setVoltageMultiplier(247704);
      // Supla::GUI::counterHLW8012->setPowerMultiplier(2586583);
#endif
      break;
    case BOARD_MINITIGER_1CH:
      addLedCFG(1);
      addButtonCFG(5);
      addButton(5, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addRelay(12);
      break;
    case BOARD_MINITIGER_2CH:
      addLedCFG(1);
      addButtonCFG(3);
      addButton(3, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addButton(4, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addRelay(13);
      addRelay(14);
      break;
    case BOARD_MINITIGER_3CH:
      addLedCFG(1);
      addButtonCFG(3);
      addButton(3, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addButton(5, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addButton(4, Supla::Event::ON_PRESS, Supla::Action::TOGGLE, true, false);
      addRelay(13);
      addRelay(12);
      addRelay(14);
      break;
     case BOARD_WEATHER_STATION :
#define SUPLA_WEATHER_STATION
     //DEFAULT_HOSTNAME = "Weather station";    
      ConfigManager->set(KEY_HOST_NAME,"Weather station" );
      ConfigManager->set(KEY_AQI_ECO_STATUS,"Not connected");
      ConfigManager->set(KEY_AIR_MONITOR_STATUS,"Not connected");
      addLedCFG(12, LOW);                             //D6
      addButtonCFG(16);                               //D0
      ConfigESP->setGpio(0, FUNCTION_SDA);            //D3
      ConfigESP->setGpio(2, FUNCTION_SCL);            //D4
  
      ConfigESP->setGpio(5, FUNCTION_SMOG_SENSOR_RX); //D1
      ConfigESP->setGpio(4, FUNCTION_SMOG_SENSOR_TX); //D2
       
      ConfigManager->set(KEY_SMOG_SENSOR_PERIOD,"10");
      ConfigManager->set(KEY_SMOG_SENSOR_PM25,"25");
      ConfigManager->set(KEY_SMOG_SENSOR_PM10,"50"); 
     
      ConfigManager->setElement(KEY_ACTIVE_SENSOR, SENSOR_I2C_BME280,BMx280_ADDRESS_0X77);
      ConfigManager->setElement(KEY_ACTIVE_SENSOR, SENSOR_I2C_SHT3x,SHT3x_ADDRESS_0X44);
      ConfigManager->setElement(KEY_ACTIVE_SENSOR, SENSOR_I2C_BH1750,BH1750_ADDRESS_0X23);
      ConfigESP->setGpio(13, FUNCTION_ANEMOMETR);   //D7
      ConfigManager->set(KEY_ANEMOMETR_FACTORY,"0.6666");   
      ConfigESP->setGpio(14, FUNCTION_PLUVIOMETER); //D5
      ConfigManager->set(KEY_PLUVIOMETER_FACTORY,"0.1234");  
      ConfigManager->set(KEY_PLUVIOMETER_TIME_H,"12");  
         
     break; 
  }
}
