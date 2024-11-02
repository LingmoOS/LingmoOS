#ifndef SCANNERINTERACTIVEQML_H
#define SCANNERINTERACTIVEQML_H

#include <QObject>
#include <QDebug>
#include "global/variable.h"

class ScannerInteractiveQml : public QObject
{
    Q_OBJECT
public:
    explicit ScannerInteractiveQml(QObject *parent = nullptr);
    Q_PROPERTY(int operateWay READ getOperateWay WRITE setOperateWay NOTIFY operateWayChanged)

    static ScannerInteractiveQml *getInstance();    //获取单例
    int getOperateWay();                            //操作方式
    void setOperateWay(int way);

private:
    static ScannerInteractiveQml *m_interactionQml; //单例指针
    int m_operateWay = 0;           //操作方式

Q_SIGNALS:
    void operateWayChanged(QVariant);       //操作方式

};

#endif // SCANNERINTERACTIVEQML_H
