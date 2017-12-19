#include "Waveform.h"

Waveform::Waveform() {
  int computedIndex = 0;
  originalBuffer = nullptr;
}

Waveform::~Waveform() {
}

// ------

int Waveform::getSpp() {
  return spp;
}

bool Waveform::isLoaded() {
  return originalBuffer != nullptr;
}

int Waveform::getComputedIndex() {
  return computedIndex;
}

bool Waveform::isComputed() {
  return isLoaded() && ( computedIndex == length );
}

float* Waveform::getRawBuffer() {
  return buffer.getRawDataPointer();
}

// ------

void Waveform::loadBuffer( const float* _buffer, int _length, int _spp, int _ci ) {
  originalBuffer = _buffer;
  originalLength = _length;

  spp = _spp;
  ci = _ci == 0 ? _spp : _ci;
  length = originalLength / spp + 1;

  buffer.clear();
  buffer.resize( length );

  resetBuffer();
}

// ------

void Waveform::resetBuffer() {
  computedIndex = 0;
}

void Waveform::computeBuffer() {
  if ( !isLoaded() ) { return; }
  int64 begin = Time::currentTimeMillis();

  while ( !isComputed() ) {
    int64 now = Time::currentTimeMillis();
    float* buf = getRawBuffer();
    if ( COMPUTE_TIMEOUT < now - begin ) { break; }

    float max = 0.0f;
    double sum = 0.0;
    int count = spp / ci;

    for ( int is = 0; is < spp / ci; is ++ ) {
      for ( int ic = 0; ic < ci; ic ++ ) {
        int index = computedIndex * spp + ( is * ci + ic );
        if ( originalLength <= index ) { break; }

        max = jmax( max, abs( originalBuffer[ index ] ) );
      }
      sum += max;
    }

    buf[ computedIndex ] = sum / count;
    computedIndex ++;
  }
}

// ------

void Waveform::draw( Graphics& g, const Rectangle<int>& rect, int start, bool flip ) {
  if ( !isLoaded() ) { return; }
  if ( !isComputed() ) { computeBuffer(); }

  int index = start - 1;
  for ( int i = 0; i < rect.getWidth(); i ++ ) {
    index ++;
    if ( index < 0 ) { continue; }
    if ( computedIndex <= index ) { break; }

    int x = rect.getX() + i;
    float v = buffer[ index ] * 0.6;
    int yb = rect.getY() + ( flip ? 0 : rect.getHeight() );
    int yt = rect.getY() + rect.getHeight() * ( flip ? v : ( 1.0 - v ) );
    g.drawLine( x, yb, x, yt );
  }
}

void Waveform::draw( Graphics& g, const Rectangle<int>& rect, int start ) {
  draw( g, rect, start, false );
}
