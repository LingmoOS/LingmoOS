#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QObject>
#include <QDBusInterface>

class Wallpaper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(bool dimsWallpaper READ dimsWallpaper NOTIFY dimsWallpaperChanged)
    Q_PROPERTY(QString color READ color NOTIFY colorChanged)

public:
    explicit Wallpaper(QObject *parent = nullptr);

    int type() const;

    QString path() const;
    bool dimsWallpaper() const;

    QString color() const;

signals:
    void pathChanged();
    void dimsWallpaperChanged();
    void typeChanged();
    void colorChanged();

private slots:
    void onPathChanged(QString path);

private:
    QDBusInterface m_interface;
    QString m_wallpaper;
};

#endif // WALLPAPER_H
