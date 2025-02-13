#ifndef X11SHADOW_H
#define X11SHADOW_H

#include <memory>

#include <QObject>

class X11Shadow : public QObject
{
    Q_OBJECT

public:
    explicit X11Shadow(QObject *parent = nullptr);

private:
    quint32 m_atom_net_wm_shadow;
    quint32 m_atom_net_wm_window_type;
};

#endif // X11SHADOW_H
