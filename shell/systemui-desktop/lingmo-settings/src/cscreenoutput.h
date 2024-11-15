#ifndef CSCREENOUTPUT_H
#define CSCREENOUTPUT_H

#include <QObject>
#include <QScreen>

class CScreenOutput : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY outputChanged)
    Q_PROPERTY(Rotation rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool primary READ primary WRITE setPrimary NOTIFY primaryChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    enum Rotation {
        None = 1,
        Left = 2,
        Inverted = 4,
        Right = 8,
    };
    Q_ENUM(Rotation);

    explicit CScreenOutput(QObject *parent = nullptr);

    int id() const;
    void setId(int id);

    QString name() const;
    void setName(QString name);

    QString hash() const;

    Rotation rotation() const;
    void setRotation(const Rotation &rotation);

    bool connected() const;
    void setConnected(bool connected);

    bool primary() const;
    void setPrimary(bool primary);

    bool enabled() const;
    void setEnabled(bool enabled);

    const QScreen *qscreen() const;

signals:
    void outputChanged();
    void rotationChanged();
    void primaryChanged();
    void enabledChanged();
    void connectedChanged();

private:
    int m_id;
    QString m_name;
    QString m_hash;
    Rotation m_rotation;
    bool m_connected;
    bool m_primary;
    bool m_enabled;

    QScreen *m_qscreen;
};

#endif // CSCREENOUTPUT_H
