#pragma once

#include <QFont>
#include <QList>
#include <QUrl>

class Util
{
public:
    static QList<QUrl> convertToUrlList(const QStringList & files);
    static QFont defaultMonoFont();
    static QString defaultMonoFontFamily();
};

