#ifndef DHT_HPP
#define DHT_HPP

#include "../../hal/hal.h"
#include "../../tools/data_types/byte_types.h" 
#include "../weather_sensor.h"

#include "dht_settings.h"

typedef enum DHTType{
    DHT11, DHT22, DHT21
} DHTType;

typedef struct DHTInterface {
    gpio_t signal;
    DHTType sensorType;
} DHTInterface;


bool readDHT(int16_t* temp, int16_t* humid);
void startDHT(void);
void resetDHT(void);

void createDHT(WeatherSensor* self, DHTInterface* interface);


////// 

int32_t _pollSignal(uint8_t state, uint32_t timeout);
bool _readPulses(uint16_t* buff, uint8_t size);

#endif /* DHT_HPP */
