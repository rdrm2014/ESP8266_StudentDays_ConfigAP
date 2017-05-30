#include "Sensors.h"

#include <DHT.h>

  /*int sensorPin;
  SensorType sensorType;
  public:*/
    Sensor::Sensor(){}
    Sensor::Sensor(int pin){
      sensorPin=pin;
      initialize();
      }
    void Sensor::initialize(){
      }
    void Sensor::write(){};
    String Sensor::read(){
      return "Sensor";
      }       

//    DHT_Sensor::DHT_Sensor(){}
    /*DHT_Sensor::DHT_Sensor(int pin){
      //sensorPin=pin;
      initialize();
    }*/
    void DHT_Sensor::initialize() {
      _dht = DHT(sensorPin, 22);
    };
    //override void write() {}
    String DHT_Sensor::read() { 
      hum = _dht.readHumidity();
      temp = _dht.readTemperature();  
      
      snprintf(resultRead, sizeof(resultRead), "{\"hum\": %d.%d, \"temp\": %d.%d}", (int)hum, (int)((hum - (int)hum) * 100), (int)temp, (int)((temp - (int)temp) * 100));
      return String(resultRead); 
    }
/*  
class Hum_Sensor:public Sensor{
  int hum;
  public:
    void write() {}
    String read() { 
      int moistureValue = analogRead(pinArray[5]);
      /*if(moistureValue<250) moistureValue=250;
      if(moistureValue>750) moistureValue=750;
      //Serial.print("moistureValue: ");  
      //Serial.println(moistureValue);  
      hum = 100-(((moistureValue-250))*100/500);*
    
      moistureValue = constrain(moistureValue, 250, 750);
      hum = map(moistureValue, 250, 750, 100, 0);
      
      //Serial.print("hum: ");
      //Serial.println(hum);
      snprintf(result, 200, "{\"hum\": %d}", (int)hum);
      return String(result);
    }
};

class Relay_Sensor:public Sensor{
  int sensorPin;
  public:
    void write() { 
      if (code=="0") {    
        digitalWrite (sensorPin, LOW);
      }
      if (code=="1") {    
        digitalWrite (sensorPin, HIGH);
      }
    }
    int read() {}
};*/

