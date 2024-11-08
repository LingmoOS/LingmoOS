#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QObject>
#include <QFileInfo>

class FileHelper : public QObject
{
    Q_OBJECT

public:
    explicit FileHelper(QObject *parent = nullptr);

    Q_INVOKABLE void addPath(const QString &path);

signals:
    void newPath(const QString &path);
    void unavailable(const QString &path);
};

#endif // TEXTEDITOR_H
