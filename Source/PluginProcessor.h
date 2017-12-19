#pragma once

#include "AudioPlayer.h"
#include "../JuceLibraryCode/JuceHeader.h"

class CatDeckAudioProcessor : public AudioProcessor {
public:
  CatDeckAudioProcessor();
  ~CatDeckAudioProcessor();
  
  // ------
  
  AudioPlayHead::CurrentPositionInfo currentPosition;
  AudioPlayer audioPlayer;

  // ------

  const String getName() const override;
  double getTailLengthSeconds() const override;
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram( int index ) override;
  const String getProgramName( int index ) override;
  void changeProgramName( int index, const String& newName ) override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;

  // ------

  void prepareToPlay( double sampleRate, int samplesPerBlock ) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool setPreferredBusArrangement( bool isInput, int bus, const AudioChannelSet& preferredSet ) override;
#endif

  void processBlock( AudioSampleBuffer&, MidiBuffer& ) override;
  
  bool isCurrentPositionValid();

  // ------

  bool hasEditor() const override;
  AudioProcessorEditor* createEditor() override;

  // ------

  void getStateInformation( MemoryBlock& destData ) override;
  void setStateInformation( const void* data, int sizeInBytes ) override;

  // ------

  Waveform waveformMacro;
  Waveform waveformMicroL;
  Waveform waveformMicroR;
  
  // ------

private:
  bool currentPositionValid;
  
  // ------
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( CatDeckAudioProcessor )
};
