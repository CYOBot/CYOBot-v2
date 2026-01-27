#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AUDIO_HAL_CODEC_MODE_ENCODE = 1,
    AUDIO_HAL_CODEC_MODE_DECODE = 2,
    AUDIO_HAL_CODEC_MODE_BOTH = 3,
    AUDIO_HAL_CODEC_MODE_LINE_IN = 4
} audio_hal_codec_mode_t;

typedef enum {
    AUDIO_HAL_CTRL_START = 1,
    AUDIO_HAL_CTRL_STOP = 2
} audio_hal_ctrl_t;

typedef enum {
    AUDIO_HAL_I2S_NORMAL = 0,
    AUDIO_HAL_I2S_LEFT = 1,
    AUDIO_HAL_I2S_RIGHT = 2,
    AUDIO_HAL_I2S_DSP = 3,
} audio_hal_iface_format_t;

typedef enum {
    AUDIO_HAL_MODE_SLAVE = 0,
    AUDIO_HAL_MODE_MASTER = 1,
} audio_hal_iface_mode_t;

typedef enum {
    AUDIO_HAL_SAMPLE_RATE_8K = 8000,
    AUDIO_HAL_08K_SAMPLES = 8000,
    AUDIO_HAL_SAMPLE_RATE_11K = 11025,
    AUDIO_HAL_11K_SAMPLES = 11025,
    AUDIO_HAL_SAMPLE_RATE_16K = 16000,
    AUDIO_HAL_16K_SAMPLES = 16000,
    AUDIO_HAL_SAMPLE_RATE_22K = 22050,
    AUDIO_HAL_22K_SAMPLES = 22050,
    AUDIO_HAL_SAMPLE_RATE_24K = 24000,
    AUDIO_HAL_24K_SAMPLES = 24000,
    AUDIO_HAL_SAMPLE_RATE_32K = 32000,
    AUDIO_HAL_32K_SAMPLES = 32000,
    AUDIO_HAL_SAMPLE_RATE_44K = 44100,
    AUDIO_HAL_44K_SAMPLES = 44100,
    AUDIO_HAL_SAMPLE_RATE_48K = 48000,
    AUDIO_HAL_48K_SAMPLES = 48000,
} audio_hal_iface_samples_t;

typedef enum {
    AUDIO_HAL_BIT_LENGTH_16BITS = 16,
    AUDIO_HAL_BIT_LENGTH_24BITS = 24,
    AUDIO_HAL_BIT_LENGTH_32BITS = 32,
} audio_hal_iface_bits_t;

typedef struct {
    audio_hal_iface_mode_t mode;
    audio_hal_iface_format_t fmt;
    audio_hal_iface_samples_t samples;
    audio_hal_iface_bits_t bits;
} audio_hal_codec_i2s_iface_t;

typedef struct {
    audio_hal_codec_mode_t codec_mode;
    audio_hal_codec_i2s_iface_t i2s_iface;
} audio_hal_codec_config_t;

#ifdef __cplusplus
}
#endif
