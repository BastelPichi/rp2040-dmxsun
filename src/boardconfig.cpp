#include "boardconfig.h"

#include "statusleds.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/flash.h>
#include <pico/unique_id.h>

extern StatusLeds statusLeds;
const uint8_t *config_flash_contents = (const uint8_t *) (XIP_BASE + CONFIG_FLASH_OFFSET);

ConfigData* BoardConfig::activeConfig;

void BoardConfig::init() {
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    // Pull-ups are populated on Rev 0.1 base boards
    // so the internal ones are not required. However, they don't hurt
    // and the plan is to remove the external ones
    gpio_pull_up(PIN_I2C_SCL);
    gpio_pull_up(PIN_I2C_SDA);
    memset(this->rawData, 0xff, 5*256);
}

void BoardConfig::readIOBoards() {
    this->responding[0] = false;
    this->responding[1] = false;
    this->responding[2] = false;
    this->responding[3] = false;

    for (int addr = 80; addr < 84; ++addr) {
        int ret;
        uint8_t src = 0;
        // Set EEPROM address to read from
        i2c_write_blocking(i2c0, addr, &src, 1, false);
        // Try to read the EEPROM data
        ret = i2c_read_blocking(i2c0, addr, this->rawData[addr - 80], 256, false);
        if (ret > 0) {
            this->responding[addr - 80] = true;
            if (this->rawData[addr - 80][0] == 0xff) {
                // EEPROM detected but content invalid => orange LED
                statusLeds.setLed(addr - 80, 255, 127, 0);
            } else {
                // EEPROM detected and content seems valid => green LED
                statusLeds.setLed(addr - 80, 0, 255, 0);
            }
        } else {
            // EEPROM not detected :(
            statusLeds.setLed(addr - 80, 255, 0, 0);
        }
    }
    statusLeds.writeLeds();
}

void BoardConfig::prepareConfig() {
    ConfigData* cfg00 = (ConfigData*)this->rawData[0];
    ConfigData* cfg01 = (ConfigData*)this->rawData[1];
    ConfigData* cfg10 = (ConfigData*)this->rawData[2];
    ConfigData* cfg11 = (ConfigData*)this->rawData[3];
    ConfigData* cfgbase = (ConfigData*)this->rawData[4];
    uint8_t r, g, b;

    // Also copy the data from the internal flash to RAM so it can be modified
    memcpy(this->rawData[4], config_flash_contents, 256);

    // Check if any board is connected and has a valid config
    // All IO boards in order, followed by baseboard
    if (
        (cfg00->boardType > BoardType::invalid_00) &&
        (cfg00->boardType < BoardType::invalid_ff) &&
        (cfg00->configVersion == CONFIG_VERSION)
    ) {
        BoardConfig::activeConfig = cfg00;
        statusLeds.getLed(0, &r, &g, &b);
        statusLeds.setLed(0, r, g, 255);
    } else if (
        (cfg01->boardType > BoardType::invalid_00) &&
        (cfg01->boardType < BoardType::invalid_ff) &&
        (cfg01->configVersion == CONFIG_VERSION)
    ) {
        BoardConfig::activeConfig = cfg01;
        statusLeds.getLed(1, &r, &g, &b);
        statusLeds.setLed(1, r, g, 255);
    } else if (
        (cfg10->boardType > BoardType::invalid_00) &&
        (cfg10->boardType < BoardType::invalid_ff) &&
        (cfg10->configVersion == CONFIG_VERSION)
    ) {
        BoardConfig::activeConfig = cfg10;
        statusLeds.getLed(2, &r, &g, &b);
        statusLeds.setLed(2, r, g, 255);
    } else if (
        (cfg11->boardType > BoardType::invalid_00) &&
        (cfg11->boardType < BoardType::invalid_ff) &&
        (cfg11->configVersion == CONFIG_VERSION)
    ) {
        BoardConfig::activeConfig = cfg11;
        statusLeds.getLed(3, &r, &g, &b);
        statusLeds.setLed(3, r, g, 255);
    } else if (
        (cfgbase->boardType > BoardType::invalid_00) &&
        (cfgbase->boardType < BoardType::invalid_ff) &&
        (cfgbase->configVersion == CONFIG_VERSION)
    ) {
        BoardConfig::activeConfig = cfgbase;
        statusLeds.getLed(4, &r, &g, &b);
        statusLeds.setLed(4, r, g, 255);
    } else {
        // We don't have any valid configuration at all :-O
        // Create a default one and use it for now
        // Since we don't know then nature of the IO boards, we save that
        // default config in the slot of the base board!
        *cfgbase = this->defaultConfig();
        BoardConfig::activeConfig = cfgbase;
        statusLeds.setLed(4, 255, 0, 255);
    }
    sleep_ms(200);
    statusLeds.writeLeds();
}

ConfigData BoardConfig::defaultConfig() {
    ConfigData cfg;

    memcpy(&cfg, &constDefaultConfig, sizeof(ConfigData));

    // Compute the second and third byte of the IP with values from
    // the unique board id: 10.X.Y.1 (board), 10.X.Y.2 (host)
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    memcpy(((void*)&(cfg.ownIp)) + 2, id.id, 1);
    memcpy(((void*)&(cfg.hostIp)) + 2, id.id, 1);

    return cfg;
}

int BoardConfig::saveConfig(uint8_t slot) {
    ConfigData* targetConfig = (ConfigData*)this->rawData[slot];

    if ((slot == 0) || (slot == 1) || (slot == 2) || (slot == 3)) {
        // Save to an IO board, so check if it's connected and configured
        if (
            (this->responding[slot]) &&
            (targetConfig->boardType > BoardType::invalid_00) &&
            (targetConfig->boardType < BoardType::invalid_ff)
        ) {
            // Save only the non-board-specific part
            uint8_t offset = sizeof(BoardType) + 4 * sizeof(PortParams);
            memcpy(targetConfig + offset, BoardConfig::activeConfig + offset, sizeof(ConfigData) - offset);
            // TODO: EEPROM writing (page-wise!)
            // TODO: Compare after writing
            return 0;
        } else if (!this->responding[slot]) {
            // IO board is not connected
            return 1;
        } else {
            // IO board is connected but not configured
            return 2;
        }
    } else if (slot == 4) {
        // Save to the base board
        memcpy(targetConfig, BoardConfig::activeConfig, sizeof(ConfigData));
        targetConfig->boardType = BoardType::baseboard_fallback;

        // Erase the flash sector
        // Note that a whole number of sectors must be erased at a time.
        flash_range_erase(CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

        // Program the flash sector with the new values
        flash_range_program(CONFIG_FLASH_OFFSET, (const uint8_t*)targetConfig, sizeof(ConfigData));

        // Compare that what should have been written has been written
        if (memcmp(targetConfig, config_flash_contents, sizeof(ConfigData))) {
            // Comparison failed :-O
            return 3;
        }

        // All good :)
        return 0;
    }

    // Slot unknown
    return 4;
}

uint8_t getUsbProtocol() {
    return BoardConfig::activeConfig->usbProtocol;
}

uint32_t getOwnIp() {
    return BoardConfig::activeConfig->ownIp;
}

uint32_t getOwnMask() {
    return BoardConfig::activeConfig->ownMask;
}

uint32_t getHostIp() {
    return BoardConfig::activeConfig->hostIp;
}

uint32_t getHostMask() {
    return BoardConfig::activeConfig->hostMask;
}
