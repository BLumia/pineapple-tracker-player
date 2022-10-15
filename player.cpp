#include "player.h"

#include <QUrl>
#include <QDebug>

#include <fstream>

#include <libopenmpt/libopenmpt.hpp>
#include <libopenmpt/libopenmpt_ext.hpp>

#include <portaudiocpp/PortAudioCpp.hxx>

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
    portaudio::AutoSystem pa_initializer;
    pa_initializer.initialize();
    portaudio::System & pa = portaudio::System::instance();
    portaudio::Device & outputDevice = pa.defaultOutputDevice();
    portaudio::DirectionSpecificStreamParameters inputParams(
                portaudio::DirectionSpecificStreamParameters::null());
    portaudio::DirectionSpecificStreamParameters outputParams(
                outputDevice, 2, portaudio::FLOAT32, true,
                outputDevice.defaultHighOutputLatency(), 0);
    portaudio::StreamParameters stream_params(
                inputParams, outputParams,
                SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag);
    m_stream = new portaudio::MemFunCallbackStream<Player>(stream_params, *this, &Player::streamCallback);
    m_stream->start();
}

Player::~Player()
{

}

bool Player::load(const QUrl &filename)
{
    std::ifstream file(filename.toLocalFile().toStdString(), std::ios::binary);
    try {
        m_module = new openmpt::module_ext(file);
    } catch (const openmpt::exception & e) {
        qDebug() << "openmpt::exception:" << e.what() << Qt::endl;
        return false;
    }
    m_interactive = static_cast<openmpt::ext::interactive*>(m_module->get_interface(openmpt::ext::interactive_id));

    m_module->set_repeat_count(m_options.repeatCount);

    emit fileLoaded();

    return true;
}

void Player::play()
{
    if (!m_module) return;
    m_isPlaying = true;
}

void Player::pause()
{
    m_isPlaying = false;
}

int Player::streamCallback(const void *inputBuffer, void *outputBuffer, unsigned long numFrames,
                           const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags)
{
    float* buffer = (float*)outputBuffer;
    if (m_module && m_isPlaying) {
        std::size_t count = m_module->read_interleaved_stereo(SAMPLE_RATE, numFrames, buffer);
        updateCachedState();
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

QString Player::message() const
{
    return m_module ? QString::fromStdString(m_module->get_metadata("message")) : QString();
}

void Player::seek(int32_t order, int32_t row)
{
    if (!m_module) return;
    m_module->set_position_order_row(order, row);
}

void Player::setGlobalVolume(double volume)
{
    if (!m_interactive) return;
    m_interactive->set_global_volume(volume);
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
