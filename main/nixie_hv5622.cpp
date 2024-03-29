/*
	...


*/

#include <string.h>
#include <math.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"

#include "nixie_hv5622.h"

// static TAG
static const char *TAG = "nixie_hv5622";


// =============================================================================================================
// CLASS HV5622 - Public methods
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

esp_err_t NixieHv5622::writeTimeToHv5622(struct tm time_)
{
	return NixieHv5622::writeToHv5622(time_, HV5622_TIME);
}

esp_err_t NixieHv5622::writeDateToHv5622(struct tm time_)
{
	return NixieHv5622::writeToHv5622(time_, HV5622_DATE);
}

// =============================================================================================================
// CLASS HV5622 - Private methods
// =============================================================================================================

esp_err_t NixieHv5622::writeToHv5622(struct tm time_, hv5622_conversion_t conversionType_)
{
	// convert time to 64 bits sequence, depending on type
	uint64_t sequence = NixieHv5622::convertTo64BitSequence(time_,conversionType_);
	
	// polarity and blank are set high
	m_gpio_POL.setOutput(1);
	m_gpio_BL.setOutput(1);
	// chip-select, which is our latch, is set low
	NixieHv5622::select();
	//NixieHv5622::release();
	// just to be sure, wait a very short time
	vTaskDelay(1);
	// write the bit sequence sequence to spi bus
	esp_err_t ok = NixieHv5622::write_bytes((uint8_t *)&sequence, 64);
	// wait again
	vTaskDelay(1);

	// activate by creating a "pulse" of our latch, which is the chipselect in our case
	// --> make sure it's just a positive edge and not a constant high
	NixieHv5622::release();
	vTaskDelay(3);
	NixieHv5622::select();

	// ESP_LOGE(TAG, "total sequence: %llu", sequence);
	return ESP_OK;
}

uint64_t NixieHv5622::convertTo64BitSequence(struct tm time_, hv5622_conversion_t conversionType_)
{
	uint8_t nrOfDigits = 6;						// we have 6 digits; hh:mm:ss, or DD:MM:YY
	uint8_t digitArray[nrOfDigits] = {};		// array to store the single digits (as numbers)
	uint16_t bitArray[nrOfDigits] = {};			// array to store correctly manipulated single digits as bits

	// depending on type, get single digits as numbers
	if(conversionType_ == HV5622_TIME){
		//[h][h][min][min][s][s] = [5][4][3][2][1][0]
		NixieHv5622::getTimeDigits(digitArray,time_);
	}else if(conversionType_ == HV5622_DATE){
		// [d][d][m][m][y][y] = [5][4][3][2][1][0]
		NixieHv5622::getDateDigits(digitArray,time_);
	}else{
		// no correct conversion type was selected...
		return 42;
	}
	//ESP_LOGE(TAG, "xx:yy:zz: %i%i:%i%i:%i%i",digitArray[5],digitArray[4],digitArray[3],digitArray[2],digitArray[1],digitArray[0]);
	
	// next, we get the bit matching the digit-number: 1 = 0000 0010 0000 0000, 2 = 0000 0001 0000 0000 and so on
	NixieHv5622::getTimeDigitsAsBits(bitArray,digitArray,nrOfDigits);

	// next we build up the bits-sequence, mapping every 10 bits of each digit together to a 64bits representation
	uint64_t sequence = NixieHv5622::get64BitSequence(bitArray, nrOfDigits, 10, 0);

	// now we must shift 4 bits because these are "lost" in our specific application
	uint64_t sequenceShifted = sequence << 4; 

	// next, I think because our chip is little endian, we need to swap every bit of each single byte
	uint64_t sequenceCorrectedEndiannes = NixieHv5622::reverseBitsOf8Bytes(sequenceShifted);

	//ESP_LOGE(TAG, "6 digit Sequence xx:yy:zz: %llu, shifted by 4bits: %llu, corrected: %llu", sequence, sequenceShifted, sequenceCorrectedEndiannes);

	return sequenceCorrectedEndiannes;
}

void NixieHv5622::getTimeDigits(uint8_t * digitArray_, struct tm time_)
{
	digitArray_[0] = NixieHv5622::getDigit(time_.tm_sec, 0);
	digitArray_[1] = NixieHv5622::getDigit(time_.tm_sec, 1);
	digitArray_[2] = NixieHv5622::getDigit(time_.tm_min, 0);
	digitArray_[3] = NixieHv5622::getDigit(time_.tm_min, 1);
	digitArray_[4] = NixieHv5622::getDigit(time_.tm_hour, 0);
	digitArray_[5] = NixieHv5622::getDigit(time_.tm_hour, 1);
}

void NixieHv5622::getDateDigits(uint8_t * digitArray_, struct tm time_)
{
	digitArray_[0] = NixieHv5622::getDigit(time_.tm_year, 0);
	digitArray_[1] = NixieHv5622::getDigit(time_.tm_year, 1);
	digitArray_[2] = NixieHv5622::getDigit(time_.tm_mon, 0);
	digitArray_[3] = NixieHv5622::getDigit(time_.tm_mon, 1);
	digitArray_[4] = NixieHv5622::getDigit(time_.tm_mday, 0);
	digitArray_[5] = NixieHv5622::getDigit(time_.tm_mday, 1);
}

uint8_t NixieHv5622::getDigit(uint32_t number_, uint8_t n_)
{
	return (number_ / (uint32_t)powf(10, n_)) % 10;
}

uint16_t NixieHv5622::getDigitBits(uint8_t digit_)
{
	// digit zero is the first bit
	if (digit_ < 1)
	{
		return 1;
	}
	// 1 = 
	return 1 << (10 - digit_);
}

void NixieHv5622::getTimeDigitsAsBits(uint16_t* bitArray_, uint8_t * digitArray_, uint8_t nrOfDigits_)
{
	for (int i = 0; i < nrOfDigits_; i++)
	{
		bitArray_[i] = NixieHv5622::getDigitBits(digitArray_[i]);
	}

}

uint64_t NixieHv5622::get64BitSequence(uint16_t *bitArray_, uint8_t nrOfDigits_, uint8_t bitsPerDigit_, uint8_t inverted_)
{
	uint64_t bits = 0;
	uint64_t Tempbits = 0;

	// build up all single digits (as bits) to one 64 bit string --> [5][4][3][2][1][0], so index 5 must be shifted to the far left and so on
	for (uint8_t i = 0; i < nrOfDigits_; i++)
	{
		Tempbits = bitArray_[nrOfDigits_-1-i];				// we extract an index and isolate it...
		bits = bits | (Tempbits << (bitsPerDigit_ * i));	// ...then we place it into the 64bit variable, at its correct place (shifted by n*10bits)
	}

	//ESP_LOGE(TAG, "The Array of Bits in Order ([5][4][3][2][1][0]): %i,%i,%i,%i,%i,%i", digitBitsArray_[5],digitBitsArray_[4],digitBitsArray_[3],digitBitsArray_[2],digitBitsArray_[1],digitBitsArray_[0]);
	//ESP_LOGE(TAG, "6 digit Sequence: 0x%llx", bits);

	return bits;
}

uint8_t NixieHv5622::reverse8Bits(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint64_t NixieHv5622::reverse64Bits(uint64_t b)
{
	uint64_t reversed = 0;
	uint64_t one64 = 1;
	
	for (int i = 0; i < 64; i++)
	{		
		reversed = reversed | (((b & (one64<<i))>>i)<<(63-i));
	}

	return b;
}

uint16_t NixieHv5622::reverseBitsOf2Bytes(uint16_t ui16TwoBytes_)
{
	// extract first byte (LSB)
	uint8_t firstByte = (uint8_t)(ui16TwoBytes_ & 0x00FF);
	// extract second byte (MSB)
	uint8_t secondByte = (uint8_t)((ui16TwoBytes_ & 0xFF00) >> 8);
	// reverse both bit sequences
	uint16_t firstByteReversed = (uint16_t)NixieHv5622::reverse8Bits(firstByte);
	uint16_t secondByteReversed = (uint16_t)NixieHv5622::reverse8Bits(secondByte);
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
		pPeak[i] = NixieHv5622::reverse8Bits(pPeak[i]);
	}
	// cast our peak pointer to a 64bit pointer and return value at said memory address
	return *((uint64_t *)pPeak);
}


