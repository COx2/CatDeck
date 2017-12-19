#include "PluginProcessor.h"
#include "PluginEditor.h"

// ------

CatDeckAudioProcessorEditor::CatDeckAudioProcessorEditor( CatDeckAudioProcessor& p )
: AudioProcessorEditor( &p ), processor( p ) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize( 400, 250 );

  // ------
  
  isNudging = false;
  nudgeRate = 0.0;
  dragBpmActive = false;

  // ------
  
  addAndMakeVisible( &labelPlay );
  labelPlay.setJustificationType( Justification::centred );
  labelPlay.addMouseListener( this, false );

  addAndMakeVisible( &labelPrev );
  labelPrev.setJustificationType( Justification::centred );
  labelPrev.addMouseListener( this, false );

  addAndMakeVisible( &labelForw );
  labelForw.setJustificationType( Justification::centred );
  labelForw.addMouseListener( this, false );

  for ( int i = 0; i < CUE_N; i ++ ) {
    addAndMakeVisible( labelCue[ i ] );
    labelCue[ i ].setJustificationType( Justification::centred );
    labelCue[ i ].addMouseListener( this, false );
  }
  
  addAndMakeVisible( &labelNudge );
  labelNudge.setText( String( "Nudge" ), dontSendNotification );
  labelNudge.setJustificationType( Justification::centred );
  labelNudge.setColour( Label::backgroundColourId, Colour::fromRGB( 68, 68, 68 ) );
  labelNudge.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );
  labelNudge.addMouseListener( this, false );
  
  addAndMakeVisible( &labelBpm );
  labelBpm.setJustificationType( Justification::centred );
  labelBpm.setEditable( false, true );
  labelBpm.setColour( Label::backgroundColourId, Colour::fromRGB( 68, 68, 68 ) );
  labelBpm.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );
  labelBpm.setColour( Label::backgroundWhenEditingColourId, Colour::fromRGB( 68, 68, 68 ) );
  labelBpm.setColour( Label::textWhenEditingColourId, Colour::fromRGB( 255, 255, 255 ) );
  labelBpm.addMouseListener( this, false );
  labelBpm.addListener( this );
  
  addAndMakeVisible( &compoWaveformMacro );
  compoWaveformMacro.addMouseListener( this, false );
  
  addAndMakeVisible( &compoWaveformMicro );
  compoWaveformMicro.addMouseListener( this, false );

  // ------

  timePrev = Time::getCurrentTime();
  startTimerHz( 60 );
}

CatDeckAudioProcessorEditor::~CatDeckAudioProcessorEditor() {}

// ------

void CatDeckAudioProcessorEditor::togglePlay() {
  if ( processor.audioPlayer.isPlaying() ) {
    processor.audioPlayer.stop();
  } else {
    processor.audioPlayer.play();
  }
}

void CatDeckAudioProcessorEditor::pressCue( int index, bool shift, bool ctrl ) {
  if ( ctrl ) {
    processor.audioPlayer.cueSet( index );
  } else {
    processor.audioPlayer.cueJump( index, shift ? 4.0 : 0.0 );
  }
}

void CatDeckAudioProcessorEditor::dragNudge( Point<int> point, bool mouseUp ) {
  if ( mouseUp ) {
    nudgeRate = 0.0;
    labelNudge.setText( "Nudge", dontSendNotification );
  } else {
    double posx = ( point.getX() + 0.5 ) / labelNudge.getWidth();
    double rate = posx * 2.0 - 1.0;
    nudgeRate = rate;
    labelNudge.setText( String( nudgeRate ), dontSendNotification );
  }
  processor.audioPlayer.nudge( nudgeRate );
}

void CatDeckAudioProcessorEditor::seekWaveformMacro( Point<int> point, bool mouseDown ) {
  double prog = ( point.getX() + 0.5 ) / compoWaveformMacro.getWidth();
  double pos = prog * processor.audioPlayer.getBufferSamples();

  if ( processor.audioPlayer.isPlaying() ) {
    if ( !mouseDown ) { return; }
    processor.audioPlayer.seek( pos, 4 );
  } else {
    processor.audioPlayer.seek( pos );
  }
}

void CatDeckAudioProcessorEditor::changeBpm( double bpm ) {
  double b = bpm;
  labelBpm.setText( String( bpm ), dontSendNotification );
  if ( isnan( b ) || b == 0.0 ) {
    labelBpm.setColour( Label::textColourId, Colour::fromRGB( 255, 0, 102 ) );
  } else {
    processor.audioPlayer.setBpm( b );
    labelBpm.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );
  }
}

void CatDeckAudioProcessorEditor::dragBpm( Point<int> point, bool mouseDown, bool mouseUp ) {
  int y = point.getY();
  if ( mouseDown ) {
    dragBpmStartBpm = floor( processor.audioPlayer.getBpm() );
    dragBpmStartY = y;
    dragBpmActive = true;
  } else if ( mouseUp ) {
    dragBpmActive = false;
  } else if ( dragBpmActive ) {
    changeBpm( floor( dragBpmStartBpm - ( y - dragBpmStartY ) * 0.2 ) );
  }
}

void CatDeckAudioProcessorEditor::scratchWaveformMicro( Point<int> point, bool mouseDown, bool mouseUp ) {
  double delta = ( point.getX() - scratchPrev ) / compoWaveformMicro.getWidth();
  scratchPrev = point.getX();
  if ( mouseDown ) { delta = 0.0; }
  
  double gap = -delta * computeWaveformMicroRange();
  processor.audioPlayer.scratch( gap, mouseUp );
}

void CatDeckAudioProcessorEditor::skip( double beats ) {
  processor.audioPlayer.seek( processor.audioPlayer.getPosition() + beats * 60.0 / processor.audioPlayer.getBpm() * processor.audioPlayer.getBufferSampleRate() );
}

// ------

void CatDeckAudioProcessorEditor::mouseDown( const MouseEvent& event ) {
  if ( event.eventComponent == &labelPlay ) { togglePlay(); }
  for ( int i = 0; i < CUE_N; i ++ ) {
    if ( event.eventComponent == &labelCue[ i ] ) { pressCue( i, event.mods.isShiftDown(), event.mods.isCtrlDown() ); }
  }
  if ( event.eventComponent == &labelPrev ) { skip( -4 ); }
  if ( event.eventComponent == &labelForw ) { skip( 4 ); }
  if ( event.eventComponent == &labelNudge ) { dragNudge( event.getPosition(), false ); }
  if ( event.eventComponent == &labelBpm ) { dragBpm( event.getPosition(), true, false ); }
  if ( event.eventComponent == &compoWaveformMacro ) { seekWaveformMacro( event.getPosition(), true ); }
  if ( event.eventComponent == &compoWaveformMicro ) { scratchWaveformMicro( event.getPosition(), true, false ); }
}

void CatDeckAudioProcessorEditor::mouseMove( const MouseEvent& event ) {
}

void CatDeckAudioProcessorEditor::mouseDrag( const MouseEvent& event ) {
  if ( event.eventComponent == &labelNudge ) { dragNudge( event.getPosition(), false ); }
  if ( event.eventComponent == &labelBpm ) { dragBpm( event.getPosition(), false, false ); }
  if ( event.eventComponent == &compoWaveformMacro ) { seekWaveformMacro( event.getPosition(), false ); }
  if ( event.eventComponent == &compoWaveformMicro ) { scratchWaveformMicro( event.getPosition(), false, false ); }
}

void CatDeckAudioProcessorEditor::mouseUp( const MouseEvent& event ) {
  if ( event.eventComponent == &labelNudge ) { dragNudge( event.getPosition(), true ); }
  if ( event.eventComponent == &labelBpm ) { dragBpm( event.getPosition(), false, true ); }
  if ( event.eventComponent == &compoWaveformMicro ) { scratchWaveformMicro( event.getPosition(), false, true ); }
}

void CatDeckAudioProcessorEditor::mouseWheelMove( const MouseEvent& event, const MouseWheelDetails &wheel ) {
  if ( event.eventComponent == &compoWaveformMicro ) { processor.audioPlayer.nudgeOnce( ( 0.0 < wheel.deltaY ) ? 1.0 : -1.0 ); }
}

void CatDeckAudioProcessorEditor::labelTextChanged( Label* label ) {
  if ( label == &labelBpm ) { changeBpm( labelBpm.getText().getDoubleValue() ); }
}

// ------

bool CatDeckAudioProcessorEditor::isInterestedInFileDrag( const StringArray& files ) {
  String path = files[ 0 ];
  File file( path );
  return (
    file.hasFileExtension( "mp3" )
    || file.hasFileExtension( "wav" )
    || file.hasFileExtension( "ogg" )
  );
}

void CatDeckAudioProcessorEditor::filesDropped( const StringArray& files, int x, int y ) {
  bool result = processor.audioPlayer.loadAudio( files[ 0 ] );
  if ( result ) {
    asdf = files[ 0 ];

    const float* buffer = processor.audioPlayer.getBufferPointer( 0 );
    int samples = processor.audioPlayer.getBufferSamples();
    int sampleRate = processor.audioPlayer.getBufferSampleRate();

    int macroSpp = samples / compoWaveformMacro.getWidth();
    int microSpp = 2.0 * sampleRate / compoWaveformMicro.getWidth();
    processor.waveformMacro.loadBuffer( buffer, samples, macroSpp, microSpp );
    processor.waveformMicroL.loadBuffer( buffer, samples, microSpp );
    processor.waveformMicroR.loadBuffer( processor.audioPlayer.getBufferPointer( 1 ), samples, microSpp );
  }
}

// ------

double CatDeckAudioProcessorEditor::computeWaveformMicroRange() {
  return ( 60.0 / processor.audioPlayer.getBpm() * 4.0 ) * processor.audioPlayer.getBufferSampleRate();
}

// ------

void CatDeckAudioProcessorEditor::paint( Graphics& g ) {
  Time time = Time::getCurrentTime();
  double deltaTime = ( time.toMilliseconds() - timePrev.toMilliseconds() ) * 0.001;

  // ------

  g.fillAll( Colour::fromRGB( 34, 34, 34 ) );

  // ------

  labelPlay.setText( processor.audioPlayer.isPlaying() ? "Stop" : "Play", dontSendNotification );
  labelPlay.setColour( Label::backgroundColourId, processor.audioPlayer.isPlaying() ? Colour::fromRGB( 102, 102, 102 ) : Colour::fromRGB( 68, 68, 68 ) );
  labelPlay.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );

  for ( int i = 0; i < CUE_N; i ++ ) {
    labelCue[ i ].setText( String( i + 1 ), dontSendNotification );
    labelCue[ i ].setColour( Label::backgroundColourId, Colour::fromRGB( 68, 68, 68 ) );
    labelCue[ i ].setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );
  }

  labelNudge.setColour( Label::backgroundColourId, labelNudge.isMouseButtonDown() ? Colour::fromRGB( 102, 102, 102 ) : Colour::fromRGB( 68, 68, 68 ) );
  labelNudge.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );

  labelPrev.setText( "<<", dontSendNotification );
  labelPrev.setColour( Label::backgroundColourId, labelPrev.isMouseButtonDown() ? Colour::fromRGB( 102, 102, 102 ) : Colour::fromRGB( 68, 68, 68 ) );
  labelPrev.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );

  labelForw.setText( ">>", dontSendNotification );
  labelForw.setColour( Label::backgroundColourId, labelForw.isMouseButtonDown() ? Colour::fromRGB( 102, 102, 102 ) : Colour::fromRGB( 68, 68, 68 ) );
  labelForw.setColour( Label::textColourId, Colour::fromRGB( 255, 255, 255 ) );

  if ( !labelBpm.isBeingEdited() ) {
    labelBpm.setText( String( processor.audioPlayer.getBpm() ), dontSendNotification );
  }
  
  { // macroThumbnail
    Rectangle<int> rect = compoWaveformMacro.getBounds();
    g.setColour( Colour::fromRGB( 68, 68, 68 ) );
    g.fillRect( rect );
    g.setColour( Colour::fromRGB( 255, 255, 255 ) );
    processor.waveformMacro.draw( g, rect, 0 );

    g.setColour( Colour::fromRGB( 255, 0, 102 ) );
    double progress = processor.audioPlayer.getPosition() / processor.audioPlayer.getBufferSamples();
    int top = rect.getY();
    int bottom = rect.getBottom();
    int x = rect.getX() + rect.getWidth() * progress;
    g.drawLine( x, top, x, bottom );

    g.setColour( Colour::fromRGB( 0, 102, 255 ) );
    for ( int i = 0; i < CUE_N; i ++ ) {
      int x = rect.getX() + rect.getWidth() * processor.audioPlayer.getCuePosition( i ) / processor.audioPlayer.getBufferSamples();
      g.drawLine( x, top, x, bottom );
      g.drawText( String( i + 1 ), x, top, 100, bottom - top, Justification::bottomLeft );
    }
  }
  
  { // microThumbnail
    Rectangle<int> rect = compoWaveformMicro.getBounds();
    g.setColour( Colour::fromRGB( 68, 68, 68 ) );
    g.fillRect( rect );

    g.setColour( Colour::fromRGB( 255, 255, 255 ) );
    double pos = processor.audioPlayer.getPosition();
    int start = ( pos - processor.audioPlayer.getBufferSampleRate() * 1.0 ) / processor.waveformMicroL.getSpp();

    if ( dragBpmActive ) {
      for ( int i = 0; i < 4; i ++ ) {
        double y = rect.getHeight() * ( i == 0 ? 0.0 : ( 0.5 + ( i - 1.0 ) / 6.0 ) );
        double h = rect.getHeight() * ( i == 0 ? 0.5 : 1.0 / 6.0 );
        Rectangle<int> r( rect.getX(), rect.getY() + y, rect.getWidth(), h );
        int gap = 240.0 / processor.audioPlayer.getBpm() * processor.audioPlayer.getBufferSampleRate() / processor.waveformMicroL.getSpp();
        processor.waveformMicroL.draw( g, r, start + gap * i );
      }
    } else {
      Rectangle<int> r( rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight() * 0.5 );
      processor.waveformMicroL.draw( g, r, start );
      r.setY( r.getY() + r.getHeight() );
      processor.waveformMicroR.draw( g, r, start, true );
    }

    int top = rect.getY();
    int bottom = rect.getBottom();

    g.setColour( Colour::fromRGB( 255, 102, 0 ) );
    double beatTime = 60.0 / processor.audioPlayer.getBpm();
    int beats = floor( 1.0 / beatTime ) + 1;
    for ( int i = -beats; i <= beats; i ++ ) {
      double p = 0.5 * ( 1.0 + i * beatTime );
      if ( processor.isCurrentPositionValid() && processor.currentPosition.isPlaying ) {
        p -= ( mod( processor.currentPosition.ppqPosition + 0.5, 1.0 ) - 0.5 ) * 0.5 * beatTime;
      }
      if ( p < 0.0 || 1.0 < p ) { continue; }
      int x = rect.getX() + rect.getWidth() * p;
      g.drawLine( x, top, x, bottom );
    }

    g.setColour( Colour::fromRGB( 255, 0, 102 ) );
    int x = rect.getX() + rect.getWidth() * 0.5;
    g.drawLine( x, top, x, bottom );

    g.setColour( Colour::fromRGB( 0, 102, 255 ) );
    for ( int i = 0; i < CUE_N; i ++ ) {
      double prog = processor.audioPlayer.getCuePosition( i ) - ( pos - processor.audioPlayer.getBufferSampleRate() * 1.0 );
      prog /= processor.audioPlayer.getBufferSampleRate() * 2.0;
      if ( prog < 0.0 || 1.0 < prog ) { continue; }

      int x = rect.getX() + rect.getWidth() * prog;
      g.drawLine( x, top, x, bottom, 1.0 );
      g.drawText( String( i + 1 ), x, top, 100, bottom - top, Justification::bottomLeft );
    }
  }

  // ------

  g.setColour( Colour::fromRGB( 0, 255, 102 ) );
  g.drawText( String( asdf ), 100, 100, 200, 200, Justification::topLeft );

  // ------

  timePrev = time;
}

void CatDeckAudioProcessorEditor::resized() {
  labelPlay.setBounds( Rectangle<int>( 10, 170, 240, 40 ) );
  for ( int i = 0; i < CUE_N; i ++ ) {
    labelCue[ i ].setBounds( Rectangle<int>( 10 + 50 * i, 220, 40, 20 ) );
  }
  labelPrev.setBounds( Rectangle<int>( 260, 220, 60, 20 ) );
  labelForw.setBounds( Rectangle<int>( 330, 220, 60, 20 ) );
  labelBpm.setBounds( Rectangle<int>( 260, 170, 130, 40 ) );
  labelNudge.setBounds( Rectangle<int>( 10, 120, 380, 40 ) );
  compoWaveformMacro.setBounds( Rectangle<int>( 10, 10, 380, 30 ) );
  compoWaveformMicro.setBounds( Rectangle<int>( 10, 50, 380, 60 ) );
}

void CatDeckAudioProcessorEditor::timerCallback() {
  repaint();
}