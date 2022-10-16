#include "player.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Player>("Pineapple.TrackerPlayer", 1, 0, "TrackerPlayer");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/player/main.qml"_qs);
    engine.load(url);

    return app.exec();
}
