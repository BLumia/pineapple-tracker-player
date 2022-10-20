#include "mainwindow.h"
#include "util.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument("File list", QCoreApplication::translate("main", "File list."));
    parser.addHelpOption();
    parser.process(app);

    QStringList urlStrList = parser.positionalArguments();
    QList<QUrl> urlsToLoad = Util::convertToUrlList(urlStrList);

    MainWindow w;
    w.show();

    w.playFiles(urlsToLoad);

    return app.exec();
}
