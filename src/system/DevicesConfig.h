#ifndef DEVICES_CONFIG_h
#define DEVICES_CONFIG_h

#include "SensorConfig.h"

using std::vector;

struct DeviceConfig {
    String deviceName;
    String sensorsApiUrl;

    int sensorsCount;
    vector<SensorConfig> sensors;

    DeviceConfig() {}
    DeviceConfig(String const& _deviceName, String const& _url) {
        this->deviceName = _deviceName;
        this->sensorsApiUrl = _url + _deviceName;
    }
};

vector<SensorConfig> getGeneralSensors() {
    vector<SensorConfig> generalSensors = {
        buildUptimeSensor("uptime"),                     // ======================
        buildBuildVersionSensor("build_version"),        // ======================
        buildFirmwareVersionSensor("firmware_version"),  // ======================
        buildFreeHeapVersionSensor("free_heap"),         // ======================
        buildRssiSensor("rssi"),                         // ======================
    };

    return generalSensors;
}

void updateSensors(DeviceConfig& deviceConfig, vector<SensorConfig>& sensorConfigs) {
    vector<SensorConfig> generalSensors = getGeneralSensors();
    sensorConfigs.insert(sensorConfigs.end(), generalSensors.begin(), generalSensors.end());

    deviceConfig.sensors = sensorConfigs;
    deviceConfig.sensorsCount = sensorConfigs.size();
}

DeviceConfig genConfig(String const& _deviceName, String const& _url, vector<SensorConfig>& sensorConfigs) {
    DeviceConfig config = DeviceConfig(_deviceName, _url);
    updateSensors(config, sensorConfigs);
    return config;
}

DeviceConfig getDinoConfig(String const& deviceName) {
    DeviceAddress addrDino = {0x28, 0x44, 0x04, 0xA7, 0x33, 0x14, 0x01, 0x34};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D6, addrDino, "temp_in"),
        buildDhtSensor(D5, 22, "hum_in"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getColumbusConfig(String const& deviceName) {
    DeviceAddress addrTempOut = {0x28, 0xB0, 0x37, 0xE9, 0x09, 0x00, 0x00, 0x98};
    DeviceAddress addrTempIn = {0x28, 0x41, 0xF7, 0xE9, 0x09, 0x00, 0x00, 0xFF};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D7, addrTempIn, "temp_in"),
        buildDallasSensor(D7, addrTempOut, "temp_out"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getWaveConfig(String const& deviceName) {
    DeviceAddress addrTempIn = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D6, addrTempIn, "temp_in"),
        buildDhtSensor(D1, 22, "hum_in"),
        buildCo2SenseairSensor(D7, D8, "co2"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getOpusConfig(String const& deviceName) {
    DeviceAddress addrTempIn = {0x28, 0x47, 0x60, 0x12, 0x33, 0x14, 0x01, 0x73};
    DeviceAddress addrTempOut = {0x28, 0xFF, 0x1D, 0xD6, 0x65, 0x18, 0x03, 0x38};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D6, addrTempIn, "temp_in"),
        buildDallasSensor(D6, addrTempOut, "temp_out"),
        buildDhtSensor(D1, 22, "hum_in"),
        buildCo2SenseairSensor(D7, D8, "co2"),
        buildMaxLoopTimeSensor("max_loop_time"),
        buildUpdateSensorTimeSensor("max_update_sensor_time"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getSunnyConfig(String const& deviceName) {
    vector<SensorConfig> sensors = {
        buildIrmsSensor(30, 210, "irms"),
        buildPowerSensor(30, 210, "power"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getChelConfig(String const& deviceName) {
    DeviceAddress newOut = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};
    DeviceAddress newIn = {0x28, 0xAA, 0x7A, 0xB3, 0x1A, 0x13, 0x02, 0x61};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D6, newIn, "temp_in"),
        buildDallasSensor(D6, newOut, "temp_out"),
        buildDhtSensor(D3, 22, "hum_in"),
        buildCo2SenseairSensor(D7, D8, "co2"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getArcticConfig(String const& deviceName) {
    DeviceAddress newOut = {0x28, 0xFF, 0x6E, 0x31, 0x20, 0x18, 0x01, 0x6C};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D6, newOut, "temp_out"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getMoxovichConfig(String const& deviceName) {
    DeviceAddress addrTempParilka = {0x28, 0xB0, 0xF8, 0x75, 0xD0, 0x01, 0x3C, 0x68};
    DeviceAddress addrTempMoxOut = {0x28, 0x5C, 0xCF, 0x95, 0xF0, 0x01, 0x3C, 0x34};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D1, addrTempParilka, "temp_in"),
        buildDallasSensor(D1, addrTempMoxOut, "temp_out"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getMoxovichSecondConfig(String const& deviceName) {
    DeviceAddress addrTempParilka = {0x28, 0x66, 0x8D, 0x75, 0xD0, 0x01, 0x3C, 0xA1};

    vector<SensorConfig> sensors = {
        buildDallasSensor(D1, addrTempParilka, "temp_in"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getDimpleConfig(String const& deviceName) {
    DeviceAddress addrTempIn1 = {0x28, 0xFF, 0x64, 0x01, 0xB0, 0x6D, 0x5F, 0x3C};

    vector<SensorConfig> sensors = {
        buildPowerSensor(30, 210, "power"),
        buildPowerSpentSensor("power_spent"),
        buildDallasSensor(D1, addrTempIn1, "temp_in"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getChelFlanerConfig(String const& deviceName) {
    DeviceAddress addrTempIn = {0x28, 0x36, 0x4B, 0x95, 0xF0, 0xFF, 0x3C, 0xA9};
    DeviceAddress addrTempIn3 = {0x28, 0xFF, 0x42, 0x75, 0xD0, 0x01, 0x3C, 0xCB};

    vector<SensorConfig> sensors = {
        buildDhtSensor(D6, 11, "hum_in"),
        buildDallasSensor(D1, addrTempIn, "temp_in"),
        buildDallasSensor(D1, addrTempIn3, "temp_out"),
        buildWaterSpentSensor(D2, "water_spent"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getChelOsmosConfig(String const& deviceName) {
    vector<SensorConfig> sensors = {
        buildWaterSpentSensor(D2, "water_spent"),
    };

    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

DeviceConfig getStubConfig(String const& deviceName) {
    vector<SensorConfig> sensors = {};
    return genConfig(deviceName, SEND_DATA_URL, sensors);
}

String getDeviceNameById(int id) {
    switch (id) {
        case 45266:  // Dino
            return "dino";
        case 3741649:  // Columbus
            return "columbus";
        case 8825711:  // Wave (usually at work)
            return "wave";
        case 6499365:  // Opus
            return "opus";
        case 6500042:  // Sunny - этот чип сгорел, вместо sunny теперь arctic
            // return getSunnyConfig();
            return "undef";
        case 8825236:  // chel
            return "chel";
        case 902588:  // arctic - он теперь стал sunny, но на нем не работает монитор
            return "sunny";
            // return getArcticConfig();
        case 14967167:  // Moxovich
            return "moxovich";
        case 10742575:  // Moxovich_second
            return "moxovich_second";
        case 6376678:  // dimple chel electro measurer
            return "dimple";
        case 14921653:  // chel_flaner
            return "chel_flaner";
        case 10690830:  // chel_osmos filter osmso chel
            return "chel_osmos";
        default:
            return "undef";
    }
}

DeviceConfig getDeviceConfigByName(String const& deviceName) {
    if (deviceName == "opus") {
        return getOpusConfig(deviceName);
    }
    if (deviceName == "chel") {
        return getChelConfig(deviceName);
    }
    if (deviceName == "chel_osmos") {
        return getChelOsmosConfig(deviceName);
    }
    if (deviceName == "chel_flaner") {
        return getChelFlanerConfig(deviceName);
    }
    if (deviceName == "dimple") {
        return getDimpleConfig(deviceName);
    }

    return getStubConfig(deviceName);
}

DeviceConfig getDeviceConfigById(int id) {
    String deviceName = getDeviceNameById(id);

    switch (id) {
        case 45266:  // Dino
            return getDinoConfig(deviceName);
        case 3741649:  // Columbus
            return getColumbusConfig(deviceName);
        case 8825711:  // Wave (usually at work)
            return getWaveConfig(deviceName);
        // case 6499365:  // Opus
        //     return getOpusConfig(deviceName);
        case 6500042:  // Sunny - этот чип сгорел, вместо sunny теперь arctic
            // return getSunnyConfig();
            return getStubConfig(deviceName);
        // case 8825236:  // chel
        //     return getChelConfig(deviceName);
        case 902588:  // arctic - он теперь стал sunny, но на нем не работает монитор
            return getSunnyConfig(deviceName);
            // return getArcticConfig();
        case 14967167:  // Moxovich
            return getMoxovichConfig(deviceName);
        case 10742575:  // Moxovich_second
            return getMoxovichSecondConfig(deviceName);
        // case 6376678:  // dimple chel electro measurer
        //     return getDimpleConfig(deviceName);
        // case 14921653:  // chel_flaner
        //     return getChelFlanerConfig(deviceName);
        // case 10690830:  // chel_osmos filter osmso chel
        //     return getChelOsmosConfig(deviceName);
        default:
            return getDeviceConfigByName(deviceName);
    }
}

#endif
