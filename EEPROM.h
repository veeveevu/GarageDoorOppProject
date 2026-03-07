//
// Created by vuhav on 04/03/2026.
//

#ifndef GARAGE_DOOR_EEPROM_H
#define GARAGE_DOOR_EEPROM_H

#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

constexpr uint8_t  EEPROM_I2C_ADDR   = 0x50;
constexpr uint     I2C_SCL_PIN       = 17;
constexpr uint     I2C_SDA_PIN       = 16;

constexpr uint16_t LOG_START_ADDR    = 0;
constexpr uint     LOG_ENTRY_SIZE    = 64;
constexpr uint     LOG_MAX_ENTRIES   = 32;
constexpr uint     LOG_MAX_STR_LEN   = 61;   // 64 - 1 null - 2 crc

bool eeprom_write_multi(uint16_t addr, const uint8_t* data, size_t len);
bool eeprom_read_multi(uint16_t addr, uint8_t* buffer, size_t len);
bool is_valid_log_entry(uint16_t entry_addr);
void eeprom_log_init();
void eeprom_log_write(const char* message);
void eeprom_log_read_and_print();
void eeprom_log_erase();

#endif //GARAGE_DOOR_EEPROM_H