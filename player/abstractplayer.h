// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <vector>

typedef void PaStream;

class AbstractPlayer
{
public:
    struct AudioSettings
    {
        int deviceIndex = -1;
        int sampleRate = 44100;
        int channels = 2;
        unsigned long framesPerBuffer = 0; // 0 = unspecified/auto
        double suggestedLatency = 0.0; // 0 = use device default
    };

    struct DeviceInfo
    {
        int index = -1;
        std::string name;
        std::string hostApi;
        int maxOutputChannels = 0;
        double defaultSampleRate = 0.0;
        double defaultLowLatency = 0.0;
        double defaultHighLatency = 0.0;
        bool isDefaultOutput = false;
    };

    enum class StreamState {
        Ok,
        DeviceNotFound,
        FormatNotSupported,
        StreamOpenFailed,
        UnknownError
    };

    virtual ~AbstractPlayer();

    StreamState play();
    void pause();
    void stop();
    bool isPlaying() const;
    void setLoop(bool loop);
    bool loop() const;
    void setVolume(float volume);

    AudioSettings currentAudioSettings() const;
    std::vector<DeviceInfo> enumerateOutputDevices() const;
    StreamState applyAudioSettings(const AudioSettings &settings);
    StreamState getStreamState() const;
    void refreshDeviceList();

    void onIsPlayingChanged(std::function<void(bool)> cb);
    void onStreamStateChanged(std::function<void(StreamState)> cb);

protected:
    AbstractPlayer();

    void setStreamState(StreamState state);

    // Pure virtual functions to be implemented by derived classes
    virtual void renderAudio(float *buffer, unsigned long numFrames) = 0;
    virtual void onStop() = 0;
    virtual void onSeek(unsigned int ms) = 0;

    StreamState setupAndStartStream();
    int streamCallback(const void *inputBuffer, void *outputBuffer, unsigned long numFrames);

    float m_volume = 1.0f;
    std::atomic<bool> m_isPlaying{ false };
    std::atomic<bool> m_loop{ true };

    PaStream *m_stream = nullptr;
    AudioSettings m_audioSettings;
    StreamState m_lastStreamState = StreamState::Ok;

    std::function<void(bool)> mf_onIsPlayingChanged;
    std::function<void(StreamState)> mf_onStreamStateChanged;
};
