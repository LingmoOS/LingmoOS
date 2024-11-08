#ifndef PAGE3_H
#define PAGE3_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QLabel>
class Page3 : public QWidget
{
    Q_OBJECT
public:
    explicit Page3(QString strImgDir,QWidget *parent = nullptr);
QLabel * lab;
    void setImgDir(QString strImgDir); //传入图片文件夹路径  就会自动开始播放 50ms刷一帧
    void closeAE(); //关闭动画显示  停止刷新
    void startAE(int);
private:
    void stopAE();
protected:
    void paintEvent(QPaintEvent *event);
private:
    QTimer *m_pTimer = nullptr;
    QString m_strImgDir = "";
    QStringList m_strListImg;
    int m_nIndex = 0;
    int m_nTimerId = 0;
    int i=0;
};

#endif // PAGE3_H
