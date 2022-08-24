/*
    ...


*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "maag_i2c_device.h"

// static TAG
static const char *TAG = "maag_i2c_device";

// =============================================================================================================
// CLASS MaagI2CDevice
// =============================================================================================================
MaagI2CDevice::MaagI2CDevice(/* args */)
{
    ESP_LOGW(TAG, "MaagI2C device instance created");
}

void MaagI2CDevice::setPort(i2c_port_t port_)
{
    m_port = port_;
}

void MaagI2CDevice::setDeviceAddress(uint8_t dev_addr_)
{
    m_dev_addr = dev_addr_;
}

esp_err_t MaagI2CDevice::read(uint8_t *data_reg_, size_t data_reg_size_, uint8_t *in_data_, size_t in_size_)
{
    // check input and output data is valid
    if(data_reg_ == NULL || in_data_ == NULL || data_reg_size_ < 1 || in_size_ < 1){
        ESP_LOGE(TAG, "Invalid buffer pointers or invalid size");
        return ESP_ERR_INVALID_ARG;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // write slave address, writing bit and which register we want to read from slave afterwards
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (m_dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data_reg_, data_reg_size_, true);
    // write slave address, reading bit and get data that we requested in last command
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (m_dev_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, in_data_, in_size_, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    // start sequence
    esp_err_t res = i2c_master_cmd_begin(m_port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", m_dev_addr, m_port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t MaagI2CDevice::write(uint8_t *data_reg_, size_t data_reg_size_, uint8_t *out_data_, size_t out_data_size_)
{
    // check input and output data is valid
    if(data_reg_ == NULL || out_data_ == NULL || data_reg_size_ < 1 || out_data_size_ < 1){
        ESP_LOGE(TAG, "Invalid buffer pointers or invalid size");
        return ESP_ERR_INVALID_ARG;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // write slave address, writing bit and which register we want to write to slave afterwards
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (m_dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data_reg_, data_reg_size_, true);
    // write data to said slave address
    i2c_master_write(cmd, out_data_, out_data_size_, true);
    i2c_master_stop(cmd);
    // start sequence
    esp_err_t res = i2c_master_cmd_begin(m_port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d", m_dev_addr, m_port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}
