#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLES 4096

// Audio callback function
void AudioCallback(void* userdata, U8* stream, int len) {
  static double phase = 0;
  I16* buffer = (I16*)stream;
  int length = len / 2;
  double frequency = *((double*)userdata);
    
  for(int i = 0; i < length; i++) {
    buffer[i] = 32767 * sin(phase);
    phase += 2 * M_PI * frequency / AUDIO_FREQUENCY;
    if(phase > 2 * M_PI) {
      phase -= 2 * M_PI;
    }
  }
}

// Function to play beep sound
void PlayBeep(double frequency) {
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;

  SDL_zero(want);
  want.freq = AUDIO_FREQUENCY;
  want.format = AUDIO_FORMAT;
  want.channels = AUDIO_CHANNELS;
  want.samples = AUDIO_SAMPLES;
  want.callback = AudioCallback;
  want.userdata = &frequency;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) {
    fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    return;
  }

  SDL_PauseAudioDevice(dev, 0);
  SDL_Delay(100);
  SDL_CloseAudioDevice(dev);
}
