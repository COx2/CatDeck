// ref: https://www.juce.com/doc/tutorial_looping_audio_sample_buffer

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ------

CatDeckAudioProcessor::CatDeckAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", AudioChannelSet::stereo(), true)
#endif
	)
#endif
{ // construtor
}

CatDeckAudioProcessor::~CatDeckAudioProcessor() { // destructor
}

// ------

const String CatDeckAudioProcessor::getName() const { return JucePlugin_Name; }
double CatDeckAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int CatDeckAudioProcessor::getNumPrograms() { return 1; }
int CatDeckAudioProcessor::getCurrentProgram() { return 0; }
void CatDeckAudioProcessor::setCurrentProgram( int index ) {}
const String CatDeckAudioProcessor::getProgramName( int index ) { return String(); }
void CatDeckAudioProcessor::changeProgramName( int index, const String& newName ) {}

bool CatDeckAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool CatDeckAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool CatDeckAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

// ------

void CatDeckAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock ) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CatDeckAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CatDeckAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void CatDeckAudioProcessor::processBlock( AudioSampleBuffer& buffer, MidiBuffer& midiMessages ) {
  currentPositionValid = getPlayHead()->getCurrentPosition( currentPosition );
  
  audioPlayer.setPlaybackRate( isCurrentPositionValid() ? currentPosition.bpm / audioPlayer.getBpm() : 1.0 );
  audioPlayer.processBlock( buffer, *this );
}

bool CatDeckAudioProcessor::isCurrentPositionValid() {
  return currentPositionValid;
}

// ------

bool CatDeckAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* CatDeckAudioProcessor::createEditor() {
  return new CatDeckAudioProcessorEditor( *this );
}

// ------

void CatDeckAudioProcessor::getStateInformation( MemoryBlock& destData ) {
}

void CatDeckAudioProcessor::setStateInformation( const void* data, int sizeInBytes ) {
}

// ------

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CatDeckAudioProcessor(); }
