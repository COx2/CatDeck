#pragma once

#define PI 3.14159265
#define CUE_N 5

#include "../JuceLibraryCode/JuceHeader.h"
#include "Spring.h"
#include "Waveform.h"

class AudioPlayer {
public:
  AudioPlayer();
  ~AudioPlayer();
  
  // ------
  
  void processBlock( AudioSampleBuffer& buffer, AudioProcessor& processor );

  void reset();
  bool loadAudio( String path );

  // ------

  double getProp( String key );
  void setProp( String key, double value );
  void initProps();
  bool loadProps();
  
  // ------
  
  // getters
  bool isLoaded();
  bool isPlaying();
  bool isScratching();
  double getBpm();
  double getPosition();
  double getCuePosition( int index );
  double getPlaybackRate();
  int getBufferChannels();
  int getBufferSamples();
  double getBufferSampleRate();
  const float* getBufferPointer( int ch );
  
  // ------
  
  // setters, controllers
  void play();
  void stop();
  void seek( double pos, double sync );
  void seek( double pos );
  void nudge( double rate );
  void nudgeOnce( double rate );
  void scratch( double gap, bool release );
  void setPlaybackRate( double rate );
  void setBpm( double value );

  void cueSet( int num );
  void cueJump( int num, double sync );
  
  // ------
  
private:
  // parameters
  bool loaded;
  bool playing;
  bool scratching;

  double bpm;
  double bufferSampleRate;
  double position;
  double playbackRate;
  double nudgeRate;
  bool isNudgeOnce;
  
  Spring springScratch;

  // ------

  double cuePosition[ CUE_N ];
  
  // ------
  
  float getSample( const float* buffer, double index );
  
  // ------
  
  AudioSampleBuffer buffer;
  String bufferHash;

  AudioFormatManager formatManager;
  ScopedPointer<AudioFormatReaderSource> readerSource;

  ApplicationProperties appProps;
  PropertiesFile* appPropsFile;
  XmlElement* appPropsElement;
  XmlElement* appPropsElementBuffer;
};