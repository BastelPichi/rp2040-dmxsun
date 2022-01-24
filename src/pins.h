/**
 * @file
 * <pre>
 * GPIO pins used by the rp2040-dongle
 * GPIO numbers of the RP2040 chip,
 * NOT hardware pin numbers of the pico module!
 * All in one file for easy reference
 * </pre>
*/

#include <bsp/board.h>          // On-board-LED

// I2C to IO boards for detection and configuration
// (every IO board has an 2048 bit = 256 byte EEPROM)
//! I2C SCL (clock)
#define PIN_I2C_SCL     0
//! I2C SDA (data)
#define PIN_I2C_SDA     1

// SPI to the optional nRF24L01+ module
//! SPI CLK
#define PIN_SPI_CLK     2
//! SPI MOSI
#define PIN_SPI_MOSI    3
//! SPI MISO
#define PIN_SPI_MISO    4
//! SPI Chip select 0 (= nRF24 module)
#define PIN_SPI_CS0     5
//! nRF24 module chip enable
#define PIN_RF24_CE    28

// IO board 00
//! IO board 00, data line 0
#define PIN_IO00_0      6
//! IO board 00, data line 1
#define PIN_IO00_1      7
//! IO board 00, data line 2
#define PIN_IO00_2      8
//! IO board 00, data line 3
#define PIN_IO00_3      9

// IO board 01
//! IO board 01, data line 0
#define PIN_IO01_0     10
//! IO board 01, data line 1
#define PIN_IO01_1     11
//! IO board 01, data line 2
#define PIN_IO01_2     12
//! IO board 01, data line 3
#define PIN_IO01_3     13

// IO board 10
//! IO board 10, data line 0
#define PIN_IO10_0     14
//! IO board 10, data line 1
#define PIN_IO10_1     15
//! IO board 10, data line 2
#define PIN_IO10_2     16
//! IO board 10, data line 3
#define PIN_IO10_3     17

// IO board 11
//! IO board 11, data line 0
#define PIN_IO11_0     18
//! IO board 11, data line 1
#define PIN_IO11_1     19
//! IO board 11, data line 2
#define PIN_IO11_2     20
//! IO board 11, data line 3
#define PIN_IO11_3     21

//! Pico's on-board, single-color status LED
#define PIN_LED        PICO_DEFAULT_LED_PIN // = 25 on the pico

//! WS2812-based status LEDs
#define PIN_LEDS       22

//! Helper pin for DMX TX (DriverEnable output)
//! this is mainly useful to trigger an oscilloscope to check the
//! generated DMX frames
#define PIN_TRIGGER    26
