#ifndef ALBUMTHUMBNAIL_H
#define ALBUMTHUMBNAIL_H

#include <QThread>
#include "global/variable.h"
#include "model/processing/processing.h"
#include "model/file/file.h"

class AlbumThumbnail : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void albumFinish(QVariant var);

public:
    AlbumThumbnail(const QString &path, const QString &realFormat);
    void run();

private:
    QString m_path;
    QString m_realFormat = "";
    QPixmap changImageSize(const QPixmap &pix);
};

class AlbumRealImg : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void albumRealFinish(QVariant var);
public:
    AlbumRealImg(const QString &path, const QString &realFormat);
    void run();
private:
    QString m_path;
    QString m_realFormat = "";
};

#endif // ALBUMTHUMBNAIL_H
