/*
    ...

*/
#ifndef __MAAG_GPIO_H__
#define __MAAG_GPIO_H__

#include "driver/gpio.h"

// Example: EspGpio gpioOut(GPIO_NUM_15, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);
class EspGpio
{
private:
    gpio_num_t m_gpio_nr;

public:
    EspGpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type);

    EspGpio(gpio_num_t gpio_nr, gpio_mode_t mode);

    ~EspGpio();

    bool getInput();

    esp_err_t setOutput(bool out_);
};

#endif /* __MAAG_GPIO_H__ */