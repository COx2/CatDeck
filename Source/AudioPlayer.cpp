#include "AudioPlayer.h"

AudioPlayer::AudioPlayer() {
  reset();
  
  formatManager.registerBasicFormats();
  setBpm( 140.0 );

  PropertiesFile::Options appPropsOptions;
  appPropsOptions.applicationName = "CatDeck";
  appPropsOptions.filenameSuffix = ".props";
  appPropsOptions.folderName = "JUCE";
  appPropsOptions.osxLibrarySubFolder = "Application Support";
  appPropsOptions.millisecondsBeforeSaving = 2000;
  appProps.setStorageParameters( appPropsOptions );

  appPropsFile = appProps.getUserSettings();

  if ( appPropsFile->containsKey( "props" ) ) {
    appPropsElement = appPropsFile->getXmlValue( "props" );
  } else {
    appPropsElement = new XmlElement( "props" );
    appPropsFile->setValue( "props", appPropsElement );
  }

}

AudioPlayer::~AudioPlayer() {
}

// ------

void AudioPlayer::play() {
  playing = true;
}

void AudioPlayer::stop() {
  playing = false;
}

void AudioPlayer::seek( double pos, double sync ) {
  if ( 0 < sync ) {
    double intrv = 60.0 / getBpm() * sync * getBufferSampleRate();
    pos = getPosition() + round( ( pos - getPosition() ) / intrv ) * intrv;
  }

  position = pos;
}

void AudioPlayer::seek( double pos ) {
  seek( pos, 0.0 );
}

void AudioPlayer::scratch( double gap, bool release ) {
  if ( isScratching() ) {
    if ( release ) {
      scratching = false;
    } else {
      double pos = springScratch.getTarget() + gap;
      springScratch.setTarget( pos );
    }
  } else if ( !release ) {
    scratching = true;
    
    double pos = position + gap;
    springScratch.setPosition( pos );
    springScratch.setVelocity( 0.0 );
    springScratch.setTarget( pos );
  }
}

void AudioPlayer::nudge( double rate ) {
  isNudgeOnce = false;
  nudgeRate = rate;
}

void AudioPlayer::nudgeOnce( double rate ) {
  isNudgeOnce = true;
  nudgeRate = rate;
}

void AudioPlayer::setPlaybackRate( double rate ) {
  playbackRate = rate;
}

void AudioPlayer::setBpm( double value ) {
  bpm = value;
  setProp( "bpm", value );
}

// ------

void AudioPlayer::cueSet( int num ) {
  if ( num < 0 || CUE_N < num ) { return; }
  cuePosition[ num ] = position;
  setProp( "cue" + String( num + 1 ), long( position ) );
}

void AudioPlayer::cueJump( int num, double sync = 0.0 ) {
  if ( num < 0 || CUE_N < num ) { return; }
  seek( cuePosition[ num ], sync );
}

// ------

void AudioPlayer::processBlock( AudioSampleBuffer& outBuffer, AudioProcessor& processor ) {
  int length = outBuffer.getNumSamples();
  int channels = processor.getTotalNumOutputChannels();
  double deltaTime = length / processor.getSampleRate();
  
  float rate = playbackRate * ( isPlaying() ? 1.0 : nudgeRate );
  rate *= getBufferSampleRate() / processor.getSampleRate();
  
  double nudgeGap = isPlaying() ? ( rate * nudgeRate * length ) : 0.0;
  position += nudgeGap;

  if ( isNudgeOnce ) { nudgeRate = 0.0; }
  
  if ( isScratching() ) {
    springScratch.update( deltaTime );
    rate = springScratch.getVelocity() / getBufferSampleRate();
  }
  
  // ------
  
  if ( !isLoaded() || rate == 0.0 ) {
    for ( int iCh = 0; iCh < channels; iCh ++ ) {
      float* out = outBuffer.getWritePointer( iCh );
      for ( int i = 0; i < length; i ++ ) {
        out[ i ] = 0;
      }
    }
    return;
  }
  
  double outpos;
  
  for ( int iCh = 0; iCh < channels; iCh ++ ) {
    const float* in = getBufferPointer( iCh % getBufferChannels() );
    float* out = outBuffer.getWritePointer( iCh );
    double pos = getPosition();
    
    for ( int i = 0; i < length; i ++ ) {
      if ( getBufferSamples() <= pos ) {
        for ( int j = i; j < length; j ++ ) {
          out[ i ] = 0.0;
        }
        break;
      }
      
      if ( pos < 0.0 ) {
        out[ i ] = 0.0;
      } else if ( nudgeGap != 0.0 ) {
        double nudgeMix = exp( -0.01 * i );
        out[ i ] = getSample( in, pos ) * ( 1.0 - nudgeMix ) + getSample( in, pos - nudgeGap ) * nudgeMix;
      } else {
        out[ i ] = getSample( in, pos );
      }
      
      pos += rate;
    }
    outpos = pos;
  }
  
  position = outpos;
  if ( getBufferSamples() <= position ) {
    position = getBufferSamples() - 1.0;
    stop();
  }
}

void AudioPlayer::reset() {
  loaded = false;
  playing = false;
  scratching = false;

  position = 0;
  for ( int i = 0; i < CUE_N; i ++ ) {
    cuePosition[ i ] = 0;
  }
}

bool AudioPlayer::loadAudio( String path ) {
  if ( isPlaying() ) { return false; }
  reset();

  const File file( path );
  ScopedPointer<AudioFormatReader> reader( formatManager.createReaderFor( file ) );
  
  if ( reader != nullptr ) {
    buffer.setSize( reader->numChannels, reader->lengthInSamples );
    reader->read( &buffer, 0, reader->lengthInSamples, 0, true, true );
    bufferSampleRate = reader->sampleRate;
  } else {
    return false;
  }
  
  if ( 0 < buffer.getNumChannels() ) {
    loaded = true;

    int dataLen = getBufferSamples() * sizeof( float );
    bufferHash = SHA256( getBufferPointer( 0 ), fmin( dataLen, getBufferSampleRate() * 5.0 * sizeof( float ) ) ).toHexString().substring( 0, 16 );

    loadProps();
  } else {
    return false;
  }

  return true;
}

// ------

double AudioPlayer::getProp( String key ) {
  if ( !isLoaded() ) { return 0.0; }

  XmlElement* prop = appPropsElementBuffer->getChildByName( key );
  if ( prop != nullptr ) {
    return prop->getDoubleAttribute( "value" );
  } else {
    return 0.0;
  }
}

void AudioPlayer::setProp( String key, double value ) {
  if ( !isLoaded() ) { return; }

  XmlElement* prop = appPropsElementBuffer->getChildByName( key );
  if ( prop == nullptr ) {
    prop = new XmlElement( key );
    appPropsElementBuffer->addChildElement( prop );
  }
  prop->setAttribute( "value", value );

  appPropsFile->setValue( "props", appPropsElement );
}

void AudioPlayer::initProps() {
  if ( !isLoaded() ) { return; }

  appPropsElementBuffer = new XmlElement( "buffer" );
  appPropsElementBuffer->setAttribute( "hash", bufferHash );
  appPropsElement->addChildElement( appPropsElementBuffer );

  setProp( "bpm", 140.0 );
  setProp( "cue1", 0.0 );
  setProp( "cue2", 0.0 );
  setProp( "cue3", 0.0 );
  setProp( "cue4", 0.0 );
  setProp( "cue5", 0.0 );
}

bool AudioPlayer::loadProps() {
  if ( !isLoaded() ) { return false; }
  
  appPropsFile->reload();
  appPropsElement = appPropsFile->getXmlValue( "props" );

  appPropsElementBuffer = appPropsElement->getChildByAttribute( "hash", bufferHash );
  if ( appPropsElementBuffer == nullptr ) {
    initProps();
  }

  bpm = getProp( "bpm" );
  for ( int i = 0; i < CUE_N; i ++ ) {
    cuePosition[ i ] = getProp( "cue" + String( i + 1 ) );
  }
}

// ------

// super sinc interpolation guy
float AudioPlayer::getSample( const float* buffer, double index ) {
  double sum = 0.0;
  for ( int i = -5; i <= 5; i ++ ) {
    int ii = int( floor( index + 0.5 ) ) + i;
    if ( ii < 0 || getBufferSamples() < ii ) { continue; }
    
    double d = ii - index;
    sum += buffer[ ii ] * ( d == 0.0 ? 1.0 : sin( d * PI ) / d / PI );
  }
  return sum;
}

// ------

bool AudioPlayer::isLoaded() { return loaded; }
bool AudioPlayer::isPlaying() { return playing; }
bool AudioPlayer::isScratching() { return scratching; }
int AudioPlayer::getBufferChannels() { return loaded ? buffer.getNumChannels() : 0; }
int AudioPlayer::getBufferSamples() { return loaded ? buffer.getNumSamples() : 0; }
double AudioPlayer::getBpm() { return bpm; }
double AudioPlayer::getPosition() { return position; }
double AudioPlayer::getCuePosition( int index ) { return cuePosition[ index ]; }
double AudioPlayer::getBufferSampleRate() { return loaded ? bufferSampleRate : 0.0; }
double AudioPlayer::getPlaybackRate() { return playbackRate; }
const float* AudioPlayer::getBufferPointer( int ch ) { return buffer.getReadPointer( ch ); }