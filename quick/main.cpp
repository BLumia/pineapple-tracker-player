#include "player.h"
#include "playlistmanager.h"
#include "util.h"

#include <QQmlContext>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QIcon>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale(), QLatin1String("pineapple-tracker-player"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        app.installTranslator(&translator);
    }

    app.setWindowIcon(QIcon(":/icons/dist/pineapple-tracker-player.svg"));

    QCommandLineParser parser;
    parser.addPositionalArgument("File list", QCoreApplication::translate("main", "File list."));
    parser.addHelpOption();
    parser.process(app);

    QStringList urlStrList = parser.positionalArguments();
    QList<QUrl> urlsToLoad = Util::convertToUrlList(urlStrList);

    qmlRegisterUncreatableType<PlaylistModel>("Pineapple.TrackerPlayer", 1, 0, "PlaylistModel", "managed by PlaylistManager");
    qmlRegisterType<PlaylistManager>("Pineapple.TrackerPlayer", 1, 0, "PlaylistManager");
    qmlRegisterType<Player>("Pineapple.TrackerPlayer", 1, 0, "TrackerPlayer");

    QQmlApplicationEngine engine;

    QQmlContext * ctx = engine.rootContext();
    ctx->setContextProperty("fileList", QVariant::fromValue(urlsToLoad));
    ctx->setContextProperty("monoFontFamily", Util::defaultMonoFontFamily());

    const QUrl url(u"qrc:/player/main.qml"_s);
    engine.load(url);

    return app.exec();
}
