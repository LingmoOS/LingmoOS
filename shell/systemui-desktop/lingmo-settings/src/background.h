#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDirIterator>
#include <QDir>

class Background : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentBackgroundPath READ currentBackgroundPath WRITE setBackground NOTIFY backgroundChanged)
    Q_PROPERTY(QVariantList backgrounds READ backgrounds NOTIFY stub)

    Q_PROPERTY(int backgroundType READ backgroundType WRITE setBackgroundType NOTIFY backgroundTypeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    explicit Background(QObject *parent = nullptr);

    QVariantList backgrounds();
    QString currentBackgroundPath();
    Q_INVOKABLE void setBackground(QString newBackgroundPath);

    int backgroundType();
    void setBackgroundType(int type);

    QString backgroundColor();
    void setBackgroundColor(const QString &color);

signals:
    void backgroundChanged();
    void backgroundColorChanged();
    void backgroundTypeChanged();
    void stub();

private:
    QDBusInterface m_interface;
    QString m_currentPath;
};

#endif
