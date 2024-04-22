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
        synth.addVoice(new LifeSynthVoice());
    }
    synth.addSound(new LifeSynthSound());
    synth.setNoteStealingEnabled(true);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        auto voice = dynamic_cast<LifeSynthVoice*>(synth.getVoice(i));
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
    // Set the sample rate for the synth
    synth.setCurrentPlaybackSampleRate(sampleRate);

    // Initialise the game of life state for each voice
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
        auto voice = dynamic_cast<LifeSynthVoice*>(synth.getVoice(i));
        voice->setInitialState(initialState, sampleRate);
    }

    // Reset the reverb and set the sample rate
    reverb.reset();
    reverb.setSampleRate(sampleRate);
    juce::Reverb::Parameters reverbParams;
    reverbParams.dryLevel = apvts.getRawParameterValue("reverbDry")->load();
    reverbParams.wetLevel = apvts.getRawParameterValue("reverbWet")->load();
    reverbParams.roomSize = apvts.getRawParameterValue("reverbRoomSize")->load();
    reverbParams.width = apvts.getRawParameterValue("reverbWidth")->load();
    reverbParams.damping = apvts.getRawParameterValue("reverbDamping")->load();
    
    // Set reverb parameters
    reverb.setParameters(reverbParams);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    chorus.prepare(spec);

    juce::File logFile("~/logfile.txt");
    logFile.deleteFile(); // Clear the log file at startup
    juce::Logger::setCurrentLogger(new juce::FileLogger(logFile, "Log Header", 0));
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

    // Process the buffer with the synth
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    // Create an AudioBlock from the given audio buffer. This wraps the buffer in a DSP-friendly format.
    juce::dsp::AudioBlock<float> block(buffer);

    // Create a processing context for replacing the audio in the block with processed audio.
    // This context is used to apply DSP effects directly to the audio block.
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Change chorus parameters
    chorus.setRate(apvts.getRawParameterValue("rate")->load());
    chorus.setDepth(apvts.getRawParameterValue("depth")->load());
    chorus.setMix(apvts.getRawParameterValue("mix")->load());

    // Process the audio block with the updated chorus effect parameters.
    chorus.process(context);
    
    // Check if the reverb is on or off. If it is off, there is no point changing the parameters
    if (apvts.getRawParameterValue("reverbChoice")->load() == 1)
    {
        // Change the reverb parameters if the reverb is turned on
        juce::Reverb::Parameters reverbParams;
        reverbParams.dryLevel = apvts.getRawParameterValue("reverbDry")->load();
        reverbParams.wetLevel = apvts.getRawParameterValue("reverbWet")->load();
        reverbParams.roomSize = apvts.getRawParameterValue("reverbRoomSize")->load();
        reverbParams.width = apvts.getRawParameterValue("reverbWidth")->load();
        reverbParams.damping = apvts.getRawParameterValue("reverbDamping")->load();
        reverb.setParameters(reverbParams);

        // Apply the reverb to the left and right channel
        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        reverb.processStereo(left, right, numSamples);
    }
    

    // Assuming 'synth' is your synthesizer object and it is a JUCE Synthesiser
    int activeVoices = 0;
    for (int i = 0; i < synth.getNumVoices(); ++i) {
        if (auto* voice = dynamic_cast<LifeSynthVoice*>(synth.getVoice(i))) {
            if (voice->isVoiceActive())
                ++activeVoices;
        }
    }

    // Now, adjust the output levels based on the number of active voices
    if (activeVoices > 0) {
        float compensationGain = 1.0f / static_cast<float>(activeVoices);
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            float* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i) {
                channelData[i] *= compensationGain;
            }
        }
    }

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
