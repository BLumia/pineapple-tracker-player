#pragma once

#include <thread>

#include <QObject>

namespace openmpt {
    class module_ext;
    namespace ext {
        class interactive;
    }
}

namespace portaudio {
    template<typename T>
    class MemFunCallbackStream;
}
struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;

struct CachedPlaybackState
{
    std::int32_t currentOrder;
    std::int32_t currentPattern;
    std::int32_t currentRow;
};

struct PlaybackOptions
{
    std::int32_t repeatCount = -1;
};

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);
    ~Player();

    Q_PROPERTY(int currentOrder READ currentOrder NOTIFY currentOrderChanged)
    Q_PROPERTY(int currentPattern READ currentPattern NOTIFY currentPatternChanged)
    Q_PROPERTY(int currentRow READ currentRow NOTIFY currentRowChanged)

    bool load(const QUrl &filename);
    void play();

    int streamCallback(const void *inputBuffer, void *outputBuffer, unsigned long numFrames,
                       const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags);

    std::int32_t currentOrder() const;
    std::int32_t totalOrders() const;
    std::int32_t currentPattern() const;
    std::int32_t totalPatterns() const;
    std::int32_t currentRow() const;
    std::int32_t patternTotalRows(std::int32_t pattern) const;
    std::int32_t positionSec() const;
    std::int32_t durationSec() const;

    QString title() const;
    QString message() const;

    void seek(std::int32_t order, std::int32_t row = 0);

signals:
    void currentOrderChanged();
    void currentPatternChanged();
    void currentRowChanged();
    void fileLoaded();

private:
    void updateCachedState();

private:
    portaudio::MemFunCallbackStream<Player> * m_stream;
    openmpt::module_ext * m_module = nullptr;
    openmpt::ext::interactive * m_interactive = nullptr;
    CachedPlaybackState m_cachedState;
    PlaybackOptions m_options;
};

