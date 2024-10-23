#include "mainwindow.h"
#include "util.h"

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

    w.playFiles(urlsToLoad);

    return QApplication::exec();
}
