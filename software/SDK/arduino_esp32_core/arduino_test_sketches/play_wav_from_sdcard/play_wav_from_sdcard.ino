/*
 * Play SD Card Wave Example (Arduino Friendly)
 * 
 * This sketch plays a WAV file from SD Card using CYOBot v2 hardware.
 * It uses the ES8311 Codec driver ported from ESP-ADF.
 * 
 * Pins:
 * I2S: MCLK=16, BCLK=9, WS=45, DOUT=8
 * I2C: SDA=17, SCL=18
 * SD: MOSI=11, MISO=13, CLK=12, CS=14
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <driver/i2s.h>
#include "board_pins.h"
#include "es8311.h"

// Audio Configuration
#define SAMPLE_RATE 16000
#define I2S_NUM I2S_NUM_0

File audioFile;

void i2s_init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = true,
        .tx_desc_auto_clear = true
    };
    
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_MCLK_PIN,
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = -1
    };
    
    i2s_set_pin(I2S_NUM, &pin_config);
}

void setup() {
    Serial.begin(115200);
    while(!Serial);

    Serial.println("Initializing connections...");

    // 1. Initialize SD Card
    SPI.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Initialisation failed!");
        while(1);
    }
    Serial.println("SD Initialized");

    // 2. Initialize Codec
    // Wire.begin() is called inside es8311_codec_init internal i2c_init via static helper
    // Setup Codec Config
    audio_hal_codec_config_t cfg;
    cfg.codec_mode = AUDIO_HAL_CODEC_MODE_DECODE;
    cfg.i2s_iface.mode = AUDIO_HAL_MODE_SLAVE; // ESP32 is Master
    cfg.i2s_iface.fmt = AUDIO_HAL_I2S_NORMAL;
    cfg.i2s_iface.samples = AUDIO_HAL_SAMPLE_RATE_16K; // Initial guess
    cfg.i2s_iface.bits = AUDIO_HAL_BIT_LENGTH_16BITS;
    
    if (es8311_codec_init(&cfg) != 0) {
        Serial.println("Codec Init Failed");
        while(1);
    }
    
    // Start Codec (Enable DAC)
    es8311_codec_ctrl_state(AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    
    // Set Volume and Enable PA
    es8311_codec_set_voice_volume(100);
    es8311_pa_power(true); 
    
    Serial.println("Codec Initialized");

    // 3. Initialize I2S
    i2s_init();

    // 4. Open File
    audioFile = SD.open("/record.wav");
    if(!audioFile) {
        Serial.println("Could not open /record.wav");
    } else {
        // Skip WAV Header (Simple 44 byte skip for PCM)
        audioFile.seek(44); 
    }
}

void loop() {
    if (audioFile && audioFile.available()) {
        uint8_t buffer[1024];
        int bytesRead = audioFile.read(buffer, sizeof(buffer));
        
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
    } else {
        if(audioFile) {
            Serial.println("Playback Finished");
            audioFile.close();
        }
    }
}
