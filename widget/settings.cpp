// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "settings.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

Settings *Settings::m_settings_instance = nullptr;

Settings *Settings::instance()
{
    if (!m_settings_instance) {
        m_settings_instance = new Settings;
    }

    return m_settings_instance;
}

QString Settings::applicationStyle() const
{
    return m_qsettings->value("application_style", QString()).toString();
}

void Settings::setApplicationStyle(const QString &styleName)
{
    m_qsettings->setValue("application_style", styleName);
    m_qsettings->sync();
}

#if defined(FLAG_PORTABLE_MODE_SUPPORT) && defined(Q_OS_WIN)
#include <windows.h>
// QCoreApplication::applicationDirPath() parses the "applicationDirPath" from arg0, which...
// 1. rely on a QApplication object instance
//    but we need to call QGuiApplication::setHighDpiScaleFactorRoundingPolicy() before QApplication get created
// 2. arg0 is NOT garanteed to be the path of execution
//    see also: https://stackoverflow.com/questions/383973/is-args0-guaranteed-to-be-the-path-of-execution
// This function is here mainly for #1.
QString getApplicationDirPath()
{
    WCHAR buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    QString appPath = QString::fromWCharArray(buffer);

    return appPath.left(appPath.lastIndexOf('\\'));
}
#endif // defined(FLAG_PORTABLE_MODE_SUPPORT) && defined(Q_OS_WIN)

Settings::Settings()
    : QObject(qApp)
{
    QString configPath;

#if defined(FLAG_PORTABLE_MODE_SUPPORT) && defined(Q_OS_WIN)
    QString portableConfigDirPath = QDir(getApplicationDirPath()).absoluteFilePath("data");
    QFileInfo portableConfigDirInfo(portableConfigDirPath);
    if (portableConfigDirInfo.exists() && portableConfigDirInfo.isDir() && portableConfigDirInfo.isWritable()) {
        // we can use it.
        configPath = portableConfigDirPath;
    }
#endif // defined(FLAG_PORTABLE_MODE_SUPPORT) && defined(Q_OS_WIN)

    if (configPath.isEmpty()) {
        // %LOCALAPPDATA%\<APPNAME> under Windows, ~/.config/<APPNAME> under Linux.
        configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    }

    m_qsettings = new QSettings(QDir(configPath).absoluteFilePath("config.ini"), QSettings::IniFormat, this);
}

