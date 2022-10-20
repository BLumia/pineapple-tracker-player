#include "util.h"

#include <QUrl>
#include <QFile>
#include <QFontDatabase>

QList<QUrl> Util::convertToUrlList(const QStringList &files)
{
    QList<QUrl> urlList;
    for (const QString & str : qAsConst(files)) {
        if (!QFile::exists(str)) continue;
        QUrl url = QUrl::fromLocalFile(str);
        if (url.isValid()) {
            urlList.append(url);
        }
    }

    return urlList;
}

QFont Util::defaultMonoFont()
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

QString Util::defaultMonoFontFamily()
{
    const QFont && fixedFont = defaultMonoFont();
    return fixedFont.family();
}
