/*
  ==============================================================================

    SoundfontAudioSource.h
    Created: 4 May 2018 2:10:01pm
    Author:  Chris Penny

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"

//==========================================================================
//==========================================================================
/** This uses the Fluidlite library to load soundfont files and play them.
    By default it uses MIDI channel 1, though you can optionally specify a channel.
    To inspect or create soundfont files, I recommend the "Polyphone" app.
 */
class SoundfontAudioSource   :   public AudioSource
{
public:
    
    /** Initializes fluidsynth. */
    SoundfontAudioSource(int numberOfVoices = 256);
    
    /** Destructor */
    ~SoundfontAudioSource();
    
    /** AudioSource Methods */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    /** Load a .sf2 file. Will not reload a file if it is already loaded.
        If another file is loaded, it will unload that first. */
    bool loadSoundfont (const File file);
    
    /** Sends an incoming midi message to fluidsynth */
    void processMidi (const MidiMessage& message);
    
    /** Send a noteon message. */
    void noteOn (int note, int velocity, int channel = 1);
    
    /** Send a noteoff message. */
    void noteOff (int note, int channel = 1);
    
    /** Send a continuous controller message. */
    void cc (int control, int value, int channel = 1);
    
    /** Get a continuous controller value. */
    int getCc (int control, int channel = 1);
    
    /** Send a pitch bend message. */
    void pitchBend (int value, int channel = 1);
    
    /** Get the current pitch bend value. */
    int getPitchBend (int channel = 1);
    
    /** Set the pitch wheel sensitivity. */
    void setPitchBendRange (int value, int channel = 1);
    
    /** Get the pitch wheel sensitivity. */
    int getPitchBendRange (int channel = 1);
    
    /** Send a channel pressure message. */
    void channelPressure(int value, int channel = 1);
    
    /** Set the fluidsynth gain */
    void setGain (float gain);
    
    /** Get the fluidsynth gain */
    float getGain();
    
    /** Send a reset. A reset turns all the notes off and resets the
        controller values. */
    void systemReset();
    
    /** Returns the raw fluid_synth_t object for direct use with the Fluidsynth API. */
    fluid_synth_t* getSynth()       { return synth; }
    
    /** Returns the raw settings for use with the Fluidsynth API. */
    fluid_settings_t* getSettings() { return settings; }
    
private:
    
    CriticalSection lock;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    int sfontID;
    File loadedSoundfont;
};
