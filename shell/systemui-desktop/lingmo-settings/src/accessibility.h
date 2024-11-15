#ifndef ACCESSIBILITY_H
#define ACCESSIBILITY_H

#include <QObject>
#include <QDBusInterface>
#include <QStandardPaths>
#include <QSettings>
class Accessibility : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool wobblyWindows READ wobblyWindows WRITE setWobblywindows NOTIFY wobblyWindowsChanged)
    Q_PROPERTY(bool thumbnailaside READ thumbnailaside WRITE setThumbnailaside NOTIFY thumbnailasideChanged)
    Q_PROPERTY(bool touchpoints READ touchpoints WRITE setTouchpoints NOTIFY touchpointsChanged)
    Q_PROPERTY(bool snaphelper READ snaphelper WRITE setSnaphelper NOTIFY snaphelperChanged)
    Q_PROPERTY(bool diminactive READ diminactive WRITE setDiminactive NOTIFY diminactiveChanged)


public:
    explicit Accessibility(QObject *parent = nullptr);
    bool wobblyWindows() const;
    void setWobblywindows(bool enabled);
    bool thumbnailaside() const;
    void setThumbnailaside(bool enabled);
    bool touchpoints() const;
    void setTouchpoints(bool enabled);
    bool snaphelper() const;
    void setSnaphelper(bool enabled);
    bool diminactive() const;
    void setDiminactive(bool enabled);
signals:
    void wobblyWindowsChanged();
    void thumbnailasideChanged();
    void touchpointsChanged();
    void snaphelperChanged();
    void diminactiveChanged();

private slots:

private:
    QSettings *m_kwinSettings;
    bool m_wobblyWindows;
    bool m_thumbnailaside;
    bool m_touchpoints;
    bool m_snaphelper;
    bool m_diminactive;

};

#endif // ACCESSIBILITY_H