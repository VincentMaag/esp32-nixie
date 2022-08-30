/*
    ...

*/
#ifndef __NIXIE_HV5622_H__
#define __NIXIE_HV5622_H__

#include "maag_gpio.h"
#include "maag_spi_device.h"





// class for a (double) HV5622 Serial-to-Parallel Converter Module
class NixieHv5622 : public MaagSpiDevice
{
private:
    
    




public:
    NixieHv5622();

    // function list
    /*
    - initialize LE, BL, POL as GPIO outputs. Pass these gpio pins in constructor, use an init() function to create/init gpios
        - LE is actually the chip select!
    - convert() function that converts number up to 9 into relevant bits
    - setTime() that takes hours, minutes, seconds and creates 64 bit array
    - write() function that does the latching, writing to spi, de-latching



    */




};










#endif /* __NIXIE_HV5622_H__ */