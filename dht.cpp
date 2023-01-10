#include "dht.h"

static DHTInterface* _interfaceDHT;

bool readDHT(int16_t* temp, int16_t* humid){
    uint16_t checksum = 0, hm, tmp;

    uint32_t lastTimeUs;
    uint32_t currentTimeUs = timerUsStart();

    int16_t tempCache = 0, humCache =0;

    if  ( ( currentTimeUs - lastTimeUs ) > DHT_DATA_HOLD_TIME_US ){
        gpioSet(_interfaceDHT->signal, LOW);
        sleepUs(DHT_START_LOW_TIME_US);
        gpioSet(_interfaceDHT->signal, HIGH);
        sleepUs(DHT_START_HIGH_TIME_US);

        gpioInit(_interfaceDHT->signal, GPIO_IN);
        if(_pollSignal(HIGH, DHT_TIMEOUT_US)){
            resetDHT();
            return false;
        }

        if(_pollSignal(LOW, DHT_TIMEOUT_US)){
            resetDHT();
            return false;
        }

        // read sensor values sequantially
        if( !_readPulses( &hm, 16U ) ){ resetDHT(); return false; }
        if( !_readPulses( &tmp, 16U ) ){ resetDHT(); return false; }
        if( !_readPulses( &checksum, 8U ) ){ resetDHT(); return false; }

        resetDHT();

        // validate sensor data 
        uint8_t sum = (tmp >> 8) + (tmp & 0xff) + (hm >> 8) + (hm & 0xff);
        if (sum != checksum) {
            print("incorrect checksum! \n");
            return false;
        }

        // parse values based on type of sensor used
        switch (_interfaceDHT->sensorType){
            case DHT11:
                tempCache = (int16_t)((tmp >> 8) * 10);
                humCache = (int16_t)((hm >> 8) * 10);
                break;
            case DHT21:
            case DHT22:
                humCache = (int16_t)(hm);
                if (tmp & 0x8000){
                    tempCache = (int16_t)((tmp & ~0x8000) * -1); // negative
                }else{
                    tempCache = (int16_t)tmp;
                }
                break;
            default:
                print("No or incorrect DHT version specified\n");
                return false;
        }

        lastTimeUs = currentTimeUs;
    }

    if(temp != nullptr){
        *temp = tempCache;
    }

    if(hum != nullptr){
        *hum = humCache;
    }

    return true;
}

void startDHT(void){
    resetDHT();
    sleepMs(2000);
    print("DHT sensor is online!\n");
}

void resetDHT(void){
    gpioInit(_interfaceDHT->signal, GPIO_OUT);
    gpioSet(_interfaceDHT->signal, HIGH);
}


void createDHT(WeatherSensor* self, DHTInterface* interface){
    _interfaceDHT = interface;
    self->read = &readDHT;
    self->start = &startDHT;
    self->reset = &resetDHT;
    print( "DHT sensor sucessfully created! \n" );
}


int32_t _pollSignal(uint8_t state, uint32_t timeout){
    while(
         ( gpioRead( _interfaceDHT->signal ) > 0 ) != state ) 
         && timeout
    ){
        sleepUs(1);
        timeout--;
    }

    return (timeout > 0) ? 0 : -1;
}

bool _readPulses(uint16_t* buff, uint8_t size){
    uint16_t result = 0;
    for(uint8_t i=0; i<size; i++){
        uint32_t beginTime, endTimeUs;
        result <<= 1;

        if(_pollSignal(HIGH, DHT_TIMEOUT_US)){ return false; }
        beginTimeUs = timerUsStart();
        if(_pollSignal(LOW, DHT_TIMEOUT_US)){ return false; }
        endTimeUs = timerUsStart();

        if( (endTimeUs - beginTimeUs) > DHT_PULSE_WIDTH_THRESHOLD_US ){ result |= 0x001; }
    }
    *buff = result;
    return true;
}
