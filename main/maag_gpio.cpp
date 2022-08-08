/*
	ToDo:

	- create sub-classes for:
		- input
		- output
		- input/output

		... so that getIn(), setOut() are clear when and how they can be set, i.e. seen!


*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"

#include "esp_event_loop.h"
#include "esp_log.h"

//#include "driver/gpio.h"

#include "maag_gpio.h"

// static TAG
static const char *TAG = "esp_gpio";

// =============================================================================================================
// CLASS ESPGPIO
// =============================================================================================================

// constructor if we want to configure completely
EspGpio::EspGpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type)
{
	// store gpio number
	m_gpio_nr = gpio_nr;
	// configure and set gpio
	gpio_config_t io_conf;
	io_conf.intr_type = intr_type;
	io_conf.mode = mode;
	io_conf.pin_bit_mask = (1ULL << gpio_nr);
	io_conf.pull_down_en = pull_down_en;
	io_conf.pull_up_en = pull_up_en;
	gpio_config(&io_conf);
}
// constructor if only number and mode are set
EspGpio::EspGpio(gpio_num_t gpio_nr, gpio_mode_t mode)
{
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = mode;
	io_conf.pin_bit_mask = (1ULL << gpio_nr);
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);
}

EspGpio::~EspGpio()
{
}

bool EspGpio::getInput()
{
	return (bool)gpio_get_level(m_gpio_nr);
}

esp_err_t EspGpio::setOutput(bool out_)
{
	return gpio_set_level(m_gpio_nr,(uint32_t)out_);
}
