#include "player.h"

#include <QUrl>
#include <QFile>
#include <QDebug>

#include <fstream>

#include <libopenmpt/libopenmpt.hpp>
#include <libopenmpt/libopenmpt_ext.hpp>

#include <portaudio.h>

constexpr std::int32_t SAMPLE_RATE = 48000;

QDebug& operator<<(QDebug& out, const std::string& str)
{
    out << QString::fromStdString(str);
    return out;
}

Player::Player(QObject *parent)
    : QObject{parent}
    , m_isPlaying(false)
{
    Pa_Initialize();
    setupAndStartStream();
}

Player::~Player()
{
    Pa_StopStream(m_stream);
}

bool Player::load(const QUrl &filename)
{
    QFile file(filename.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filename;
        return false;
    }
    QByteArray data(file.readAll());
    try {
        // not using ifstream here since it might cause filename encoding issue
        m_module = new openmpt::module_ext(data.data(), data.size());
    } catch (const openmpt::exception & e) {
        qDebug() << "openmpt::exception:" << e.what() << Qt::endl;
        return false;
    }
    m_interactive = static_cast<openmpt::ext::interactive*>(m_module->get_interface(openmpt::ext::interactive_id));

    // apply options
    m_module->set_repeat_count(m_options.repeatCount);
    m_module->set_render_param(openmpt::module::RENDER_MASTERGAIN_MILLIBEL, m_options.gain);

    emit fileLoaded();
    emit currentPatternChanged();
    emit totalOrdersChanged();

    return true;
}

void Player::play()
{
    if (!m_module) return;
    PaError ret = Pa_IsStreamActive(m_stream);
    if (ret == 0) {
        qDebug() << "Playback stream stopped, restarting...";
        bool succ = setupAndStartStream();
        if (!succ) {
            qDebug() << "Not able to restart playback stream";
            return;
        }
    }
    m_isPlaying = true;
    emit playbackStatusChanged();
}

void Player::pause()
{
    m_isPlaying = false;
    emit playbackStatusChanged();
}

int Player::streamCallback(const void *inputBuffer, void *outputBuffer, unsigned long numFrames)
{
    float* buffer = (float*)outputBuffer;
    if (m_module && m_isPlaying) {
        std::size_t count = m_module->read_interleaved_stereo(SAMPLE_RATE, numFrames, buffer);
        updateCachedState();
        if (count == 0) {
            // api like get_current_order() will return as it rewinded to the beginning of the song
            // but read() will still return 0 frame and won't write to the buffer...
            seek(0);
            m_isPlaying = false;
            emit playbackStatusChanged();
            emit endOfSongReached();
        }
    } else {
        std::fill(buffer, buffer + numFrames * 2, 0);
    }

    return 0;
}

int32_t Player::currentOrder() const
{
    return m_module ? m_module->get_current_order() : -1;
}

int32_t Player::totalOrders() const
{
    return m_module ? m_module->get_num_orders() : -1;
}

int32_t Player::currentPattern() const
{
    return m_module ? m_module->get_current_pattern() : -1;
}

int32_t Player::totalPatterns() const
{
    return m_module ? m_module->get_num_patterns() : -1;
}

int32_t Player::currentRow() const
{
    return m_module ? m_module->get_current_row() : -1;
}

int32_t Player::patternTotalRows(std::int32_t pattern) const
{
    return m_module ? m_module->get_pattern_num_rows(pattern) : -1;
}

/*!
 * \brief get current playback position in seconds
 *
 * \note position can be larger than the total duration since it can be infinitely looped
 */
int32_t Player::positionSec() const
{
    return m_module ? m_module->get_position_seconds() : -1;
}

int32_t Player::durationSec() const
{
    return m_module ? m_module->get_duration_seconds() : -1;
}

int32_t Player::totalChannels() const
{
    return m_module ? m_module->get_num_channels() : -1;
}

int32_t Player::currentSubsong() const
{
    return m_module ? m_module->get_selected_subsong() : -1;
}

int32_t Player::totalSubsongs() const
{
    return m_module ? m_module->get_num_subsongs() : -1;
}

QStringList Player::instrumentNames() const
{
    QStringList result;
    if (!m_module) return result;

    std::vector<std::string> && names = m_module->get_instrument_names();
    for (const std::string & name : names) {
        result << QString::fromStdString(name);
    }
    return result;
}

int32_t Player::totalInstruments() const
{
    return m_module ? m_module->get_num_instruments() : -1;
}

int32_t Player::totalSamples() const
{
    return m_module ? m_module->get_num_samples() : -1;
}

QString Player::title() const
{
    return m_module ? QString::fromStdString(m_module->get_metadata("title")) : QString();
}

// most of the case it's empty...
QString Player::artist() const
{
    return m_module ? QString::fromStdString(m_module->get_metadata("artist")) : QString();
}

QString Player::tracker() const
{
    return m_module ? QString::fromStdString(m_module->get_metadata("tracker")) : QString();
}

QString Player::message() const
{
    return m_module ? QString::fromStdString(m_module->get_metadata("message")) : QString();
}

QVector<QStringList> Player::patternContent(int32_t pattern) const
{
    if (!m_module) return {};

    std::int32_t channelCount = m_module->get_num_channels();
    QVector<QStringList> patternContent;
    for (std::int32_t row = 0; row < m_module->get_pattern_num_rows(pattern); row++) {
        QStringList content;
        for (std::int32_t channel = 0; channel < channelCount; channel++) {
//            content << QString::fromStdString(m_module->format_pattern_row_channel_command(pattern, row, channel, openmpt::module::command_index::command_note));
            content << QString::fromStdString(m_module->format_pattern_row_channel(pattern, row, channel, 13));
        }
        patternContent << content;
    }

    return patternContent;
}

bool Player::isPlaying() const
{
    return m_isPlaying;
}

bool Player::setupAndStartStream()
{
    Pa_OpenDefaultStream(&m_stream, 0, 2, paFloat32, SAMPLE_RATE, paFramesPerBufferUnspecified,
        +[](const void *inputBuffer, void *outputBuffer,
            unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags, void *userData) -> int {
            return ((Player *)userData)->streamCallback(inputBuffer, outputBuffer, framesPerBuffer);
        }, this);
    PaError err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        // TODO: logging?
        return false;
    }
    return true;
}

int32_t Player::repeatCount() const
{
    return m_options.repeatCount + 1;
}

// 0: inf, 1: single time, etc. NOT a direct wrapper for the libopenmpt API.
void Player::setRepeatCount(int32_t repeatCount)
{
    m_options.repeatCount = repeatCount - 1;
    if (m_module) m_module->set_repeat_count(m_options.repeatCount);
    emit repeatCountChanged();
}

void Player::seek(int32_t order, int32_t row)
{
    if (!m_module) return;
    m_module->set_position_order_row(order, row);
    emit currentPatternChanged();
    emit currentRowChanged();
}

void Player::setChannelMuteStatus(int32_t channel, bool mute)
{
    if (!m_interactive) return;
    m_interactive->set_channel_mute_status(channel, mute);
}

void Player::setInstrumentMuteStatus(int32_t instrument, bool mute)
{
    if (!m_interactive) return;
    m_interactive->set_instrument_mute_status(instrument, mute);
}

void Player::setSubsong(int32_t subsong)
{
    if (!m_module) return;
    try {
        m_module->select_subsong(subsong);
    } catch (const openmpt::exception & e) {
        qDebug() << "openmpt::exception:" << e.what() << Qt::endl;
    }
}

void Player::setGlobalVolume(double volume)
{
    if (!m_interactive) return;
    m_interactive->set_global_volume(volume);
}

int32_t Player::gain() const
{
    return m_options.gain;
}

void Player::setGain(std::int32_t dBx100)
{
    m_options.gain = dBx100;
    emit gainChanged();
    if (!m_module) return;
    m_module->set_render_param(openmpt::module::RENDER_MASTERGAIN_MILLIBEL, m_options.gain);
}

void Player::updateCachedState()
{
    if (!m_module) return;

    // maybe use setProperty() so the signal can be emitted automatically?
    std::int32_t curOrder = m_module->get_current_order();
    if (curOrder != m_cachedState.currentOrder) {
        emit currentOrderChanged();
        m_cachedState.currentOrder = curOrder;
    }

    std::int32_t curPattern = m_module->get_current_pattern();
    if (curPattern != m_cachedState.currentPattern) {
        emit currentPatternChanged();
        m_cachedState.currentPattern = curPattern;
    }

    std::int32_t curRow = m_module->get_current_row();
    if (curRow != m_cachedState.currentRow) {
        emit currentRowChanged();
        m_cachedState.currentRow = curRow;
    }
}
