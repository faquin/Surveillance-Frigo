#include "functions-frigo-monitoring.h"


/* sensors adresses */
DeviceAddress Thermometers[3]={
  { 0x28, 0xFF, 0x70, 0x59, 0x60, 0x16, 0x5, 0xB2 },
  { 0x28, 0xFF, 0x02, 0xC6, 0x60, 0x16, 0x5, 0x27 },
  { 0x28, 0xFF, 0x8A, 0xDA, 0x60, 0x16, 0x5, 0xF8 }
};
double Derniere_Mesure_temperature[3]={ -1, -1, -1};
int seuils_temperature[3] = {SEUIL_T1, SEUIL_T2, SEUIL_T3};


OneWire oneWire(D2);  // on pin D2 (a 4.7K pull-up resistor is necessary)
DallasTemperature sensors(&oneWire);

FuelGauge battery_status;

uCommand uCmd;


/* Thingspeak configuration */
TCPClient client;
unsigned long myChannelNumber = 215423;
const char * myWriteAPIKey = "WT661JQPTD76RV2W";

void initialiseSensors(void){
  sensors.begin();
  sensors.setResolution(TEMPERATURE_PRECISION);
}

double LireTemperature(const uint8_t* mySensor){
  double tTemp;
  int dsAttempts=0;

  sensors.requestTemperatures();

  tTemp=sensors.getTempC(mySensor);
  while(tTemp==DEVICE_DISCONNECTED_C && dsAttempts <4){
    delay(400);
    tTemp=sensors.getTempC(mySensor);
    dsAttempts++;
  }

  return tTemp; // return -127 if reading didnt work

}

void TraiterTemperature(void){
  sensors.requestTemperatures();
  Derniere_Mesure_temperature[0]=LireTemperature(Thermometers[0]);
  Derniere_Mesure_temperature[1]=LireTemperature(Thermometers[1]);
  Derniere_Mesure_temperature[2]=LireTemperature(Thermometers[2]);

  if(DEBUG_APP){
    Serial.println(Derniere_Mesure_temperature[0]);
    Serial.println(Derniere_Mesure_temperature[1]);
    Serial.println(Derniere_Mesure_temperature[2]);
  }
}

float check_battery_status(void){
  return battery_status.getSoC();
}


void initialiseSMSmode(void){

    int atResult;

    uCmd.setDebug(false);
  	// set up text mode for the sms
  	atResult = uCmd.setSMSMode(1);
    if(DEBUG_APP){
    	if(atResult == RESP_OK){
    		Serial.println("Text mode setup was OK");
    	}
    	else{
    		Serial.println("Did not set up text mode");
    	}
    }
}

void TraiterSMSRecu(void){

  char str[80];

  initialiseSMSmode();

  if(DEBUG_APP) Serial.println("Looking for an SMS message");

  // read next available messages
  if(uCmd.checkMessages(10000) == RESP_OK){
  uCmd.smsPtr = uCmd.smsResults;
  if(DEBUG_APP){
    if(uCmd.numMessages==0) Serial.println("no SMS received");
  }
  for(int i=0;i<uCmd.numMessages;i++){
    if(DEBUG_APP) Serial.printlnf("message received from %s %s",uCmd.smsPtr->phone, uCmd.smsPtr->sms);
    String strTemp = String(uCmd.smsPtr->sms);

    if(strTemp.indexOf("LireTemperature")!=-1){  // if SMS contains "LireTemperature"
      snprintf(str, sizeof(str), "Temperature1=%.2f Temperature2=%.2f Temperature3=%.2f",Derniere_Mesure_temperature[0], Derniere_Mesure_temperature[1], Derniere_Mesure_temperature[2]);
      if(DEBUG_APP) Serial.printlnf("Envoi SMS: %s", str);
      EnvoyerSMS(NUM_FABIEN, str);
    }

    else if(strTemp.indexOf("ConnecteCloud")!=-1){  // if SMS contains "LireTemperature"
      Particle.connect();
      if (waitFor(Particle.connected, 20000)) {
        if(DEBUG_APP) Serial.printlnf("Envoi SMS: Particle Connected");
        EnvoyerSMS(NUM_FABIEN, "Particle Connected");
      }
      else{
        Particle.disconnect();
          if(DEBUG_APP) Serial.printlnf("Envoi SMS: Particle cannot connect to cloud");
        EnvoyerSMS(NUM_FABIEN, "Particle cannot connect to cloud");
      }
    }

    else if(strTemp.indexOf("DeconnecteCloud")!=-1){  // if SMS contains "LireTemperature"
      Particle.disconnect();
      delay(1000);
      if(DEBUG_APP) Serial.printlnf("Envoi SMS: Particle Disonnected");
      EnvoyerSMS(NUM_FABIEN, "Particle Disonnected");
    }

    else if(strTemp.indexOf("RechercheCapteurs")!=-1){  // if SMS contains "LireTemperature"
      RechercheCapteurs();  //
    }

    // delete the message so it won't be processed anmyore
    if(uCmd.deleteMessage(uCmd.smsPtr->mess,10000) == RESP_OK){
        if(DEBUG_APP) Serial.println("SMS deleted successfully");
      }
      else{
        if(DEBUG_APP) Serial.println("could not delete SMS");
      }

      uCmd.smsPtr++;
    }

  }
  else{
    if(DEBUG_APP) Serial.println("could not look for new SMS");
  }




}

void EnvoyerSMS(char* NumTel, char* ContenuSMS){
    initialiseSMSmode();
    if(uCmd.sendMessage(ContenuSMS,NumTel,10000) == RESP_OK){
      if(DEBUG_APP) Serial.println("SMS sent successfully");
    }
    else{
      if(DEBUG_APP) Serial.println("SMS failed to send");
    }
}

void initialiseThingSpeak(void){
  ThingSpeak.begin(client);
}

void EnvoyerDataOnline(void){
  char str[120];
  ThingSpeak.setField(1, (float)Derniere_Mesure_temperature[0]);
  ThingSpeak.setField(2, (float)Derniere_Mesure_temperature[1]);
  ThingSpeak.setField(3, (float)Derniere_Mesure_temperature[2]);
  ThingSpeak.setField(6, check_battery_status());
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  snprintf(str, sizeof(str), "T1=%.0f T2=%.0f T3=%.0f Batt=%.0f%%",Derniere_Mesure_temperature[0], Derniere_Mesure_temperature[1], Derniere_Mesure_temperature[2], battery_status.getSoC());
  if(DEBUG_APP) Serial.printlnf("Publishing: %s", str);
  Particle.publish("Surveillance Frigo", str);

}


void RechercheCapteurs(void){
/*  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    //delay(250);
    return;
  }
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("Chip = DS18S20");  // or old DS1820
      break;
    case 0x28:
      Serial.println("Chip = DS18B20");
      break;
    case 0x22:
      Serial.println("Chip = DS1822");
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  Serial.print("ROM = ");
  Serial.print("0x");
  Serial.print(addr[0],HEX);
  for( int i = 1; i < 8; i++) {
    Serial.print(", 0x");
    Serial.print(addr[i],HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }

  Serial.println();
  ds.reset();
  */
}
