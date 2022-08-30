/*
	ToDo:

	- create sub-classes for:
		- input
		- output
		- input/output

		... so that getIn(), setOut() are clear when and how they can be set, i.e. seen!


*/

#include "esp_log.h"
#include "maag_gpio.h"

// static TAG
static const char *TAG = "esp_gpio";

// =============================================================================================================
// CLASS GPIO
// =============================================================================================================
Gpio::Gpio()
{
}

Gpio::Gpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type)
{
	configure_gpio(gpio_nr, mode, pull_down_en, pull_up_en, intr_type);
}

void Gpio::configure_gpio(gpio_num_t gpio_nr, gpio_mode_t mode, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en, gpio_int_type_t intr_type)
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

gpio_num_t Gpio::getChannelNr()
{
	return m_gpio_nr;
}

// =============================================================================================================
// CLASS GPIOINPUT
// =============================================================================================================
GpioInput::GpioInput()
{
}

GpioInput::GpioInput(gpio_num_t gpio_nr)
{
	configure_gpio(gpio_nr, GPIO_MODE_INPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);
}

GpioInput::GpioInput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en)
{
	configure_gpio(gpio_nr, GPIO_MODE_INPUT, pull_down_en, pull_up_en, GPIO_INTR_DISABLE);
}

void GpioInput::initAsInput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en){
	configure_gpio(gpio_nr, GPIO_MODE_INPUT, pull_down_en, pull_up_en, GPIO_INTR_DISABLE);
}

bool GpioInput::getInput(){
	return (bool)gpio_get_level(Gpio::m_gpio_nr);
}

// =============================================================================================================
// CLASS GPIOOUTPUT
// =============================================================================================================
GpioOutput::GpioOutput()
{
}

GpioOutput::GpioOutput(gpio_num_t gpio_nr)
{
	configure_gpio(gpio_nr, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);
}

GpioOutput::GpioOutput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en)
{
	configure_gpio(gpio_nr, GPIO_MODE_OUTPUT, pull_down_en, pull_up_en, GPIO_INTR_DISABLE);
}

void GpioOutput::initAsOutput(gpio_num_t gpio_nr, gpio_pulldown_t pull_down_en, gpio_pullup_t pull_up_en){
	
	configure_gpio(gpio_nr, GPIO_MODE_OUTPUT, pull_down_en, pull_up_en, GPIO_INTR_DISABLE);
}

esp_err_t GpioOutput::setOutput(bool out){
	return gpio_set_level(Gpio::m_gpio_nr,(uint32_t)out);
}






