#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QTimer>
#include <QSet>

#include <xcb/xcb.h>

class Hotkeys : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit Hotkeys(QObject *parent = nullptr);
    ~Hotkeys();

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    void registerKey(QKeySequence keySequence);
    void registerKey(quint32 keycode);

    void registerKey(quint32 key, quint32 mods);
    void unregisterKey(quint32 key, quint32 mods);


signals:
    void pressed(QKeySequence keySeq);
    void released(QKeySequence keySeq);

private:
    quint32 nativeKeycode(Qt::Key k);
    quint32 nativeModifiers(Qt::KeyboardModifiers m);
    Qt::Key getKey(const QKeySequence& keyseq);
    Qt::KeyboardModifiers getMods(const QKeySequence& keyseq);

private:
    QHash<quint32, QKeySequence> m_shortcuts;
};

#endif // HOTKEYS_H