/*
	...


*/

#include <string.h>
#include <math.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nixie_hv5622.h"

// static TAG
static const char *TAG = "nixie_hv5622";

// =============================================================================================================
// CLASS HV5622
// =============================================================================================================
NixieHv5622::NixieHv5622()
{
	ESP_LOGW(TAG, "NixieHv5622 instance created");
}

void NixieHv5622::initGpios(gpio_num_t gpio_bl_nr_, gpio_num_t gpio_pol_nr_)
{
	// init gpios
	m_gpio_BL.initAsOutput(gpio_bl_nr_, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_ENABLE);
	m_gpio_POL.initAsOutput(gpio_pol_nr_, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_ENABLE);
	// set Blank and polarity high
	m_gpio_BL.setOutput(1);
	m_gpio_POL.setOutput(1);
	// just to make sure, set chip select to low, becasue it is our latch
	MaagSpiDevice::select();
}

uint8_t NixieHv5622::getDigit(uint32_t number_, uint8_t n_)
{
	return (number_ / (uint32_t)powf(10, n_)) % 10;
}

uint16_t NixieHv5622::getDigitBits(uint8_t digit_)
{

	// digit zero is the 10th bit
	if (digit_ < 1)
	{
		return 1 << 9;
	}
	return 1 << (digit_ - 1);
}

uint64_t NixieHv5622::get64BitSequence(uint16_t *digitBitsArray_, uint8_t nrOfDigits_, uint8_t bitsPerDigit_, uint8_t inverted_)
{
	uint64_t bits = 0;
	uint64_t Tempbits = 0;

	// build up all single digits (as bits) to one 64 bit string
	for (uint8_t i = 0; i < nrOfDigits_; i++)
	{
		Tempbits = 0;
		Tempbits = digitBitsArray_[i];
		bits = bits | (Tempbits << (bitsPerDigit_ * i));
	}

	return bits;
}

uint64_t NixieHv5622::timeTo64BitSequence(struct tm time_)
{
	// we have 6 digits hh:mm:ss
	uint8_t nrOfDigits = 6;
	// array to store the single digits
	uint8_t singleDigitArray[nrOfDigits] = {};
	// array to store correctly manipulated single digits as bits
	uint16_t bitArray[nrOfDigits] = {};

	// first get all single digits, starting with seconds, minutes, hours --> [h][h][min][min][s][s]
	// we need a 16bit variable for this because there are a max of 10bits per digit
	singleDigitArray[0] = NixieHv5622::getDigit(time_.tm_sec, 0);
	singleDigitArray[1] = NixieHv5622::getDigit(time_.tm_sec, 1);
	singleDigitArray[2] = NixieHv5622::getDigit(time_.tm_min, 0);
	singleDigitArray[3] = NixieHv5622::getDigit(time_.tm_min, 1);
	singleDigitArray[4] = NixieHv5622::getDigit(time_.tm_hour, 0);
	singleDigitArray[5] = NixieHv5622::getDigit(time_.tm_hour, 1);
	// ESP_LOGE(TAG, "The Array of Single digits ([5][4][3][2][1][0]): %i,%i,%i,%i,%i,%i", singleDigitArray[5],singleDigitArray[4],singleDigitArray[3],singleDigitArray[2],singleDigitArray[1],singleDigitArray[0]);

	// next, we get the bit matching the digit-number: 1 = 0000 0000 0000 0001, 9 = 0000 0001 0000 0000 and so on
	for (int i = 0; i < nrOfDigits; i++)
	{
		bitArray[i] = NixieHv5622::getDigitBits(singleDigitArray[i]);
	}
	// ESP_LOGE(TAG, "The Array of Bits in Order ([5][4][3][2][1][0]): %i,%i,%i,%i,%i,%i", bitArray[5],bitArray[4],bitArray[3],bitArray[2],bitArray[1],bitArray[0]);

	// next we build up the bits-sequence, mapping every 10 bits of each digit together to a 64bits representation
	uint64_t sequence = NixieHv5622::get64BitSequence(bitArray, 6, 10, 0);
	// ESP_LOGE(TAG, "The Sequence looks like this: %llu", sequence);

	// next, I think because our chip is little endian, we need to swap every bit of each single byte
	// so that the bits are in fact written how ea actually want them to arrive at the serial-parallel converter
	uint64_t sequenceManipulated = NixieHv5622::reverseBitsOf8Bytes(sequence);
	// ESP_LOGE(TAG, "The Reversed Sequence looks like this: %llu", sequenceManipulated);
	return sequenceManipulated;
}

uint8_t NixieHv5622::reverseBits(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint16_t NixieHv5622::reverseBitsOf2Bytes(uint16_t ui16TwoBytes_)
{
	// extract first byte (LSB)
	uint8_t firstByte = (uint8_t)(ui16TwoBytes_ & 0x00FF);
	// extract second byte (MSB)
	uint8_t secondByte = (uint8_t)((ui16TwoBytes_ & 0xFF00) >> 8);
	// reverse both bit sequences
	uint16_t firstByteReversed = (uint16_t)NixieHv5622::reverseBits(firstByte);
	uint16_t secondByteReversed = (uint16_t)NixieHv5622::reverseBits(secondByte);
	// put them back together again and return result
	return ((secondByteReversed << 8) & 0xFF00) | (firstByteReversed & 0x00FF);
}

uint64_t NixieHv5622::reverseBitsOf8Bytes(uint64_t ui64EightBytes_)
{
	// get a pointer that looks at first address of our 64 bit variable
	uint8_t *pPeak = (uint8_t *)&ui64EightBytes_;
	// we now do the reversing on each byte seperately
	for (uint8_t i = 0; i < 8; i++)
	{
		pPeak[i] = NixieHv5622::reverseBits(pPeak[i]);
	}
	// cast our peak pointer to a 64bit pointer and return value at said memory address
	return *((uint64_t *)pPeak);
}

esp_err_t NixieHv5622::writeTimeToHv5622(struct tm time_)
{
	// convert time to 64 bits sequence
	uint64_t sequence = NixieHv5622::timeTo64BitSequence(time_);
	// polarity and blank are set high
	m_gpio_POL.setOutput(1);
	m_gpio_BL.setOutput(1);
	// chip-select, which is our latch, is set low
	NixieHv5622::select();
	// just to be sure, wait a very short time
	vTaskDelay(1);
	// write the bit sequence sequence to spi bus
	esp_err_t ok = NixieHv5622::write_bytes((uint8_t *)&sequence, 64);
	// wait again
	vTaskDelay(1);
	// activate by pulling latch high
	NixieHv5622::release();
	// ESP_LOGE(TAG, "total sequence: %llu", sequence);
	return ESP_OK;
}