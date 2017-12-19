#pragma once

#define COMPUTE_TIMEOUT 1 // millis
#include "../JuceLibraryCode/JuceHeader.h"

class Waveform {
public:
  Waveform();
  ~Waveform();

  int getSpp();
  bool isLoaded();
  int getComputedIndex();
  bool isComputed();
  float* getRawBuffer();

  void loadBuffer( const float* buffer, int length, int spp, int ci = 0 );

  void draw( Graphics& g, const Rectangle<int>& rect, int start, bool flip );
  void draw( Graphics& g, const Rectangle<int>& rect, int start );

private:
  const float* originalBuffer;
  int originalLength;
  int spp;
  int ci;
  int length;

  Array<float> buffer;
  int computedIndex;

  void resetBuffer();
  void computeBuffer();
};