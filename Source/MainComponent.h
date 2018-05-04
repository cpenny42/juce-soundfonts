/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SoundfontAudioSource.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent, public ComboBox::Listener, public MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    // AudioSource
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Component
    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    // ComboBox::Listener
    //==============================================================================
    void comboBoxChanged (ComboBox* comboBoxThatWasChanged) override;
    
    // MidiKeyboardStateListener
    //==============================================================================
    void handleNoteOn (MidiKeyboardState* source,
                       int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (MidiKeyboardState* source,
                        int midiChannel, int midiNoteNumber, float velocity) override;
    
    
    //==============================================================================
    const File getSoundfontsDirectory()
    {
        // In this demo, the soundfonts are stored next to the Source folder that contains this file
        return File (String(__FILE__))
                .getParentDirectory()
                .getParentDirectory()
                .getChildFile("Soundfonts");
    }
    
    
    
    const File getFileFromName(const String& name)
    {
        return soundfontFiles[soundfontNames.indexOf (name)];
    }

private:
    //==============================================================================
    // Your private member variables go here...

    SoundfontAudioSource soundfontPlayer;
    MidiKeyboardState keyState;
    Array<File> soundfontFiles;
    StringArray soundfontNames;
    
    ScopedPointer<MidiKeyboardComponent> keyboardComponent;
    ScopedPointer<ComboBox> soundfontSelector;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
