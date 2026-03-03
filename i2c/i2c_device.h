//
// Created by Keijo Länsikunnas on 4.3.2025.
//

#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include <cstdint>

class i2c_device {
public:
    virtual unsigned int write(const uint8_t *buffer, unsigned int length) = 0;
    virtual unsigned int read(uint8_t *buffer, unsigned int length) = 0;
    virtual unsigned int transaction(const uint8_t *wbuffer, unsigned int wlength, uint8_t *rbuffer, unsigned int rlength) = 0;
    virtual ~i2c_device() = default;
};



#endif //I2C_DEVICE_H
