#ifndef DEVICES_CONFIG_h
#define DEVICES_CONFIG_h
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
    SensorConfig() {}
    SensorType type;
    int pin1;
    int pin2;
    int pin3;
    int pin_count;
    float param1;
    float param2;
    DeviceAddress address;  // for dallas
    long timeout;
    String field_name;  // temp_in, temp_out
    String debug_name;  // dallasTemp
};

struct DeviceConfig {
    // SensorConfig* sensors;
    std::vector<SensorConfig> sensors;
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

SensorConfig buildTransmitter433Sensor(int rx, int tx, int ptt, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 3;
    sensor.pin1 = rx;
    sensor.pin2 = tx;
    sensor.pin3 = ptt;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = TRANSMITTER_433;

    return sensor;
}

SensorConfig buildReceiver433Sensor(int rx, int tx, int ptt, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 3;
    sensor.pin1 = rx;
    sensor.pin2 = tx;
    sensor.pin3 = ptt;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = RECEIVER_433;

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

// pin2 - DHT TYPE, 22/21/11
SensorConfig buildDhtSensor(int pin1, int pin2, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 1;
    sensor.pin1 = pin1;
    sensor.pin2 = pin2;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = DHT_SENSOR;

    return sensor;
}

SensorConfig buildDhtTempSensor(int pin1, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 1;
    sensor.pin1 = pin1;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = DHT_SENSOR_TEMP;

    return sensor;
}

SensorConfig buildIrmsSensor(float calibration, float voltage, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.param1 = calibration;
    sensor.param2 = voltage;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = IRMS_SENSOR;

    return sensor;
}

SensorConfig buildPowerSensor(float calibration, float voltage, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.param1 = calibration;
    sensor.param2 = voltage;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = POWER_SENSOR;

    return sensor;
}

SensorConfig buildPowerSumSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = POWER_SUM;

    return sensor;
}

SensorConfig buildPowerSpentSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = POWER_SPENT;

    return sensor;
}

SensorConfig buildWaterSpentSensor(int pin, long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin1 = pin;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = WATER_SPENT;

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

SensorConfig buildRssiSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = RSSI_SENSOR;

    return sensor;
}

SensorConfig buildMaxLoopTimeSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = MAX_LOOP_TIME;

    return sensor;
}

SensorConfig buildUpdateSensorTimeSensor(long timeout, String field_name, String debug_name) {
    SensorConfig sensor = SensorConfig();
    sensor.pin_count = 0;
    sensor.timeout = timeout;
    sensor.field_name = field_name;
    sensor.debug_name = debug_name;
    sensor.type = UPDATE_SENSORS_TIME;

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
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 28 44 04 A7 33 14 01 34
    DeviceAddress addrDino = {0x28, 0x44, 0x04, 0xA7, 0x33, 0x14, 0x01, 0x34};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D6, addrDino, 10000, "temp_in", "dallas1"),
        buildDhtSensor(D5, 22, 10000, "hum_in", "hum_in"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

// D6 dallas
// D5 D3 BME
DeviceConfig getColumbusConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "columbus";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 28B037E909000098
    DeviceAddress addrTempOut = {0x28, 0xB0, 0x37, 0xE9, 0x09, 0x00, 0x00, 0x98};

    // 2841F7E9090000FF
    DeviceAddress addrTempIn = {0x28, 0x41, 0xF7, 0xE9, 0x09, 0x00, 0x00, 0xFF};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D7, addrTempIn, 10000, "temp_in", "dallas1"),
        buildDallasSensor(D7, addrTempOut, 10000, "temp_out", "dallas2"),
        // buildBme280Sensor(D3, D5, {0x76}, 10000, "hum_in", "hum_in"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

DeviceConfig getWaveConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "wave";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58 * 1000 * 1;
    config.logSendTimeout = 58 * 1000 * 1;

    // 286CA5250A0000DA
    DeviceAddress addrTempIn = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D6, addrTempIn, 10000, "temp_in", "dallas1"),
        buildDhtSensor(D1, 22, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

std::vector<SensorConfig> getGeneralSensors() {
    std::vector<SensorConfig> generalSensors = {
        buildUptimeSensor(10000, "uptime", "uptime"),                               // ======================
        buildBuildVersionSensor(10000, "build_version", "build_version"),           // ======================
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),  // ======================
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),                // ======================
        buildRssiSensor(10000, "rssi", "rssi"),                                     // ======================
    };

    return generalSensors;
}

DeviceConfig getOpusConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "opus";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempIn = {0x28, 0x47, 0x60, 0x12, 0x33, 0x14, 0x01, 0x73};
    DeviceAddress addrTempOut = {0x28, 0xFF, 0x1D, 0xD6, 0x65, 0x18, 0x03, 0x38};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D6, addrTempIn, 10000, "temp_in", "temp_in"),
        buildDallasSensor(D6, addrTempOut, 10000, "temp_out", "temp_out"),
        buildDhtSensor(D1, 22, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),

        buildMaxLoopTimeSensor(10000, "max_loop_time", "max_loop_time"),
        buildUpdateSensorTimeSensor(10000, "max_update_sensor_time", "max_update_sensor_time")};

    std::vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();
    return config;
}

DeviceConfig getSunnyConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "sunny";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    // 286CA5250A0000DA
    // DeviceAddress addrTemp1 = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};
    // DeviceAddress addrTemp2 = {0x28, 0xAA, 0x7A, 0xB3, 0x1A, 0x13, 0x02, 0x61};

    std::vector<SensorConfig> sensorConfigs = {
        // buildDallasSensor(D6, addrTemp1, 10000, "temp1", "dallas1"),
        // buildDallasSensor(D6, addrTemp2, 10000, "temp2", "dallas2"),
        // buildDhtSensor(D1, 10000, "hum_in", "hum_in"),
        // buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
        buildIrmsSensor(30, 210, 10000, "irms", "irms"),
        buildPowerSensor(30, 210, 10000, "power", "power"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

DeviceConfig getChelConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "chel";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress newOut = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};
    DeviceAddress newIn = {0x28, 0xAA, 0x7A, 0xB3, 0x1A, 0x13, 0x02, 0x61};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D6, newIn, 10000, "temp_in", "temp_in"),
        buildDallasSensor(D6, newOut, 10000, "temp_out", "temp_out"),
        buildDhtSensor(D3, 22, 10000, "hum_in", "hum_in"),
        buildCo2SenseairSensor(D7, D8, 10000, "co2", "co2"),
    };

    std::vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();
    return config;
}

DeviceConfig getArcticConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "arctic";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress newOut = {0x28, 0xFF, 0x6E, 0x31, 0x20, 0x18, 0x01, 0x6C};
    // DeviceAddress newIn = {0x28, 0xFF, 0x1D, 0xD6, 0x65, 0x18, 0x03, 0x38};

    std::vector<SensorConfig> sensorConfigs = {
        // buildDallasSensor(D6, newIn, 10000, "temp_in", "temp_in"),
        buildDallasSensor(D6, newOut, 10000, "temp_out", "temp_out"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

DeviceConfig getMoxovichConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "moxovich";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempParilka = {0x28, 0xB0, 0xF8, 0x75, 0xD0, 0x01, 0x3C, 0x68};
    DeviceAddress addrTempMoxOut = {0x28, 0x5C, 0xCF, 0x95, 0xF0, 0x01, 0x3C, 0x34};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D1, addrTempParilka, 10000, "temp_in", "temp_in"),  // парилка
        buildDallasSensor(D1, addrTempMoxOut, 10000, "temp_out", "temp_out"),
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
        buildRssiSensor(10000, "rssi", "rssi"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

DeviceConfig getMoxovichSecondConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "moxovich_second";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempParilka = {0x28, 0x66, 0x8D, 0x75, 0xD0, 0x01, 0x3C, 0xA1};

    std::vector<SensorConfig> sensorConfigs = {
        buildDallasSensor(D1, addrTempParilka, 10000, "temp_in", "temp_in"),  // парилка
        buildUptimeSensor(10000, "uptime", "uptime"),
        buildBuildVersionSensor(10000, "build_version", "build_version"),
        buildFirmwareVersionSensor(10000, "firmware_version", "firmware_version"),
        buildFreeHeapVersionSensor(10000, "free_heap", "free_heap"),
        buildRssiSensor(10000, "rssi", "rssi"),
    };
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

    return config;
}

DeviceConfig getDimpleConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "dimple";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempIn1 = {0x28, 0xFF, 0x64, 0x01, 0xB0, 0x6D, 0x5F, 0x3C};

    std::vector<SensorConfig> sensorConfigs = {
        buildPowerSensor(30, 210, 10000, "power", "power"),
        buildPowerSpentSensor(10000, "power_spent", "power_spent"),
        buildDallasSensor(D1, addrTempIn1, 10000, "temp_in", "temp_in"),
    };

    std::vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();
    return config;
}

DeviceConfig getChelFlanerConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "chel_flaner";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    DeviceAddress addrTempIn = {0x28, 0x36, 0x4B, 0x95, 0xF0, 0xFF, 0x3C, 0xA9};
    DeviceAddress addrTempIn3 = {0x28, 0xFF, 0x42, 0x75, 0xD0, 0x01, 0x3C, 0xCB};

    std::vector<SensorConfig> sensorConfigs = {
        buildDhtSensor(D6, 11, 10000, "hum_in", "hum_in"),
        buildDallasSensor(D1, addrTempIn, 10000, "temp_in", "temp_in"),
        buildDallasSensor(D1, addrTempIn3, 10000, "temp_out", "temp_out"),
        buildWaterSpentSensor(D2, 10000, "water_spent", "water_spent"),
    };

    std::vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();
    return config;
}

DeviceConfig getChelOsmosConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "chel_osmos";
    config.sensorsApiUrl = SEND_DATA_URL + config.deviceName;
    config.sensorsSendTimeout = 58000;
    config.logSendTimeout = 58000;

    std::vector<SensorConfig> sensorConfigs = {
        buildWaterSpentSensor(D2, 10000, "water_spent", "water_spent"),
    };

    std::vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();
    return config;
}

DeviceConfig getStubConfig() {
    DeviceConfig config = DeviceConfig();
    config.deviceName = "undef";
    config.sensorsSendTimeout = 1e9;
    config.logSendTimeout = 1e9;

    const int SENSORS_COUNT = 0;
    std::vector<SensorConfig> sensorConfigs = {};
    config.sensors = sensorConfigs;
    config.sensorsCount = sensorConfigs.size();

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
        case 6500042:  // Sunny - этот чип сгорел, вместо sunny теперь arctic
            // return getSunnyConfig();
            return getStubConfig();
        case 8825236:  // chel
            return getChelConfig();
        case 902588:  // arctic - он теперь стал sunny, но на нем не работает монитор
            return getSunnyConfig();
            // return getArcticConfig();
        case 14967167:
            return getMoxovichConfig();
        case 10742575:
            return getMoxovichSecondConfig();
        case 6376678:  // chel electro measurer
            return getDimpleConfig();
        case 14921653:
            return getChelFlanerConfig();
        case 10690830:  // filter osmso chel
            return getChelOsmosConfig();
        default:
            return getStubConfig();
    }
}

#endif
