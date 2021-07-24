#include "config.h"
#include <WiFi.h>
#include "time.h"


TTGOClass *ttgo;
PCF8563_Class *rtc;
TFT_eSPI *tft;



const char *ssid            = "Zero";
const char *password        = "375Alaska";
const char *ntpServer       = "pool.ntp.org";
const long  gmtOffset_sec   = -3*3600;
const int   daylightOffset_sec = 0*3600;
//const int   INT_PIN         = 35;

char buf[128];
bool rtcIrq = false;
bool aceso = true;
bool atualizado = false;

static bool irq_axp202 = false;

void setup()
{
  //pinMode(AXP202_INT, INPUT);
  Serial.begin(115200);
  Serial.println("Inicializando");
  //pinMode(AXP202_INT, INPUT);
  //attachInterrupt(AXP202_INT, [] {irq_axp202 = true;}, FALLING);
  //ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
  //ttgo->power->clearIRQ();
  
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    ttgo->setBrightness(50);
    tft = ttgo->tft;
    tft->fillScreen(TFT_BLACK);

    ttgo->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    if(false){
      ttgo->setBrightness(128);       // 0~255
    rtc = ttgo->rtc;
    tft->setTextColor(TFT_GREEN, TFT_BLACK);

    Serial.printf("Connecting to %s \n", ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time, Restart in 3 seconds");
        delay(3000);
        esp_restart();
        while (1);
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    rtc->syncToRtc();
    tft->setFreeFont(&FreeMonoOblique9pt7b);
    atualizado = true;      
    }
}

void goToDeepSleep()
{
    Serial.println("Go To Deep Sleep Mode");
    
    // Set screen and touch to sleep mode
    ttgo->displaySleep();
    ttgo->powerOff();
    gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);  
    esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP202_INT, HIGH);    
    esp_deep_sleep_start();
}

void loop()
{
    int cur =  ttgo->power->getChargeControlCur();
    ttgo->tft->setTextColor(TFT_NAVY, TFT_BLACK);
    ttgo->tft->drawString("T-Watch RTC",  50, 50, 4);
    //cur =  ttgo->power->getChargeControlCur();
    //string corrente = "Current: " + "teste";
    //ttgo->tft->drawString(corrente, 50, 80, 4); 
    //ttgo->tft->drawString("Matheus Cruz",  50, 80, 4);

    snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
    ttgo->tft->drawString(buf, 5, 80, 7);

    String carga = "Descarregando: ";
    String percentual = "B: ";
    percentual += ttgo->power->getBattPercentage();
    ttgo->tft->drawString(carga,  50, 140, 2);

    ttgo->tft->setTextColor(TFT_DARKGREEN, TFT_BLACK);
    ttgo->tft->drawString(percentual,  5, 5, 3);

/*            tft->print("Discharge:");
            tft->print(ttgo->power->getBattDischargeCurrent());
            tft->println(" mA");
            tft->print("Per: ");
            tft->print(ttgo->power->getBattPercentage());
            tft->println(" %"); */

    Serial.println("Atualizou");
    delay(1000);
    //goToDeepSleep();
    
}
