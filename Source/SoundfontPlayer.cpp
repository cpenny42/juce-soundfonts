/*
  ==============================================================================

    jamboxx_Soundfonts.cpp
    Created: 2 Mar 2018 9:39:49am
    Author:  Chris Penny

  ==============================================================================
*/

#include "jamboxx_Soundfonts.h"
#include "jamboxx_Device.h"

const File Soundfonts::getDirectory()
{
    File soundfontDirectory (
#if JUCE_WINDOWS

#if DEBUG
        File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectory)
        .getChildFile("My Music Machines Inc")
        .getChildFile("Jamboxx")
        .getChildFile("Resources")
        .getChildFile("Soundfonts")
#else
        File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile)
                             .getParentDirectory()
                             .getChildFile("Resources")
                             .getChildFile("Soundfonts")
#endif
#elif JUCE_MAC
        File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile)
                             .getParentDirectory()
                             .getParentDirectory()
                             .getChildFile("Resources")
#elif JUCE_IOS
        File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile)
#elif JUCE_ANDROID
        File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile)
#elif JUCE_LINUX
        File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile)
                             .getParentDirectory()
                             .getChildFile("Resources")
                             .getChildFile("Soundfonts")
#else
                             // Not supported
                             jassertfalse;
#endif
                             );
    return soundfontDirectory;
}

const Array<File> Soundfonts::getAllFiles()
{
    File soundfontDirectory (getDirectory());
    
    Array<File> files;
    DirectoryIterator iter (soundfontDirectory, true, "*.sf2");
    
    while (iter.next()) {
        files.add(iter.getFile());
    }
    return files;
}

const StringArray Soundfonts::getAllNames()
{
    StringArray names;
    Array<File> files = Soundfonts::getAllFiles();
    for (auto file : files) {
        names.add (Soundfonts::getName(file));
    }
    return names;
}

const File Soundfonts::getFile(const String& name)
{
    const StringArray names = Soundfonts::getAllNames();
    const int index = names.indexOf(name);
    return index == -1 ? File() : Soundfonts::getAllFiles()[index];
}

const String Soundfonts::getName(const File& file)
{
    return TRANS(file.getFileNameWithoutExtension().replace("_", " ", true));
}

//==============================================================================
//==============================================================================
Soundfonts::Player::Player()
: breathControl (1)
, newBreathControlValue (1)
{
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    
    fluid_synth_set_gain(synth, 10.0);
    fluid_synth_set_polyphony(synth, 256);
    setGain(1.0f);
}

Soundfonts::Player::~Player()
{
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void Soundfonts::Player::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    fluid_synth_set_sample_rate(synth, (float) sampleRate);
}

void Soundfonts::Player::releaseResources()
{
    
}

float Soundfonts::Player::convertBreathValue (double breathValue)
{
    int rawExponent = Device::getPressureExponent();
    
    double normExponent = (double)rawExponent / (double) INT_MAX;
    normExponent = pow(normExponent, 2) * 5.0;
    return (float) (breathValue * (1 + normExponent));
}

void Soundfonts::Player::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    const ScopedLock l (lock);

    fluid_synth_write_float(synth,
                            bufferToFill.buffer->getNumSamples(),
                            bufferToFill.buffer->getWritePointer(0), 0, 1,
#if JUCE_IOS
                            bufferToFill.buffer->getWritePointer(0), 0, 1);
#else
                            bufferToFill.buffer->getWritePointer(1), 0, 1);
#endif
    
    float startGainValue = convertBreathValue (breathControl.get());
    float endGainValue = convertBreathValue (newBreathControlValue.get());
    float delta = (endGainValue - startGainValue) / (float) bufferToFill.buffer->getNumSamples();
    float currentGain = startGainValue;
    for (int s = 0; s < bufferToFill.buffer->getNumSamples(); s++) {
        bufferToFill.buffer->applyGain(s, 1, currentGain);
        currentGain += delta;
    }
    breathControl.set (newBreathControlValue.get());
}

bool Soundfonts::Player::loadSoundfont(const File file)
{
    if (file == loadedSoundfont) {
        // Don't reload an already loaded soundfont
        return false;
    }
    loadedSoundfont = file;
    const ScopedLock l (lock);
    fluid_synth_system_reset(synth);
    if (fluid_synth_sfcount(synth) > 0) {
        int err = fluid_synth_sfunload(synth, (unsigned int) sfontID, true);
        if (err == -1) {
            return false;
        }
    }
    sfontID = fluid_synth_sfload(synth, file.getFullPathName().toRawUTF8(), true);
    
    if (sfontID == -1) {
        return false;
    }
    else {
        return true;
    }
}

bool Soundfonts::Player::loadNamedSoundfont(const String& name)
{
    return loadSoundfont(Soundfonts::getFile(name));
}

const String Soundfonts::Player::getLoadedSoundfontName()
{
    return Soundfonts::getName(loadedSoundfont);
}

void Soundfonts::Player::processMidi (const MidiMessage& message)
{
    if (message.isNoteOn()) {
        noteOn(message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff()) {
        noteOff(message.getNoteNumber());
    }
    else if (message.isController()) {
        cc(message.getControllerNumber(), message.getControllerValue());
    }
    else if (message.isPitchWheel()) {
        pitchBend(message.getPitchWheelValue());
    }
    else if (message.isChannelPressure()) {
        channelPressure(message.getChannelPressureValue());
    }
}

void Soundfonts::Player::noteOn (int note, int velocity, int channel)
{
//    DBG("    SoundfontPlayer::noteOn (" << note << ", " << velocity << ", " << channel << ")");
    velocity == 0
        ? fluid_synth_noteoff(synth, channel, note)
        : fluid_synth_noteon(synth, channel, note, velocity);
}

void Soundfonts::Player::noteOff (int note, int channel)
{
//    DBG("    SoundfontPlayer::noteOff (" << note << ", " << channel << ")");
    fluid_synth_noteoff(synth, channel, note);
}

void Soundfonts::Player::cc(int control, int value, int channel)
{
    fluid_synth_cc(synth, channel, control, value);
}

int Soundfonts::Player::getCc(int control, int channel)
{
    int value = 0;
    fluid_synth_get_cc(synth, channel, control, &value);
    return value;
}

void Soundfonts::Player::pitchBend(int value, int channel)
{
//    DBG("    SoundfontPlayer::pitchBend (" << value << ")");
    fluid_synth_pitch_bend(synth, channel, value);
}

int Soundfonts::Player::getPitchBend(int channel)
{
    int value = 0;
    fluid_synth_get_pitch_bend(synth, channel, &value);
    return value;
}

void Soundfonts::Player::setPitchBendRange(int value, int channel)
{
    fluid_synth_pitch_wheel_sens(synth, channel, value);
}

int Soundfonts::Player::getPitchBendRange(int channel)
{
    int value = 0;
    fluid_synth_get_pitch_wheel_sens(synth, channel, &value);
    return value;
}

void Soundfonts::Player::channelPressure(int value, int channel)
{
    fluid_synth_channel_pressure(synth, channel, value);
}

void Soundfonts::Player::setGain (float gain)
{
    fluid_synth_set_gain(synth, gain);
}

float Soundfonts::Player::getGain()
{
    return fluid_synth_get_gain(synth);
}

void Soundfonts::Player::systemReset()
{
    fluid_synth_system_reset(synth);
}

void Soundfonts::Player::setBreathControl (double gain)
{
    newBreathControlValue.set (gain);
}

double Soundfonts::Player::getBreathControl()
{
    return breathControl.get();
}
