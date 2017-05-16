/*
12/03/2017
Les fonctions de base marchent
Il faut les assembler
Il faut terminer de mettre le DEBUG dans les printf
Il faut changer le numero de tel pour celui de Basile. Pkoi pas faire un changement dynamique

17/03/2017
Ajouté la gestion du secteur. Envoi un SMS à detection de perte et retour secteur.

22/03/2017
Ajouté la detection de sur-temperature. Envoie un SMS la temp depasse un seuil (25 deg pour test)
Reste a enregistrer à quelle heure le SMS est parti et renvoyer un SMS apres 1h.

05/04/2017
Rajouté renvoi SMS après 1h de plus de secteur
Rajouté renvoi SMS après 1h de sur-temperature

06/04/2017
Paufiné le code
Il y a un problème avec le SLEEP_NETWORK_STANDBY. On dirait que les SMS reçus pendant le sleep ne sont pas traités au reveil
Autre probleme: le SLEEP_MODE_DEEP fait un reset du CPU, et on perd l'info b_SMS_batterie=1; Donc quand le secteur revient pas de SMS envoyé. Il faudrait le stocker en EEPROM

11/04/2017
Corrigé doubles SMS. Ajouté sauvegarde EEPROM.
Enlevé le mode DEBUG_APP
Reste a changer le numéro de telephone
Amelioration possible: en cas de depassement d'un seuil, refaire 3 mesures à 30sec d'intervalle. Si toujours trop chaud, envoyer SMS
*/

#include "functions-frigo-monitoring.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

/* Mandatory to use Free sim card */
// https://docs.particle.io/faq/particle-devices/electron-3rdparty-sims/electron/
// https://docs.particle.io/reference/firmware/electron/#setcredentials-
STARTUP(cellular_credentials_set("free", "", "", NULL));


unsigned long Time_SMS_batterie=0;
bool b_SMS_batterie = 255;
unsigned long Time_seuil_atteint=0;
bool bSeuilAtteint=0;


void setup() {
  char str[120];

  Cellular.on();
  if(DEBUG_APP) Particle.connect();
  if(DEBUG_APP) Serial.begin(9600);
  delay(3000); // for debug: give a chance to reflash the code or reset the device

  EEPROM.get(add_b_SMS_batterie, b_SMS_batterie);
  EEPROM.get(add_Time_SMS_batterie, Time_SMS_batterie);
  if(DEBUG_APP){
    snprintf(str, sizeof(str), "Lecture b_SMS_batterie EEPROM=%d", b_SMS_batterie);
    Serial.println(str);
    snprintf(str, sizeof(str), "Lecture Time_SMS_batterie EEPROM=%lu", Time_SMS_batterie);
    Serial.println(str);
  }

  initialiseSensors();
  initialiseThingSpeak();

  delay(3000);
}



void loop() {

    bool bSeuilAtteint_temp = 0;

    char str[120];
    extern DeviceAddress Thermometers[3];
    extern double Derniere_Mesure_temperature[3];
    extern int seuils_temperature[3];

    // mesure TEMPERATURE_PRECISION
    for(int i=0; i<3; i++){
      Derniere_Mesure_temperature[i] = LireTemperature(Thermometers[i]);
      if(Derniere_Mesure_temperature[i]>seuils_temperature[i]){
        bSeuilAtteint_temp = 1;
      }
      if(DEBUG_APP){
        snprintf(str, sizeof(str), "Temperature%d = %.2f", i+1, Derniere_Mesure_temperature[i]);
        Serial.println(str);
      }
    }
    if(bSeuilAtteint_temp==1){  // sur-temperature détéctée
      if(bSeuilAtteint==0){ // nouvelle sur-temperature détéctée
        bSeuilAtteint=1;
        snprintf(str, sizeof(str), "Surveillance Frigo: Temperature trop chaude. Temperature1=%.2f Temperature2=%.2f Temperature3=%.2f",Derniere_Mesure_temperature[0], Derniere_Mesure_temperature[1], Derniere_Mesure_temperature[2]);
        if(DEBUG_APP) Serial.printlnf("Envoi SMS: %s", str);
        EnvoyerSMS(NUM_FABIEN, str);
        Time_seuil_atteint = Time.now();
      }
      else{ // sur-temperature déjà détéctée la dernière fois
        if((Time.now()-Time_seuil_atteint)>INTERVAL_1h_sec){
          snprintf(str, sizeof(str), "Surveillance Frigo: Temperature toujours trop chaude. Temperature1=%.2f Temperature2=%.2f Temperature3=%.2f",Derniere_Mesure_temperature[0], Derniere_Mesure_temperature[1], Derniere_Mesure_temperature[2]);
          if(DEBUG_APP) Serial.printlnf("Envoi SMS: %s", str);
          EnvoyerSMS(NUM_FABIEN, str);
          Time_seuil_atteint = Time.now();
        }
      }
    }
    else{ // pas de sur-temperature détéctée
      if(bSeuilAtteint==1){ // sur-temperature détéctée la dernière fois
        bSeuilAtteint=0;
        snprintf(str, sizeof(str), "Surveillance Frigo: Temperature normale. Temperature1=%.2f Temperature2=%.2f Temperature3=%.2f",Derniere_Mesure_temperature[0], Derniere_Mesure_temperature[1], Derniere_Mesure_temperature[2]);
        if(DEBUG_APP) Serial.printlnf("Envoi SMS: %s", str);
        EnvoyerSMS(NUM_FABIEN, str);
      }
    }// fin mesure temperatures




    PMIC pmic;
    if(pmic.isPowerGood()){ // 1=secteur
      if(b_SMS_batterie==1){  // si on avait envoyé un SMS de perte de secteur
        EnvoyerSMS(NUM_FABIEN, "Surveillance Frigo: secteur est normal");
        b_SMS_batterie=0; // on réinitialise le défaut
        EEPROM.put(add_b_SMS_batterie, b_SMS_batterie); // sauvegarde en EEPROM
        EEPROM.put(add_Time_SMS_batterie, Time_SMS_batterie); // sauvegarde en EEPROM
      }
    } // fin secteur

    else{ // 0=batterie
      if(check_battery_status()<20){
        EnvoyerSMS(NUM_FABIEN, "Surveillance Frigo: plus de batterie. Brancher sur le secteur et attendre 1 heure");
        System.sleep(SLEEP_MODE_DEEP, INTERVAL_1h_sec); // sleep 1h
      }
      else{
        if(b_SMS_batterie==0){ // on a pas encore envoyé de SMS pour perte du secteur
          EnvoyerSMS(NUM_FABIEN, "Surveillance Frigo: panne du secteur");
          b_SMS_batterie=1;
          Time_SMS_batterie = Time.now();
          EEPROM.put(add_b_SMS_batterie, b_SMS_batterie); // sauvegarde en EEPROM
          EEPROM.put(add_Time_SMS_batterie, Time_SMS_batterie); // sauvegarde en EEPROM
        }
        else if((b_SMS_batterie==1) && ((Time.now()-Time_SMS_batterie)>INTERVAL_1h_sec)){
          EnvoyerSMS(NUM_FABIEN, "Surveillance Frigo: secteur toujours en panne");
          Time_SMS_batterie=Time.now();
          EEPROM.put(add_Time_SMS_batterie, Time_SMS_batterie); // sauvegarde en EEPROM
        }

      }
    } // fin batterie





    TraiterSMSRecu();




    if(Particle.connected()){ // if connected, stay connected
      EnvoyerDataOnline();

      if(DEBUG_APP) Serial.printlnf("Delaying now.");
      delay(INTERVAL_5min_ms);
    }
    else{ // else go to sleep
      if(DEBUG_APP){
        Serial.printlnf("Going to sleep now.");
        delay(1000); // wait 1sec to wait for Serial to finish
      }
      System.sleep(65536,RISING, SLEEP_NETWORK_STANDBY, INTERVAL_5min_ms/1000);  // functions requires sec, not ms
    }
}
