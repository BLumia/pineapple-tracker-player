// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QDialog>

namespace Ui { class AudioSettingsDialog; }

class AbstractPlayer;
class AudioSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AudioSettingsDialog(AbstractPlayer *player, QWidget *parent = nullptr);

private slots:
    void applySettings();

    void on_refreshDevicesButton_clicked();

private:
    void populateDevices();
    void populateSampleRates();
    void populateBufferSizes();

    Ui::AudioSettingsDialog *ui;
    AbstractPlayer *m_player; // NOT managed by this dialog
};
