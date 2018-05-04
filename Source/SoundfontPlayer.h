/*
  ==============================================================================

    jamboxx_Soundfonts.h
    Created: 2 Mar 2018 9:39:49am
    Author:  Chris Penny

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"
#include "jamboxx_MidiTools.h"

//==============================================================================
//==============================================================================
/**
    Basic utility functions for finding & loading soundfonts along with a soundfont player.
 */
namespace Soundfonts
{
    /**
        Returns the directory that contains all the soundfonts.
     
        On Mac, these are embedded within the application bundle as an Xcode Resource.

        On Windows, these are installed to a special folder.

        It would be trivial to also allow users to pick a custom folder for their own soundfonts,
        should the need ever arise.
     */
    const File getDirectory();
    
    /**
        Returns all available soundfont files.
     
        On Mac, these are embedded within the application bundle as an Xcode Resource.
     
        On Windows, these are installed to a special folder.
     
        It would be trivial to also allow users to pick a custom folder for their own soundfonts,
        should the need ever arise.
     */
    const Array<File> getAllFiles();
    
    /**
        Returns the list of soundfonts as well-formatted names to display to the user. Supports
        translation if the name of the soundfont is in the relevant translation file.
     */
    const StringArray getAllNames();
    
    /**
        Returns a single soundfont file from its name as returned by getName().
     */
    const File getFile (const String& name);
    
    /**
        Returns a well-formatted & potentially translated name from a soundfont file.
     */
    const String getName (const File& file);

    //==========================================================================
    //==========================================================================
    /**
        This uses the Fluidlite library to load soundfont files and play them.
     
        By default it uses MIDI channel 1, though you can optionally specify a channel.
     
        To inspect or create soundfont files, I recommend the "Polyphone" app.
     */
    class Player   :   public AudioSourceMidiListener
    {
    public:
        
        /** Load a .sf2 file. Will not reload a file if it is already loaded.
            If another file is loaded, it will unload that first. */
        bool loadSoundfont (const File file);
        
        /** Load a .sf2 file. Will not reload a file if it is already loaded.
            If another file is loaded, it will unload that first.
            Uses the file provided Soundfonts::getFile(const String& name).
         */
        bool loadNamedSoundfont (const String& name);

        const String getLoadedSoundfontName();
        
        /** Sends an incoming midi message to fluidsynth */
        void processMidi (const MidiMessage& message) override;
        
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
        
        /** Set the breath control */
        void setBreathControl (double gain);
        
        /** Get the breath control */
        double getBreathControl();
        
        /** Send a reset. A reset turns all the notes off and resets the
            controller values. */
        void systemReset();
        
        /** Tells the source to prepare for playing.

            An AudioSource has two states: prepared and unprepared.

            The prepareToPlay() method is guaranteed to be called at least once on an 'unpreprared'
            source to put it into a 'prepared' state before any calls will be made to getNextAudioBlock().
            This callback allows the source to initialise any resources it might need when playing.

            Once playback has finished, the releaseResources() method is called to put the stream
            back into an 'unprepared' state.

            Note that this method could be called more than once in succession without
            a matching call to releaseResources(), so make sure your code is robust and
            can handle that kind of situation.

            @param samplesPerBlockExpected  the number of samples that the source
            will be expected to supply each time its
            getNextAudioBlock() method is called. This
            number may vary slightly, because it will be dependent
            on audio hardware callbacks, and these aren't
            guaranteed to always use a constant block size, so
            the source should be able to cope with small variations.
            @param sampleRate               the sample rate that the output will be used at - this
            is needed by sources such as tone generators.
            @see releaseResources, getNextAudioBlock
         */
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        
        /** Allows the source to release anything it no longer needs after playback has stopped.
         
            This will be called when the source is no longer going to have its getNextAudioBlock()
            method called, so it should release any spare memory, etc. that it might have
            allocated during the prepareToPlay() call.

            Note that there's no guarantee that prepareToPlay() will actually have been called before
            releaseResources(), and it may be called more than once in succession, so make sure your
            code is robust and doesn't make any assumptions about when it will be called.

            @see prepareToPlay, getNextAudioBlock
         */
        void releaseResources() override;
        
        /** Called repeatedly to fetch subsequent blocks of audio data.

            After calling the prepareToPlay() method, this callback will be made each
            time the audio playback hardware (or whatever other destination the audio
            data is going to) needs another block of data.

            It will generally be called on a high-priority system thread, or possibly even
            an interrupt, so be careful not to do too much work here, as that will cause
            audio glitches!

            @see AudioSourceChannelInfo, prepareToPlay, releaseResources
         */
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        
        /**
            Returns the raw fluid_synth_t object for direct use with the Fluidsynth API.
         */
        fluid_synth_t* getSynth()       { return synth; }
        
        /**
             Returns the raw settings for use with the Fluidsynth API.
         */
        fluid_settings_t* getSettings() { return settings; }
        
        /** Initializes fluidsynth. */
        Player();
        
        /** Destructor */
        ~Player();
        
    private:
        
        float convertBreathValue (double breathValue);
        
        /** Prevents fluidsynth from writing audio while soundfont is being loaded */
        CriticalSection lock;
        fluid_settings_t *settings;
        fluid_synth_t    *synth;
        int sfontID;
        File loadedSoundfont;
        Atomic<double> breathControl;
        Atomic<double> newBreathControlValue;
    };
};
