#pragma once
#include "AudioBoard.h" // install audio-driver library
#include "AudioConfig.h"
#include "AudioI2S/I2SStream.h"

namespace audio_tools {

struct I2SCodecConfig : public I2SConfig {
  adc_input_t adc_input = ADC_INPUT_LINE1;
  dac_output_t dac_output = DAC_OUTPUT_ALL;
};

/**
 * @brief  I2S Stream which also sets up a codec chip
 * @ingroup io
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class I2SCodecStream : public AudioStream {
public:
  I2SCodecStream(AudioBoard *board) { p_board = board; }

  /// Provides the default configuration
  I2SCodecConfig defaultConfig(RxTxMode mode = TX_MODE) {
    auto cfg1 = i2s.defaultConfig(mode);
    I2SCodecConfig cfg;
    memcpy(&cfg, &cfg1, sizeof(cfg1));
    cfg.adc_input = ADC_INPUT_LINE1;
    cfg.dac_output = DAC_OUTPUT_ALL;
    return cfg;
  }

  bool begin() {
    if (!beginCodec(cfg)) {
      TRACEE()
    }
    return i2s.begin(cfg);
  }

  /// Starts the I2S interface
  bool begin(I2SCodecConfig cfg) {
    TRACED();
    this->cfg = cfg;
    return begin();
  }

  /// Stops the I2S interface
  void end() {
    TRACED();
    p_board->end();
    i2s.end();
  }

  /// updates the sample rate dynamically
  virtual void setAudioInfo(AudioInfo info) {
    TRACEI();
    AudioStream::setAudioInfo(info);
    beginCodec(info);
    i2s.setAudioInfo(info);
  }

  /// Writes the audio data to I2S
  virtual size_t write(const uint8_t *buffer, size_t size) {
    LOGD("I2SStream::write: %d", size);
    return i2s.write(buffer, size);
  }

  /// Reads the audio data
  virtual size_t readBytes(uint8_t *data, size_t length) override {
    return i2s.readBytes(data, length);
  }

  /// Provides the available audio data
  virtual int available() override { return i2s.available(); }

  /// Provides the available audio data
  virtual int availableForWrite() override { return i2s.availableForWrite(); }

  /// sets the volume (range 0.0 - 1.0)
  bool setVolume(float vol) { return p_board->setVolume(vol * 100.0); }
  int getVolume() { return p_board->getVolume(); }
  bool setMute(bool mute) { return p_board->setMute(mute); }
  bool setPAPower(bool active) { return p_board->setPAPower(active); }

  AudioBoard &board() { return *p_board; }

protected:
  I2SStream i2s;
  I2SCodecConfig cfg;
  AudioBoard *p_board = nullptr;

  bool beginCodec(AudioInfo info) {
    cfg.sample_rate = info.sample_rate;
    cfg.bits_per_sample = info.bits_per_sample;
    cfg.channels = info.channels;
    return beginCodec(cfg);
  }

  bool beginCodec(I2SCodecConfig info) {
    CodecConfig cfg;
    cfg.adc_input = info.adc_input;
    cfg.dac_output = info.dac_output;
    cfg.i2s.bits = toCodecBits(info.bits_per_sample);
    cfg.i2s.rate = toRate(info.sample_rate);
    cfg.i2s.fmt = toFormat(info.i2s_format);
    // use reverse logic for codec setting
    cfg.i2s.mode = info.is_master ? MODE_SLAVE : MODE_MASTER;
    return p_board->begin(cfg);
  }

  sample_bits_t toCodecBits(int bits) {
    switch (bits) {
    case 16:
      return BIT_LENGTH_16BITS;
    case 24:
      return BIT_LENGTH_24BITS;
    case 32:
      return BIT_LENGTH_32BITS;
    }
    LOGE("Unsupported bits: %d", bits);
    return BIT_LENGTH_16BITS;
  }
  samplerate_t toRate(int rate) {
    if (rate <= 8000)
      return RATE_08K;
    if (rate <= 11000)
      return RATE_11K;
    if (rate <= 16000)
      return RATE_16K;
    if (rate <= 22000)
      return RATE_22K;
    if (rate <= 32000)
      return RATE_32K;
    if (rate <= 44000)
      return RATE_44K;
    return RATE_48K;
  }

  i2s_format_t toFormat(I2SFormat fmt) {
    switch (fmt) {
    case I2S_PHILIPS_FORMAT:
    case I2S_STD_FORMAT:
      return I2S_NORMAL;
    case I2S_LEFT_JUSTIFIED_FORMAT:
    case I2S_MSB_FORMAT:
      return I2S_LEFT;
    case I2S_RIGHT_JUSTIFIED_FORMAT:
    case I2S_LSB_FORMAT:
      return I2S_RIGHT;
    case I2S_PCM:
      return I2S_DSP;
    default:
      LOGE("unsupported mode");
      return I2S_NORMAL;
    }
  }
};

} // namespace audio_tools