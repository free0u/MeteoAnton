#ifndef DEVICES_CONFIG_h
#define DEVICES_CONFIG_h

enum SensorType {
    DALLAS_SENSOR,
    DHT_SENSOR,
    BME_SENSOR,
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
    String sensorsApiUrl;
    String logApiUrl;
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

SensorConfig buildBme280Sensor(int pin1, int pin2, DeviceAddress const& addr, long timeout, String field_name,
                               String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 2;
    sensor.pin1 = pin1;
    sensor.pin2 = pin2;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = BME_SENSOR;
    memcpy(sensor.address, addr, sizeof(addr[0]) * 1);

    return sensor;
}

SensorConfig buildDhtSensor(int pin1, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 1;
    sensor.pin1 = pin1;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = DHT_SENSOR;

    return sensor;
}

SensorConfig buildCo2SenseairSensor(int pin1, int pin2, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 2;
    sensor.pin1 = pin1;
    sensor.pin2 = pin2;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = CO2_SENSEAIR_SENSOR;

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
    config.deviceName = "dino";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 28 44 04 A7 33 14 01 34
    DeviceAddress addrDino = {0x28, 0x44, 0x04, 0xA7, 0x33, 0x14, 0x01, 0x34};

    const int SENSORS_COUNT = 6;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        buildDallasSensor(D6, addrDino, 10000, "temp_in", "dallas1"),
        buildDhtSensor(D5, 10000, "hum_in", "hum_in"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

// D6 dallas
// D5 D3 BME
DeviceConfig getColumbusConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "columbus";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 28B037E909000098
    DeviceAddress addrTempOut = {0x28, 0xB0, 0x37, 0xE9, 0x09, 0x00, 0x00, 0x98};

    // 2841F7E9090000FF
    DeviceAddress addrTempIn = {0x28, 0x41, 0xF7, 0xE9, 0x09, 0x00, 0x00, 0xFF};

    const int SENSORS_COUNT = 7;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        buildDallasSensor(D7, addrTempIn, 10000, "temp_in", "dallas1"),
        buildDallasSensor(D7, addrTempOut, 10000, "temp_out", "dallas2"),
        buildBme280Sensor(D3, D5, {0x76}, 10000, "hum_in", "hum_in"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getWaveConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "wave";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 286CA5250A0000DA
    DeviceAddress addrTempIn = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    const int SENSORS_COUNT = 7;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        buildDallasSensor(D6, addrTempIn, 10000, "temp_in", "dallas1"),
        buildDhtSensor(D1, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getOpusConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "opus";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempIn1 = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};
    DeviceAddress addrTempIn2 = {0x28, 0xAA, 0x7A, 0xB3, 0x1A, 0x13, 0x02, 0x61};
    DeviceAddress addrTempIn3 = {0x28, 0x47, 0x60, 0x12, 0x33, 0x14, 0x01, 0x73};
    DeviceAddress addrTempIn4 = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    const int SENSORS_COUNT = 7;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        // buildDallasSensor(D6, addrTempIn1, 10000, "temp_in", "dallas1"),
        // buildDallasSensor(D6, addrTempIn2, 10000, "temp_in2", "dallas2"),
        buildDallasSensor(D6, addrTempIn3, 10000, "temp_in", "dallas3"),
        // buildDallasSensor(D6, addrTempIn4, 10000, "temp_in4", "dallas4"),
        buildDhtSensor(D1, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getSunnyConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "sunny";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 286CA5250A0000DA
    DeviceAddress addrTemp1 = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};
    DeviceAddress addrTemp2 = {0x28, 0xAA, 0x7A, 0xB3, 0x1A, 0x13, 0x02, 0x61};

    const int SENSORS_COUNT = 4;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        // buildDallasSensor(D6, addrTemp1, 10000, "temp1", "dallas1"),
        // buildDallasSensor(D6, addrTemp2, 10000, "temp2", "dallas2"),
        // buildDhtSensor(D1, 10000, "hum_in", "hum_in"),
        // buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getChelConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "chel";
    config.sensorsApiUrl = "***REMOVED***" + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 288C993B0400008F
    DeviceAddress addrTempOut = {0x28, 0x8C, 0x99, 0x3B, 0x04, 0x00, 0x00, 0x8F};

    // 286164118D9646D3
    DeviceAddress addrTempInBak = {0x28, 0x61, 0x64, 0x11, 0x8D, 0x96, 0x46, 0xD3};

    // 28616411BDD852B6
    DeviceAddress addrTempIn = {0x28, 0x61, 0x64, 0x11, 0xBD, 0xD8, 0x52, 0xB6};

    const int SENSORS_COUNT = 9;
    config.sensors = new SensorConfig[SENSORS_COUNT]{
        buildDallasSensor(D6, addrTempIn, 10000, "temp_in", "temp_in"),
        buildDallasSensor(D6, addrTempInBak, 10000, "temp_in_bak", "temp_in_bak"),
        buildDallasSensor(D6, addrTempOut, 10000, "temp_out", "temp_out"),
        buildDhtSensor(D3, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getStubConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "undef";
    config.sensorsSendTimeout = 1e9;
    config.logSendTimeout = 1e9;

    const int SENSORS_COUNT = 0;
    config.sensors = new SensorConfig[SENSORS_COUNT]{};
    config.sensorsCount = SENSORS_COUNT;

    return config;
}

DeviceConfig getDeviceConfigById(int id) {
    switch (id) {
        case 45266:  // Dino
            return getDinoConfig();
        case 3741649:  // Columbus
            return getColumbusConfig();
        case 8825711:  // Wave (usually at work)
            return getWaveConfig();
        case 6499365:  // Opus
            return getOpusConfig();
        case 6500042:  // Sunny
            return getSunnyConfig();
        case 8825236:  // chel
            return getChelConfig();
        default:
            return getStubConfig();
    }
}

#endif
