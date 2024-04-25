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

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        auto voice = dynamic_cast<LifeSynthVoice*>(synth.getVoice(i));
        voice->setInitialState(apvts.getRawParameterValue("lifeInitState"), apvts.getRawParameterValue("lifeRandomNumber"), sampleRate);
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

    panning.setSampleRate(sampleRate);

    // Initialise the filter which changes cutoff based on LFOs 
    changingFilter.createLFOs(sampleRate);
    changingFilter.setLFOFrequencies(0.5f, 0.1f);
    changingFilter.initFilter();


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
        buffer.clear(i, 0, buffer.getNumSamples());

    // Store the number of samples 
    int numSamples = buffer.getNumSamples();

    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);


    // Process the buffer with the synth
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    for (int i = 0; i < numSamples; i++)
    {
        // Change coefficients of the low pass filter
        changingFilter.setCutoff(700, 100);
        changingFilter.setFilterCoefs();

        float sample = buffer.getSample(0, i);

        left[i] = changingFilter.process(sample);
        right[i] = changingFilter.process(sample);
    }
   

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

    if (apvts.getRawParameterValue("panningChoice")->load() == 1)
    {

        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);

        panning.setFrequency(apvts.getRawParameterValue("panningRate")->load());
        for (int i = 0; i < numSamples; i++)
        {
            float sample = buffer.getSample(0, i);
            std::vector<float> samples = panning.process(sample);
            left[i] = samples[0];
            right[i] = samples[1];
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
