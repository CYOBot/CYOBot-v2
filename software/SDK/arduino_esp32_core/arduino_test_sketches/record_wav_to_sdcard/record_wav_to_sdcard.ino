/*
 * Record SD Card Wave Example (Arduino Friendly)
 * 
 * This sketch records a WAV file to SD Card using CYOBot v2 hardware.
 * It uses the ES7210 Codec driver ported from ESP-ADF.
 * 
 * Pins:
 * I2S: MCLK=16, BCLK=9, WS=45, DIN=10
 * I2C: SDA=17, SCL=18
 * SD: MOSI=11, MISO=13, CLK=12, CS=14
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <driver/i2s.h>
#include "board_pins.h"
#include "es7210.h"

// Audio Configuration
#define SAMPLE_RATE 16000
#define I2S_NUM I2S_NUM_0
#define RECORD_TIME_SEC 10
#define CHANNELS 2
#define BITS_PER_SAMPLE 16

File recFile;

typedef struct {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
} wav_header_t;

void writeWavHeader(File &file, int dataSize) {
    wav_header_t header;
    memcpy(header.riff, "RIFF", 4);
    header.fileSize = dataSize + sizeof(wav_header_t) - 8;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmtSize = 16;
    header.audioFormat = 1; // PCM
    header.channels = CHANNELS;
    header.sampleRate = SAMPLE_RATE;
    header.bitsPerSample = BITS_PER_SAMPLE;
    header.byteRate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);
    header.blockAlign = CHANNELS * (BITS_PER_SAMPLE / 8);
    memcpy(header.data, "data", 4);
    header.dataSize = dataSize;
    
    file.seek(0);
    file.write((uint8_t*)&header, sizeof(wav_header_t));
}

void i2s_init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
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
        .data_out_num = -1,
        .data_in_num = I2S_DIN_PIN
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

    // 2. Initialize Codec (ADC)
    audio_hal_codec_config_t cfg;
    cfg.codec_mode = AUDIO_HAL_CODEC_MODE_ENCODE;
    cfg.i2s_iface.mode = AUDIO_HAL_MODE_SLAVE;
    cfg.i2s_iface.fmt = AUDIO_HAL_I2S_NORMAL;
    cfg.i2s_iface.samples = AUDIO_HAL_SAMPLE_RATE_16K;
    cfg.i2s_iface.bits = AUDIO_HAL_BIT_LENGTH_16BITS;
    
    if (es7210_adc_init(&cfg) != 0) {
        Serial.println("Codec Init Failed");
        while(1);
    }
    
    // Start Codec (Enable ADC)
    es7210_adc_ctrl_state(AUDIO_HAL_CODEC_MODE_ENCODE, AUDIO_HAL_CTRL_START);

    // Set Gain (Volume) to 30dB (Reasonable starting point)
    es7210_adc_set_volume(GAIN_30DB); 
    
    Serial.println("Codec Initialized");

    // 3. Initialize I2S
    i2s_init();

    // 4. Open File for Write
    if (SD.exists("/record.wav")) SD.remove("/record.wav");
    recFile = SD.open("/record.wav", FILE_WRITE);
    if(!recFile) {
        Serial.println("Could not create /record.wav");
        while(1);
    }
    
    // Write placeholder header
    writeWavHeader(recFile, 0);
    
    Serial.println("Recording for 10 seconds...");
    uint32_t startParams = millis();
    uint32_t totalBytes = 0;
    uint8_t buffer[1024];

    while (millis() - startParams < RECORD_TIME_SEC * 1000) {
        size_t bytesRead;
        // Read from I2S
        i2s_read(I2S_NUM, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
        if (bytesRead > 0) {
            recFile.write(buffer, bytesRead);
            totalBytes += bytesRead;
        }
    }
    
    // Finalize Header
    writeWavHeader(recFile, totalBytes);
    recFile.close();
    Serial.println("Recording Finished.");
}

void loop() {
}
