/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthAudioProcessor::SynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr, "ParamTree", createParameterLayout())
{
    for (int i = 0; i < voiceCount; i++)
    {
        synth.addVoice(new MySynthVoice());
    }
    synth.addSound(new MySynthSound());
    synth.setNoteStealingEnabled(false);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        auto voice = dynamic_cast<MySynthVoice*>(synth.getVoice(i));
        voice->setParametersFromAPVTS(apvts);
    }
}

SynthAudioProcessor::~SynthAudioProcessor()
{
}

//==============================================================================
const juce::String SynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    std::vector<std::pair<int, int>> initialState;
    initialState.clear();
    initialState.push_back({3, 4});
    initialState.push_back({4, 4});
    initialState.push_back({5, 4});
    initialState.push_back({7, 0});
    initialState.push_back({7, 1});
    initialState.push_back({7, 2});
    initialState.push_back({9, 4});
    initialState.push_back({10, 4});
    initialState.push_back({11, 4});
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        auto voice = dynamic_cast<MySynthVoice*>(synth.getVoice(i));
        voice->setInitialState(initialState, sampleRate);
    }


    juce::File logFile("~/logfile.txt");
    logFile.deleteFile(); // Clear the log file at startup
    juce::Logger::setCurrentLogger(new juce::FileLogger(logFile, "Log Header", 0));

    juce::Logger::writeToLog("preparing to play");

}

void SynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void SynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Store the number of samples 
    int numSamples = buffer.getNumSamples();
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    /*bool isAnyKeyPressed = false;
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) isAnyKeyPressed = true;
        if (msg.isNoteOff()) isAnyKeyPressed = checkIfAnyOtherKeyIsPressed(midiMessages);
    }


    if (isAnyKeyPressed)
    {
        for (int i = 0; i < numSamples; i++)
        {
            float randomSample = random.nextFloat();
            buffer.addSample(0, i, randomSample); 
            buffer.addSample(1, i, randomSample); 
        }
    }*/
    //toneMatrix.process(buffer, midiMessages);

    // Combine samples
    /*for (int i = 0; i < numSamples; i++)
    {
        // Mix samples and store in mixBuffer 
        mixLeft[i] = (mixLeft[i] + synthLeft[i]) / 2.0f;
        mixRight[i] = (mixRight[i] + synthRight[i]) / 2.0f;


        // Copy final signal to acutal output buffer
        buffer.copyFrom(0, 0, mixBuffer, 0, 0, numSamples);
        buffer.copyFrom(1, 0, mixBuffer, 1, 0, numSamples);
    }*/

    // Prepare a buffer for the synth
    /*juce::AudioBuffer<float> synthBuffer(2, numSamples);
    synth.renderNextBlock(synthBuffer, midiMessages, 0, numSamples);

    // Initialize mix buffer to zero for summing
    buffer.clear();

    // Check if any key is pressed
    bool isAnyKeyPressed = false;
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) isAnyKeyPressed = true;
        if (msg.isNoteOff()) isAnyKeyPressed = checkIfAnyOtherKeyIsPressed(midiMessages);
    }

    // Generate white noise if any key is pressed
    juce::Random random;
    if (isAnyKeyPressed)
    {
        for (int i = 0; i < numSamples; i++)
        {
            float noiseSample = random.nextFloat() * 0.3f; // Adjust level as needed
            buffer.addSample(0, i, noiseSample);
            buffer.addSample(1, i, noiseSample);
        }
    }

    // Mix the synth buffer with the output buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* synthData = synthBuffer.getReadPointer(channel);
        for (int i = 0; i < numSamples; ++i)
        {
            channelData[i] += synthData[i]; // Mix synth output
        }
        buffer.copyFrom(channel, 0, channelData, 0, 0, numSamples);
    }*/
}


//==============================================================================
bool SynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void SynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes as intermediaries to make it easy to save and load complex data.
}
void SynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}
