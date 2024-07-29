#pragma once

#include <thread>

#include <QObject>

namespace openmpt {
    class module_ext;
    namespace ext {
        class interactive;
    }
}

typedef void PaStream;

struct CachedPlaybackState
{
    std::int32_t currentOrder;
    std::int32_t currentPattern;
    std::int32_t currentRow;
};

struct PlaybackOptions
{
    std::int32_t repeatCount = -1; // -1: inf loop, 0: single time, 1+: 2+ times.
    std::int32_t gain = 0; // 100 * dB
};

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);
    ~Player();

    Q_PROPERTY(int currentOrder READ currentOrder NOTIFY currentOrderChanged)
    Q_PROPERTY(int totalOrders READ totalOrders NOTIFY totalOrdersChanged)
    Q_PROPERTY(int positionSec READ positionSec)
    Q_PROPERTY(int currentPattern READ currentPattern NOTIFY currentPatternChanged)
    Q_PROPERTY(int currentRow READ currentRow NOTIFY currentRowChanged)
    Q_PROPERTY(int repeatCount READ repeatCount WRITE setRepeatCount NOTIFY repeatCountChanged)
    Q_PROPERTY(int gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStatusChanged)

    Q_INVOKABLE bool load(const QUrl &filename);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    bool isPlaying() const;

    bool setupAndStartStream();
    int streamCallback(const void *inputBuffer, void *outputBuffer, unsigned long numFrames);

    std::int32_t currentOrder() const;
    std::int32_t totalOrders() const;
    std::int32_t currentPattern() const;
    std::int32_t totalPatterns() const;
    std::int32_t currentRow() const;
    Q_INVOKABLE std::int32_t patternTotalRows(std::int32_t pattern) const;
    std::int32_t positionSec() const;
    std::int32_t durationSec() const;
    Q_INVOKABLE std::int32_t totalChannels() const;
    std::int32_t currentSubsong() const;
    std::int32_t totalSubsongs() const;
    Q_INVOKABLE QStringList instrumentNames() const;
    Q_INVOKABLE std::int32_t totalInstruments() const;
    std::int32_t totalSamples() const;

    Q_INVOKABLE QString title() const;
    Q_INVOKABLE QString artist() const;
    Q_INVOKABLE QString tracker() const;
    Q_INVOKABLE QString message() const;
    Q_INVOKABLE QVector<QStringList> patternContent(std::int32_t pattern) const;

    std::int32_t repeatCount() const;
    void setRepeatCount(std::int32_t repeatCount);
    Q_INVOKABLE void seek(std::int32_t order, std::int32_t row = 0);
    Q_INVOKABLE void setChannelMuteStatus(std::int32_t channel, bool mute);
    Q_INVOKABLE void setInstrumentMuteStatus(std::int32_t instrument, bool mute);
    Q_INVOKABLE void setSubsong(std::int32_t subsong);
    Q_INVOKABLE void setGlobalVolume(double volume);
    std::int32_t gain() const;
    void setGain(std::int32_t dBx100 = 0);

signals:
    void currentOrderChanged();
    void totalOrdersChanged();
    void currentPatternChanged();
    void currentRowChanged();
    void fileLoaded();
    void endOfSongReached();
    void playbackStatusChanged(); // can be emitted even if it's not changed
    void repeatCountChanged();
    void gainChanged();

private:
    void updateCachedState();

private:
    PaStream * m_stream;
    std::function<void(unsigned int)> mf_playbackCallback;
    openmpt::module_ext * m_module = nullptr;
    openmpt::ext::interactive * m_interactive = nullptr;
    CachedPlaybackState m_cachedState;
    PlaybackOptions m_options;
    bool m_isPlaying;
};

