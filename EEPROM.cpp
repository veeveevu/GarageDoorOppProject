//
// Created by vuhav on 04/03/2026.
//

#include "EEPROM.h"
#include <cstring>
#include <cstdio>

/*Usage:
//in main:
eeprom_log_init();
//in state machine:
eeprom_log_write("Entered STATE_SOMETHING");
//debug:
int ch = getchar_timeout_us(0);

if (ch == 'r') {
    printf("=== LOG ===\n");
    eeprom_log_read_and_print();
}

else if (ch == 'e') {
    eeprom_log_erase();
    printf("Log erased.\n");
}
*/

static bool eeprom_write_multi(uint16_t addr, const uint8_t* data, size_t len) {
    if (len == 0 || len > 64) return false;

    uint8_t buf[66];
    buf[0] = static_cast<uint8_t>(addr >> 8);
    buf[1] = static_cast<uint8_t>(addr & 0xFF);
    std::memcpy(buf + 2, data, len);

    int ret = i2c_write_blocking(i2c0, EEPROM_I2C_ADDR, buf, len + 2, false);
    sleep_ms(5);  // EEPROM write cycle time
    return ret == static_cast<int>(len + 2);
}

static bool eeprom_read_multi(uint16_t addr, uint8_t* buffer, size_t len) {
    if (len == 0)
    {
        return false;
    }

    uint8_t reg[2] = {
        static_cast<uint8_t>(addr >> 8),
        static_cast<uint8_t>(addr & 0xFF)
    };

    if (i2c_write_blocking(i2c0, EEPROM_I2C_ADDR, reg, 2, true) != 2) {
        return false;
    }

    return i2c_read_blocking(i2c0, EEPROM_I2C_ADDR, buffer, len, false) == static_cast<int>(len);
}

static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    while (len--) {
        uint8_t x = (crc >> 8) ^ *data++;
        x ^= (x >> 4);
        crc = (crc << 8) ^
              (static_cast<uint16_t>(x) << 12) ^
              (static_cast<uint16_t>(x) << 5)  ^
               static_cast<uint16_t>(x);
    }
    return crc;
}

static bool is_valid_log_entry(uint16_t entry_addr) {
    uint8_t buf[LOG_ENTRY_SIZE]{};
    if (!eeprom_read_multi(entry_addr, buf, LOG_ENTRY_SIZE)) {
        return false;
    }

    if (buf[0] == 0) return false;

    size_t str_len = 0;

    while (str_len < LOG_MAX_STR_LEN && buf[str_len] != 0) {
        ++str_len;
    }

    if (str_len == LOG_MAX_STR_LEN) {
        return false;
    }

    uint16_t crc_calc = crc16(buf, str_len + 1);
    uint16_t crc_stored = (buf[str_len + 1] << 8) | buf[str_len + 2];

    return crc_calc == crc_stored;
}

static int find_next_free_slot() {
    for (int i = 0; i < LOG_MAX_ENTRIES; ++i) {
        uint16_t addr = LOG_START_ADDR + i * LOG_ENTRY_SIZE;
        if (!is_valid_log_entry(addr)) {
            return i;
        }
    }
    return -1;
}

void eeprom_log_init()
{
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
}

void eeprom_log_write(const char* message) {
    size_t len = strlen(message);
    if (len == 0 || len > LOG_MAX_STR_LEN) return;

    int slot = find_next_free_slot();
    if (slot == -1) {
        eeprom_log_erase();
        slot = 0;
    }

    uint16_t addr = LOG_START_ADDR + slot * LOG_ENTRY_SIZE;
    uint8_t buf[LOG_ENTRY_SIZE]{};

    std::memcpy(buf, message, len);
    buf[len] = 0;

    uint16_t crc = crc16(buf, len + 1);
    buf[len + 1] = crc >> 8;
    buf[len + 2] = crc & 0xFF;

    eeprom_write_multi(addr, buf, len + 3);
}

void eeprom_log_read_and_print() {
    for (int i = 0; i < LOG_MAX_ENTRIES; ++i) {
        uint16_t addr = LOG_START_ADDR + i * LOG_ENTRY_SIZE;
        if (!is_valid_log_entry(addr)) break;

        uint8_t buf[LOG_ENTRY_SIZE]{};
        if (eeprom_read_multi(addr, buf, LOG_ENTRY_SIZE)) {
            printf("%s\n", reinterpret_cast<const char*>(buf));
        }
    }
}

void eeprom_log_erase() {
    for (int i = 0; i < LOG_MAX_ENTRIES; ++i) {
        uint16_t addr = LOG_START_ADDR + i * LOG_ENTRY_SIZE;
        eeprom_write_multi(addr, reinterpret_cast<const uint8_t*>("\0"), 1);
    }
}
