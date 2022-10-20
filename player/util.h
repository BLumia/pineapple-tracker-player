#pragma once

#include <QFont>
#include <QList>

class Util
{
public:
    static QList<QUrl> convertToUrlList(const QStringList & files);
    static QFont defaultMonoFont();
    static QString defaultMonoFontFamily();
};

