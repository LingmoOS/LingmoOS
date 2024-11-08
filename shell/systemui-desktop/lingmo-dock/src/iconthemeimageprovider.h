#ifndef ICONTHEMEIMAGEPROVIDER_H
#define ICONTHEMEIMAGEPROVIDER_H

#include <QtQuick/QQuickImageProvider>

class IconThemeImageProvider : public QQuickImageProvider
{
public:
    IconThemeImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *realSize, const QSize &requestedSize);
};

#endif // ICONTHEMEIMAGEPROVIDER_H
