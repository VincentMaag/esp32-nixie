/*
    ...

*/
#ifndef __NIXIE_HV5622_H__
#define __NIXIE_HV5622_H__

#include <time.h>
#include <sys/time.h>

#include "maag_gpio.h"
#include "maag_spi_device.h"




// class for a (double) HV5622 Serial-to-Parallel Converter Module
class NixieHv5622 : public MaagSpiDevice
{
private:
    // gpio objects for hv5622
    GpioOutput m_gpio_BL;
    GpioOutput m_gpio_POL;

    uint8_t reverseBits(uint8_t b);
    // reverse all bits if each byte in a 16bit variable
    uint16_t reverseBitsOf2Bytes(uint16_t ui16TwoBytes_);
    // reverse all bits if each byte in a 64bit variable
    uint64_t reverseBitsOf8Bytes(uint64_t ui64EightBytes_);
    // get n-th digit (starting from right --> LSB, i.e. n=0 equals firts digit) of a given number
    uint8_t getDigit(uint32_t number_, uint8_t n_);
    // convert one single number (0-9) to a 16-bit variable with exactly one bit set, which is the digith_-th bit
    uint16_t getDigitBits(uint8_t digit_);
    // build a 64 bit sequence from passed array and bit-length of each digit
    uint64_t get64BitSequence(uint16_t *digitBitsArray_, uint8_t nrOfDigits_, uint8_t bitsPerDigit_, uint8_t inverted_);
    // build a 64 bit sequence from current time
    uint64_t timeTo64BitSequence(struct tm time_);

public:
    NixieHv5622();
    // initialize needed gpios for latch, blanking and polarity
    void initGpios(gpio_num_t gpio_bl_nr_, gpio_num_t gpio_pol_nr_);
    // write time, passed as argument, to serial-üarallel-converter
    esp_err_t writeTimeToHv5622(struct tm time_);

};

#endif /* __NIXIE_HV5622_H__ */