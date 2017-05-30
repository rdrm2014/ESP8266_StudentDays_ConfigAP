#ifndef sensors_HHH
#define sensors_HHH

#include <DHT.h>

enum SensorType {_DHT, _Relay, _Hum};
class Sensor {
  protected:
    int sensorPin;
    SensorType sensorType;
    char resultRead[100];
  public:
    Sensor();
    Sensor(int pin);
    virtual void initialize();
    virtual void write();
    virtual String read();       
  };
 
class DHT_Sensor:public Sensor{
  DHT _dht;
  float hum;
  float temp;
  public:
    ~DHT_Sensor();
    DHT_Sensor(int pin);
    virtual void initialize();
    virtual void write();
    virtual String read();
  };
  /*
class Hum_Sensor:public Sensor{
  public:
    override void write();
    override String read();
};

class Relay_Sensor:public Sensor{
  public:
    override void write();
    override String read();
};*/
#endif
