// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "mainwindow.h"
#include "util.h"

#ifdef Q_OS_MACOS
#include "fileopeneventhandler.h"
#endif // Q_OS_MACOS

#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale(), QLatin1String("pineapple-tracker-player"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        QApplication::installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.addPositionalArgument("File list", QCoreApplication::translate("main", "File list."));
    parser.addHelpOption();
    parser.process(app);

    const QStringList urlStrList = parser.positionalArguments();
    const QList<QUrl> urlsToLoad = Util::convertToUrlList(urlStrList);

    MainWindow w;
    w.show();

#ifdef Q_OS_MACOS
    FileOpenEventHandler * fileOpenEventHandler = new FileOpenEventHandler(&app);
    app.installEventFilter(fileOpenEventHandler);
    app.connect(fileOpenEventHandler, &FileOpenEventHandler::fileOpen, [&w](const QUrl & url){
        if (w.isHidden()) {
            w.setWindowOpacity(1);
            w.showNormal();
        } else {
            w.activateWindow();
        }
        w.playFiles({url});
    });
#endif // Q_OS_MACOS

    if (!urlsToLoad.isEmpty()) {
        w.playFiles(urlsToLoad);
    }

    return QApplication::exec();
}
