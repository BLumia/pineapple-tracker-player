// SPDX-FileCopyrightText: 2024 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings *instance();

    QString applicationStyle() const;

    void setApplicationStyle(const QString & styleName);

    int volume() const;
    void setVolume(int volume);

    int audioDeviceIndex() const;
    void setAudioDeviceIndex(int index);

    int audioSampleRate() const;
    void setAudioSampleRate(int sampleRate);

    int audioFramesPerBuffer() const;
    void setAudioFramesPerBuffer(int framesPerBuffer);

private:
    Settings();

    static Settings *m_settings_instance;

    QSettings *m_qsettings;

signals:

public slots:
};
