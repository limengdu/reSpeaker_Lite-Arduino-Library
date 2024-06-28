/**
 * @file streams-generator-i2s.ino
 * @author Phil Schatzmann
 * @modifier Seeed Studio
 * @brief Sending analogue acoustic signals via I2S.
 * @version 1.0
 * @date 2024-06-28
 * 
 * @copyright Copyright (c) 2024
 */

#include "AudioTools.h"

AudioInfo info(16000, 2, 32);                              // Sample Rate, Number of channels: 2=stereo, 1=mono, Number of bits per sample (int16_t = 16 bits)
SineWaveGenerator<int16_t> sineWave(32000);                // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);             // Stream generated from sine wave
I2SStream out; 
StreamCopy copier(out, sound);                             // copies sound into i2s

// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // start I2S
  Serial.println("starting I2S...");
  auto config = out.defaultConfig(TX_MODE);
  config.copyFrom(info); 
  out.begin(config);

  // Setup sine wave
  sineWave.begin(info, N_B4);
  Serial.println("started...");
}

// Arduino loop - copy sound to out 
void loop() {
  copier.copy();
}
