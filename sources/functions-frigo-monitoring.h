#include "application.h"
#include "spark-dallas-temperature.h"
#include "OneWire.h"
#include "ThingSpeak.h"
#include "uCommand.h"


#define DEBUG_APP 0

#define NUM_FABIEN "+33623500183"
#define NUM_BASILE "+33647087417"

#define SEUIL_T1 25
#define SEUIL_T2 25
#define SEUIL_T3 25

#define TEMPERATURE_PRECISION 10  // 10 bits is sufficient

#define INTERVAL_5min_ms 300*1000 // interval between measurements, 5min in ms
#define INTERVAL_1h_sec 3600      // interval between SMS reminders, 1h in seconds

#define add_b_SMS_batterie 0  // address of b_SMS_batterie in EEPROM
#define add_Time_SMS_batterie 1  // address of b_SMS_batterie in EEPROM




void initialiseSensors(void);
double LireTemperature(const uint8_t* mySensor);
void TraiterTemperature(void);
float check_battery_status(void);

void initialiseSMSmode(void);
void TraiterSMSRecu(void);
void EnvoyerSMS(char* NumTel, char* ContenuSMS);

void initialiseThingSpeak(void);
void EnvoyerDataOnline(void);

void RechercheCapteurs(void);
