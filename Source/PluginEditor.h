#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "mod.h"

class CatDeckAudioProcessorEditor : public AudioProcessorEditor, public FileDragAndDropTarget, private Label::Listener, private Timer {
public:
  CatDeckAudioProcessorEditor( CatDeckAudioProcessor& );
  ~CatDeckAudioProcessorEditor();

  // ------
  
  void togglePlay();
  void pressCue( int index, bool shift, bool ctrl );
  void dragNudge( Point<int> event, bool mouseUp );
  void seekWaveformMacro( Point<int> point, bool mouseDown );
  void changeBpm( double bpm );
  void dragBpm( Point<int> point, bool mouseDown, bool mouseUp );
  void scratchWaveformMicro( Point<int> point, bool mouseDown, bool mouseUp );
  void skip( double beats );
  
  // ------
  
  void mouseDown( const MouseEvent& event ) override;
  void mouseMove( const MouseEvent& event ) override;
  void mouseDrag( const MouseEvent& event ) override;
  void mouseUp( const MouseEvent& event ) override;
  void mouseWheelMove( const MouseEvent& event, const MouseWheelDetails &wheel ) override;
  void labelTextChanged( Label* label ) override;
  
  // ------
  
  bool isInterestedInFileDrag( const StringArray& files ) override;
  void filesDropped( const StringArray& files, int x, int y ) override;
  
  // ------

  void paint( Graphics& g ) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  CatDeckAudioProcessor& processor;
  void timerCallback() override;
  
  // ------
  
  double computeWaveformMicroRange();
  
  // ------
  
  Label labelPlay;
  Label labelCue[ CUE_N ];
  Label labelPrev;
  Label labelForw;
  Label labelNudge;
  Label labelBpm;
  Component compoWaveformMacro;
  Component compoWaveformMicro;
  
  // ------
  
  Time timePrev;

  bool isNudging;
  bool dragBpmActive;
  double dragBpmStartBpm;
  int dragBpmStartY;
  double scratchPrev;
  double nudgeRate;
  String asdf;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( CatDeckAudioProcessorEditor )
};
