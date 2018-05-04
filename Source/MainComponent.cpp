/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Populate Soundfonts
    soundfontFiles = getSoundfontsDirectory().findChildFiles(File::findFiles, false, "*.sf2");
    for (auto f : soundfontFiles) {
        soundfontNames.add (f.getFileNameWithoutExtension());
    }
    
    addAndMakeVisible (soundfontSelector = new ComboBox ("SoundfontSelector"));
    soundfontSelector->addListener (this);
    soundfontSelector->addItemList(soundfontNames, 1);
    soundfontSelector->setSelectedItemIndex(0);
    
    addAndMakeVisible (keyboardComponent = new MidiKeyboardComponent (keyState, MidiKeyboardComponent::horizontalKeyboard));
    keyState.addListener (this);
    
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 200);

    // specify the number of input and output channels that we want to open
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    soundfontPlayer.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    
    soundfontPlayer.getNextAudioBlock (bufferToFill);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    
    soundfontPlayer.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    const int selectorHeight = 50;

    soundfontSelector->setBounds (0, 0, getWidth(), selectorHeight);
    keyboardComponent->setBounds (0, selectorHeight, getWidth(), getHeight() - selectorHeight);
}

void MainComponent::comboBoxChanged (ComboBox* comboBoxThatWasChanged)
{
    if (comboBoxThatWasChanged == soundfontSelector) {
        File soundfontFile = getFileFromName (soundfontSelector->getText());
        if (! soundfontPlayer.loadSoundfont (soundfontFile)) {
            // Error loading soundfont
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Invalid .sf2 File",
                                        "Could not find your soundfont file: "
                                        + soundfontFile.getFullPathName());
        }
    }
}

void MainComponent::handleNoteOn (MidiKeyboardState* source,
                   int midiChannel, int midiNoteNumber, float velocity)
{
    soundfontPlayer.noteOn(midiNoteNumber, velocity * 127, midiChannel);
}

void MainComponent::handleNoteOff (MidiKeyboardState* source,
                    int midiChannel, int midiNoteNumber, float velocity)
{
    soundfontPlayer.noteOff (midiNoteNumber, midiChannel);
}
