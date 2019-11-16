#ifndef DEVICES_CONFIG_h
#define DEVICES_CONFIG_h

enum SensorType {
    DALLAS_SENSOR,
    DHT_SENSOR,
    CO2_SENSEAIR_SENSOR,
    UPTIME_SENSOR,
    BUILD_VERSION_SENSOR,
    FIRMWARE_VERSION_SENSOR,
    FREE_HEAP_SENSOR
};

struct SensorConfig {
    SensorConfig() {}
    SensorType type;
    int pin1;
    int pin2;
    int pin_count;
    DeviceAddress address;  // for dallas
    long timeout;
    String field_name;  // temp_in, temp_out
    String debug_name;  // dallasTemp
};

struct DeviceConfig {
    SensorConfig* sensors;
    int sensorsCount;
    String deviceName;  // Columbus, Wave, Dino
    long sensorsSendTimeout;
    long logSendTimeout;
};

SensorConfig buildDallasSensor(int pin, DeviceAddress const& addr, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 1;
    sensor.pin1 = pin;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = DALLAS_SENSOR;
    memcpy(sensor.address, addr, sizeof(addr[0]) * 8);

    return sensor;
}

SensorConfig buildUptimeSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = UPTIME_SENSOR;

    return sensor;
}

SensorConfig buildBuildVersionSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = BUILD_VERSION_SENSOR;

    return sensor;
}

SensorConfig buildFirmwareVersionSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = FIRMWARE_VERSION_SENSOR;

    return sensor;
}

SensorConfig buildFreeHeapVersionSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = FREE_HEAP_SENSOR;

    return sensor;
}

DeviceConfig getDinoConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "Dino";
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 28 44 04 A7 33 14 01 34
    DeviceAddress addrDino = {0x28, 0x44, 0x04, 0xA7, 0x33, 0x14, 0x01, 0x34};

    const int SENSORS_COUNT = 5;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        buildDallasSensor(D6, addrDino, 10000, "temp_in", "dallas1"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getDeviceConfigById(int id) { return getDinoConfig(); }

#endif
