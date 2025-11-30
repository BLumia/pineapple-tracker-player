// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "abstractplayer.h"

#include <cstdio>
#include <portaudio.h>

AbstractPlayer::AbstractPlayer()
{
    PaError initErr = Pa_Initialize();
    if (initErr != paNoError) {
        fprintf(stderr, "Pa_Initialize failed: %s\n", Pa_GetErrorText(initErr));
    }
    if (setupAndStartStream() != StreamState::Ok) {
        fputs("Failed to open/start audio stream.\n", stderr);
    }
}

AbstractPlayer::~AbstractPlayer()
{
    if (m_stream) {
        PaError err = Pa_StopStream(m_stream);
        if (err != paNoError) {
            fprintf(stderr, "Pa_StopStream failed: %s\n", Pa_GetErrorText(err));
        }
        err = Pa_CloseStream(m_stream);
        if (err != paNoError) {
            fprintf(stderr, "Pa_CloseStream failed: %s\n", Pa_GetErrorText(err));
        }
        m_stream = nullptr;
    }

    Pa_Terminate();
}

AbstractPlayer::StreamState AbstractPlayer::play()
{
    if (!m_stream) {
        StreamState state = setupAndStartStream();
        if (state != StreamState::Ok) {
            fputs("Not able to create playback stream\n", stderr);
            return state;
        }
    } else {
        PaError active = Pa_IsStreamActive(m_stream);
        if (active == 0) {
            fputs("Playback stream stopped, restarting...", stderr);
            PaError err = Pa_StartStream(m_stream);
            if (err != paNoError) {
                fprintf(stderr, "Pa_StartStream failed: %s, reopening...\n", Pa_GetErrorText(err));
                StreamState state = setupAndStartStream();
                if (state != StreamState::Ok) {
                    fputs("Not able to restart playback stream\n", stderr);
                    return state;
                }
            }
        } else if (active < 0) {
            fprintf(stderr, "Pa_IsStreamActive error: %s, reopening...\n",
                Pa_GetErrorText(active));
            StreamState state = setupAndStartStream();
            if (state != StreamState::Ok) {
                fputs("Not able to restart playback stream\n", stderr);
                return state;
            }
        }
    }
    m_isPlaying.store(true);
    if (mf_onIsPlayingChanged)
        mf_onIsPlayingChanged(m_isPlaying);
    
    return StreamState::Ok;
}

void AbstractPlayer::pause()
{
    m_isPlaying.store(false);
    if (mf_onIsPlayingChanged)
        mf_onIsPlayingChanged(m_isPlaying);
}

void AbstractPlayer::stop()
{
    pause();
    onStop();
}

bool AbstractPlayer::isPlaying() const
{
    return m_isPlaying.load();
}

bool AbstractPlayer::loop() const
{
    return m_loop.load();
}

void AbstractPlayer::setLoop(bool loop)
{
    m_loop.store(loop);
}

void AbstractPlayer::setVolume(float volume)
{
    m_volume = volume;
}

AbstractPlayer::AudioSettings AbstractPlayer::currentAudioSettings() const
{
    return m_audioSettings;
}

std::vector<AbstractPlayer::DeviceInfo> AbstractPlayer::enumerateOutputDevices() const
{
    std::vector<DeviceInfo> list;
    int count = Pa_GetDeviceCount();
    if (count < 0)
        return list;
    int defaultIndex = Pa_GetDefaultOutputDevice();
    for (int i = 0; i < count; ++i) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (!info)
            continue;
        if (info->maxOutputChannels <= 0)
            continue;
        const PaHostApiInfo *api = Pa_GetHostApiInfo(info->hostApi);
        DeviceInfo di;
        di.index = i;
        di.name = info->name ? info->name : "";
        di.hostApi = api ? (api->name ? api->name : "") : "";
        di.maxOutputChannels = info->maxOutputChannels;
        di.defaultSampleRate = info->defaultSampleRate;
        di.defaultLowLatency = info->defaultLowOutputLatency;
        di.defaultHighLatency = info->defaultHighOutputLatency;
        di.isDefaultOutput = (i == defaultIndex);
        list.push_back(std::move(di));
    }
    return list;
}

AbstractPlayer::StreamState AbstractPlayer::applyAudioSettings(const AudioSettings &settings)
{
    fprintf(stderr, "Apply audio settings: device=%d, sr=%d, ch=%d, fpb=%lu\n",
            settings.deviceIndex, settings.sampleRate, settings.channels, settings.framesPerBuffer);
    bool wasPlaying = m_isPlaying.load();
    m_audioSettings = settings;
    StreamState state = setupAndStartStream();
    if (state != StreamState::Ok) {
        fprintf(stderr, "Apply audio settings failed, stream rebuild failed.\n");
        return state;
    }
    m_isPlaying.store(wasPlaying);
    return StreamState::Ok;
}

AbstractPlayer::StreamState AbstractPlayer::getStreamState() const
{
    return m_lastStreamState;
}

void AbstractPlayer::refreshDeviceList()
{
    if (m_stream) {
        Pa_StopStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
    }
    
    if (m_isPlaying.load()) {
        stop();
    }

    Pa_Terminate();
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Pa_Initialize failed during refresh: %s\n", Pa_GetErrorText(err));
    }
}

void AbstractPlayer::onIsPlayingChanged(std::function<void(bool)> cb)
{
    mf_onIsPlayingChanged = cb;
}

void AbstractPlayer::onStreamStateChanged(std::function<void(StreamState)> cb)
{
    mf_onStreamStateChanged = cb;
    if (mf_onStreamStateChanged) {
        mf_onStreamStateChanged(m_lastStreamState);
    }
}

void AbstractPlayer::setStreamState(StreamState state)
{
    if (m_lastStreamState != state) {
        m_lastStreamState = state;
        if (mf_onStreamStateChanged) {
            mf_onStreamStateChanged(state);
        }
    }
}

AbstractPlayer::StreamState AbstractPlayer::setupAndStartStream()
{
    if (m_stream) {
        Pa_StopStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
    }

    PaStreamParameters outParams{};
    outParams.device = (m_audioSettings.deviceIndex >= 0) ? m_audioSettings.deviceIndex
                                                          : Pa_GetDefaultOutputDevice();
    const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(outParams.device);
    if (!devInfo) {
        fprintf(stderr, "Invalid output device index: %d\n", outParams.device);
        setStreamState(StreamState::DeviceNotFound);
        return StreamState::DeviceNotFound;
    }
    outParams.channelCount = m_audioSettings.channels;
    outParams.sampleFormat = paFloat32;
    outParams.suggestedLatency = (m_audioSettings.suggestedLatency > 0.0)
            ? m_audioSettings.suggestedLatency
            : devInfo->defaultLowOutputLatency;
    outParams.hostApiSpecificStreamInfo = nullptr;

    double sr = (m_audioSettings.sampleRate > 0) ? (double)m_audioSettings.sampleRate
                                                 : devInfo->defaultSampleRate;
    PaError err = Pa_IsFormatSupported(nullptr, &outParams, sr);
    if (err != paFormatIsSupported) {
        fprintf(stderr, "Format not supported: device=%d (%s), ch=%d, sr=%.2f\n", outParams.device,
                devInfo->name ? devInfo->name : "?", outParams.channelCount, sr);
        setStreamState(StreamState::FormatNotSupported);
        return StreamState::FormatNotSupported;
    }

    fprintf(stderr, "Opening stream: device=%d (%s), host=%s, ch=%d, sr=%.2f, fpb=%lu\n",
            outParams.device, devInfo->name ? devInfo->name : "?",
            Pa_GetHostApiInfo(devInfo->hostApi) ? Pa_GetHostApiInfo(devInfo->hostApi)->name : "?",
            outParams.channelCount, sr,
            (m_audioSettings.framesPerBuffer == 0) ? paFramesPerBufferUnspecified
                                                   : m_audioSettings.framesPerBuffer);

    err = Pa_OpenStream(
            &m_stream, nullptr, &outParams, sr,
            (m_audioSettings.framesPerBuffer == 0) ? paFramesPerBufferUnspecified
                                                   : m_audioSettings.framesPerBuffer,
            paNoFlag,
            +[](const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                void *userData) -> int {
                return ((AbstractPlayer *)userData)
                        ->streamCallback(inputBuffer, outputBuffer, framesPerBuffer);
            },
            this);
    if (err != paNoError) {
        fprintf(stderr, "Pa_OpenStream failed: %s\n", Pa_GetErrorText(err));
        m_stream = nullptr;
        if (err == paInvalidDevice) {
            setStreamState(StreamState::DeviceNotFound);
            return StreamState::DeviceNotFound;
        }
        setStreamState(StreamState::StreamOpenFailed);
        return StreamState::StreamOpenFailed;
    }

    // sync chosen sr back to settings
    m_audioSettings.sampleRate = static_cast<int>(sr);

    err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        fprintf(stderr, "Pa_StartStream failed: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
        setStreamState(StreamState::StreamOpenFailed);
        return StreamState::StreamOpenFailed;
    }
    fprintf(stderr, "Stream started successfully.\n");
    setStreamState(StreamState::Ok);
    return StreamState::Ok;
}

int AbstractPlayer::streamCallback(const void *inputBuffer, void *outputBuffer,
                                   unsigned long numFrames)
{
    float *buffer = (float *)outputBuffer;

    if (m_isPlaying.load()) {
        renderAudio(buffer, numFrames);

        // Apply volume scaling to the rendered audio
        if (m_volume != 1.0f) {
            for (unsigned long i = 0; i < numFrames * 2; ++i) {
                buffer[i] *= m_volume;
            }
        }
    } else {
        std::fill(buffer, buffer + numFrames * 2, 0);
    }

    return paContinue;
}
