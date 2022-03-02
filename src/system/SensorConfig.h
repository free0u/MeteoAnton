#ifndef SENSOR_CONFIG_h
#define SENSOR_CONFIG_h
#include <DallasTemperature.h>

enum SensorType {
    DALLAS_SENSOR,
    DHT_SENSOR,
    DHT_SENSOR_TEMP,
    BME_SENSOR,
    CO2_SENSEAIR_SENSOR,
    TRANSMITTER_433,
    RECEIVER_433,
    IRMS_SENSOR,
    POWER_SENSOR,
    POWER_SUM,    // spend from last point of data
    POWER_SPENT,  // total spend from starting of measuring
    WATER_SPENT,  // total spend  of water from starting of measuring
    UPTIME_SENSOR,
    BUILD_VERSION_SENSOR,
    FIRMWARE_VERSION_SENSOR,
    FREE_HEAP_SENSOR,
    RSSI_SENSOR,
    MAX_LOOP_TIME,       // max loop time between sending data, kindo of local maximums
    UPDATE_SENSORS_TIME  // max update sensor time between sending data, kindo of local maximums
};

struct SensorConfig {
    SensorConfig(SensorType const& _sensorType, String const& _field_name) {
        timeout = 10000;
        type = _sensorType;
        field_name = _field_name;
    }
    SensorType type;
    int pin1;
    int pin2;
    int pin3;
    float param1;
    float param2;
    DeviceAddress address;  // for dallas
    long timeout;
    String field_name;  // temp_in, temp_out
};

SensorConfig buildDallasSensor(int pin, DeviceAddress const& addr, String field_name) {
    SensorConfig sensor = SensorConfig(DALLAS_SENSOR, field_name);

    sensor.pin1 = pin;
    memcpy(sensor.address, addr, sizeof(addr[0]) * 8);

    return sensor;
}

SensorConfig buildTransmitter433Sensor(int rx, int tx, int ptt, String field_name) {
    SensorConfig sensor = SensorConfig(TRANSMITTER_433, field_name);

    sensor.pin1 = rx;
    sensor.pin2 = tx;
    sensor.pin3 = ptt;

    return sensor;
}

SensorConfig buildReceiver433Sensor(int rx, int tx, int ptt, String field_name) {
    SensorConfig sensor = SensorConfig(RECEIVER_433, field_name);

    sensor.pin1 = rx;
    sensor.pin2 = tx;
    sensor.pin3 = ptt;

    return sensor;
}

// pin2 - DHT TYPE, 22/21/11
SensorConfig buildDhtSensor(int pin1, int pin2, String field_name) {
    SensorConfig sensor = SensorConfig(DHT_SENSOR, field_name);

    sensor.pin1 = pin1;
    sensor.pin2 = pin2;

    return sensor;
}

SensorConfig buildDhtTempSensor(int pin1, String field_name) {
    SensorConfig sensor = SensorConfig(DHT_SENSOR_TEMP, field_name);

    sensor.pin1 = pin1;

    return sensor;
}

SensorConfig buildIrmsSensor(float calibration, float voltage, String field_name) {
    SensorConfig sensor = SensorConfig(IRMS_SENSOR, field_name);

    sensor.param1 = calibration;
    sensor.param2 = voltage;

    return sensor;
}

SensorConfig buildPowerSensor(float calibration, float voltage, String field_name) {
    SensorConfig sensor = SensorConfig(POWER_SENSOR, field_name);

    sensor.param1 = calibration;
    sensor.param2 = voltage;

    return sensor;
}

SensorConfig buildPowerSumSensor(String field_name) {
    SensorConfig sensor = SensorConfig(POWER_SUM, field_name);

    return sensor;
}

SensorConfig buildPowerSpentSensor(String field_name) {
    SensorConfig sensor = SensorConfig(POWER_SPENT, field_name);

    return sensor;
}

SensorConfig buildWaterSpentSensor(int pin, String field_name) {
    SensorConfig sensor = SensorConfig(WATER_SPENT, field_name);
    sensor.pin1 = pin;

    return sensor;
}

SensorConfig buildCo2SenseairSensor(int pin1, int pin2, String field_name) {
    SensorConfig sensor = SensorConfig(CO2_SENSEAIR_SENSOR, field_name);

    sensor.pin1 = pin1;
    sensor.pin2 = pin2;

    return sensor;
}

SensorConfig buildUptimeSensor(String field_name) {
    SensorConfig sensor = SensorConfig(UPTIME_SENSOR, field_name);

    return sensor;
}

SensorConfig buildBuildVersionSensor(String field_name) {
    SensorConfig sensor = SensorConfig(BUILD_VERSION_SENSOR, field_name);

    return sensor;
}

SensorConfig buildFirmwareVersionSensor(String field_name) {
    SensorConfig sensor = SensorConfig(FIRMWARE_VERSION_SENSOR, field_name);

    return sensor;
}

SensorConfig buildRssiSensor(String field_name) {
    SensorConfig sensor = SensorConfig(RSSI_SENSOR, field_name);

    return sensor;
}

SensorConfig buildMaxLoopTimeSensor(String field_name) {
    SensorConfig sensor = SensorConfig(MAX_LOOP_TIME, field_name);

    return sensor;
}

SensorConfig buildUpdateSensorTimeSensor(String field_name) {
    SensorConfig sensor = SensorConfig(UPDATE_SENSORS_TIME, field_name);

    return sensor;
}

SensorConfig buildFreeHeapVersionSensor(String field_name) {
    SensorConfig sensor = SensorConfig(FREE_HEAP_SENSOR, field_name);

    return sensor;
}

#endif