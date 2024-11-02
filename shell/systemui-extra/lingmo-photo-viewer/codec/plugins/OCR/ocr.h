#ifndef OCR_H
#define OCR_H

#include <QObject>
#include <QVector>
#include <QString>

enum EngineType { KySDK = 0, Others = 1 };

class OCR : public QObject
{
    Q_OBJECT
public:
    OCR();
    ~OCR();
    void setEngineType(EngineType = EngineType::KySDK);
    virtual void getText(QString);
    //接口暂未启用,先注释掉
    //    virtual void getRect(QString);
Q_SIGNALS:
    void recResult(QString, QVector<QString>);
    void dectResult(QString, QVector<QVector<QVector<int>>>);

private:
    OCR *m_ocr = nullptr;
};

#endif // OCR_H
