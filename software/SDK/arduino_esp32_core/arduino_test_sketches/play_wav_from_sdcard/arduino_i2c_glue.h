#pragma once
#include <Arduino.h>
#include <Wire.h>

#ifndef ESP_OK
#define ESP_OK 0
#endif

#ifndef ESP_FAIL
#define ESP_FAIL -1
#endif


// Glue for ADF compatibility
#define AUDIO_NULL_CHECK(TAG, a, action) if (!(a)) { action; }
#define AUDIO_RET_ON_FALSE(TAG, a, action, msg) if (!(a)) { Serial.print(TAG); Serial.print(": "); Serial.println(msg); action; }

// Use TwoWire pointer as handle
typedef TwoWire* i2c_bus_handle_t;

static inline esp_err_t i2c_bus_delete(i2c_bus_handle_t handle) {
    return ESP_OK;
}

// Standard I2C Write
static inline esp_err_t i2c_bus_write_bytes(i2c_bus_handle_t handle, uint8_t addr, uint8_t *reg, int reg_len, uint8_t *data, int data_len) {
    // addr is usually 8-bit in ADF, Wire expects 7-bit
    handle->beginTransmission(addr >> 1);
    for(int i=0; i<reg_len; i++) handle->write(reg[i]);
    for(int i=0; i<data_len; i++) handle->write(data[i]);
    if(handle->endTransmission() == 0) return ESP_OK;
    return ESP_FAIL;
}

// Standard I2C Read
static inline esp_err_t i2c_bus_read_bytes(i2c_bus_handle_t handle, uint8_t addr, uint8_t *reg, int reg_len, uint8_t *data, int data_len) {
    if (reg_len > 0) {
        handle->beginTransmission(addr >> 1);
        for(int i=0; i<reg_len; i++) handle->write(reg[i]);
        if(handle->endTransmission(false) != 0) return ESP_FAIL;
    }
    
    handle->requestFrom((int)(addr >> 1), data_len);
    int read = 0;
    while(handle->available() && read < data_len) {
        data[read++] = handle->read();
    }
    return (read == data_len) ? ESP_OK : ESP_FAIL;
}

// Log Macro replacement
#define ESP_LOGE(tag, fmt, ...) do { Serial.print("E ["); Serial.print(tag); Serial.print("] "); Serial.printf(fmt, ##__VA_ARGS__); Serial.println(); } while(0)
#define ESP_LOGW(tag, fmt, ...) do { Serial.print("W ["); Serial.print(tag); Serial.print("] "); Serial.printf(fmt, ##__VA_ARGS__); Serial.println(); } while(0)
#define ESP_LOGI(tag, fmt, ...) do { Serial.print("I ["); Serial.print(tag); Serial.print("] "); Serial.printf(fmt, ##__VA_ARGS__); Serial.println(); } while(0)
#define ESP_LOGD(tag, fmt, ...) // do { Serial.print("D ["); Serial.print(tag); Serial.print("] "); Serial.printf(fmt, ##__VA_ARGS__); Serial.println(); } while(0)
