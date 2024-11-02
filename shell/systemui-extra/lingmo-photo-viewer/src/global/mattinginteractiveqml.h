#ifndef MATTINGINTERACTIVEQML_H
#define MATTINGINTERACTIVEQML_H

#include <QObject>
#include "global/variable.h"
#include <QDebug>

class MattingInteractiveQml: public QObject
{
    Q_OBJECT
public:
    explicit MattingInteractiveQml(QObject *parent = nullptr);
    Q_PROPERTY(int operateWay READ getOperateWay WRITE setOperateWay NOTIFY operateWayChanged)
    int getOperateWay()
    {
        return m_operateWay;
    }
    void setOperateWay(int way)
    {
        m_operateWay = way;
        Q_EMIT operateWayChanged(m_operateWay);
    }

    static MattingInteractiveQml *getInstance();

private:
    int m_operateWay = 0;
    static MattingInteractiveQml *m_mattingInteractionQml; //单例指针

Q_SIGNALS:
    void operateWayChanged(QVariant);
};

#endif // MATTINGINTERACTIVEQML_H
