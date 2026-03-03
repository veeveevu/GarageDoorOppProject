//
// Created by Keijo Länsikunnas on 6.3.2025.
//

#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include <cstdint>

class spi_device {
public:
    virtual unsigned int write(const uint8_t *buffer, unsigned int length) = 0;
    virtual unsigned int read(uint8_t *buffer, unsigned int length) = 0;
    virtual unsigned int transaction(const uint8_t *wbuffer, uint8_t *rbuffer, unsigned int length) = 0;
    virtual void set_cs(bool cs) = 0;
    virtual ~spi_device() = default;
};



#endif //SPI_DEVICE_H
