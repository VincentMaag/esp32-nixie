/*
    ...

*/
#ifndef __NIXIE_HV5622_H__
#define __NIXIE_HV5622_H__

#include <time.h>
#include <sys/time.h>

#include "maag_gpio.h"
#include "maag_spi_device.h"

typedef enum {
    HV5622_TIME = 0,
    HV5622_DATE
} hv5622_conversion_t;

// class for a (double) HV5622 Serial-to-Parallel Converter Module
class NixieHv5622 : public MaagSpiDevice
{
private:
    // gpio objects for hv5622
    GpioOutput m_gpio_BL;
    GpioOutput m_gpio_POL;

    // reverse all bits in a single byte
    uint8_t reverse8Bits(uint8_t b);

    // reverse all bits in a 64 bit variable
    uint64_t reverse64Bits(uint64_t b);

    // reverse all bits if each byte in a 16bit variable
    uint16_t reverseBitsOf2Bytes(uint16_t ui16TwoBytes_);

    // reverse all bits if each byte in a 64bit variable
    uint64_t reverseBitsOf8Bytes(uint64_t ui64EightBytes_);
    
    // get all 6 digits from a time struct
    void getTimeDigits(uint8_t * singleDigitArray_, struct tm time_);

    // get all 6 digits from a time struct
    void getDateDigits(uint8_t * singleDigitArray_, struct tm time_);

    // get n-th digit (starting from right --> LSB, i.e. n=0 equals firts digit) of a given number
    uint8_t getDigit(uint32_t number_, uint8_t n_);
    
    // convert all 6 digits from numbers to a bit sequence
    void getTimeDigitsAsBits(uint16_t* bitArray_, uint8_t * digitArray_, uint8_t nrOfDigits_);

    // convert one single number (0-9) to a 16-bit variable with exactly one bit set, which is the digith_-th bit
    uint16_t getDigitBits(uint8_t digit_);

    // build a 64 bit sequence from passed array and bit-length of each digit
    uint64_t get64BitSequence(uint16_t *digitBitsArray_, uint8_t nrOfDigits_, uint8_t bitsPerDigit_, uint8_t inverted_);

    // build a 64 bit sequence from current time or date
    uint64_t convertTo64BitSequence(struct tm time_, hv5622_conversion_t conversionType_);

    // write time or date to hv5622 spi device 
    esp_err_t writeToHv5622(struct tm time_, hv5622_conversion_t conversionType_);

public:
    NixieHv5622();

    // initialize additional gpios for blanking and polarity
    void initGpios(gpio_num_t gpio_bl_nr_, gpio_num_t gpio_pol_nr_);

    // write time, passed as argument, to serial-parallel-converter
    esp_err_t writeTimeToHv5622(struct tm time_);

    // write date, passed as argument, to serial-parallel-converter
    esp_err_t writeDateToHv5622(struct tm time_);

};

#endif /* __NIXIE_HV5622_H__ */