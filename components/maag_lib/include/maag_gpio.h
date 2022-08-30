/*
    ...

*/
#ifndef __MAAG_GPIO_H__
#define __MAAG_GPIO_H__

#include "driver/gpio.h"

// =============================================================================================================
// Example: EspGpio gpioOut(GPIO_NUM_15, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);
class Gpio
{
private:

    gpio_mode_t m_mode;
    gpio_pulldown_t m_pull_down_en;
    gpio_pullup_t m_pull_up_en;
    gpio_int_type_t m_intr_type;
    
protected:
    // gpio channel number
    gpio_num_t m_gpio_nr;
    // configure a gpio
    void configure_gpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type);

public:
    Gpio();
    // constructor if we want to configure gpio individually
    Gpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type);
    // get channel number
    gpio_num_t getChannelNr();

};
// =============================================================================================================
class GpioInput : public Gpio
{
private:
    /* data */

public:
    GpioInput();
    // configure pio channel nr and pullup/down
    GpioInput(gpio_num_t gpio_nr);
    // configure pio channel nr and pullup/down
    GpioInput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en);
    // configure/init an Input manually
    void initAsInput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en);
    // get input value
    bool getInput();

};

// =============================================================================================================
class GpioOutput : public Gpio
{
private:
    /* data */
public:
    GpioOutput();
    // configure pio channel nr and pullup/down
    GpioOutput(gpio_num_t gpio_nr);
    // configure pio channel nr and pullup/down
    GpioOutput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en);
    // configure/init an Output manually
    void initAsOutput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en);
    // set output value
    esp_err_t setOutput(bool out);
};













#endif /* __MAAG_GPIO_H__ */