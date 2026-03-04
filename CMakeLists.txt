# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.6)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(garage-door C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

#add_compile_definitions(MQTTCLIENT_QOS2=1)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)
file(GLOB MQTT_SOURCES "paho.mqtt.embedded-c/MQTTPacket/src/*.c")

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
        main.cpp
        uart/PicoUart.cpp
        uart/PicoUart.h
        uart/RingBuffer.cpp
        uart/RingBuffer.h
        mqtt/Countdown.cpp
        mqtt/Countdown.h
        mqtt/IPStack.cpp
        mqtt/IPStack.h
        mqtt/lwipopts.h
        ${MQTT_SOURCES}
        i2c/PicoI2CDevice.cpp
        i2c/PicoI2CDevice.h
        i2c/PicoI2CBus.cpp
        i2c/PicoI2CBus.h
        spi/PicoSPIBus.cpp
        spi/PicoSPIBus.h
        spi/PicoSPIDevice.cpp
        spi/PicoSPIDevice.h
        i2c/i2c_device.h
        spi/spi_device.h
        GarageDoorController.cpp
        GarageDoorController.h
        StepperMotor.cpp
        StepperMotor.h
        RotaryEncoder.cpp
        RotaryEncoder.h
        LimitSwitch.cpp
        LimitSwitch.h
        Button.cpp
        Button.h
        Led.cpp
        Led.h
        MQTTCom.cpp
        MQTTCom.h
        EEPROM.cpp
        EEPROM.h
        DoorState.h
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# this adds a post build command that tries to copy uf2 to Pico
#add_custom_command(TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2 d: )

target_include_directories(${PROJECT_NAME} PRIVATE
        mqtt
        uart
        i2c
        spi
        paho.mqtt.embedded-c/MQTTPacket/src
        paho.mqtt.embedded-c/MQTTClient/src
)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
        pico_stdlib
        hardware_i2c
        hardware_spi
        hardware_gpio
        pico_cyw43_arch_lwip_poll
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)