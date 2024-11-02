#ifndef INFORMATION_H
#define INFORMATION_H
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QFont>
#include <QLocale>

class Information : public QWidget
{
    Q_OBJECT
public:
    explicit Information(QWidget *parent = nullptr);
    void contentText(QFileInfo info, QString sizeImage, QString spaceColor, QString realFormat);
    void updateName(QFileInfo newFile); //重命名后更新名字和修改时间
    //控件字体变化，窗口大小变化
    void changeEverySize(double fontSize, double detio, QSize sizeWid);

private:
    QLabel *m_widName;      //窗口名--信息
    QLabel *m_name;         //名称
    QLabel *m_format;       //格式
    QLabel *m_storageSize;  //存储大小
    QLabel *m_pixelSize;    //像素尺寸
    QLabel *m_colorSpace;   //颜色空间
                            //    QLabel *m_creationTime; //创建时间
    QLabel *m_revisionTime; //修改时间

    QLabel *m_nameC;         //名称内容
    QLabel *m_formatC;       //格式内容
    QLabel *m_storageSizeC;  //存储大小内容
    QLabel *m_pixelSizeC;    //像素尺寸内容
    QLabel *m_colorSpaceC;   //颜色空间内容
                             //    QLabel *m_creationTimeC; //创建时间内容
    QLabel *m_revisionTimeC; //修改时间内容

    QWidget *m_inforWid;
    QGridLayout *m_gdLayout;

    QFont m_ft;        //标题字号字重设置
    QFont m_ftContent; //内容字号固定
    QLocale m_local;

    int m_wid; //图片名字长度
    bool m_linenum = false;
    bool m_lineInTwo = false; //判断大于1行小于2行

    void layout();
    void layoutS(int line);

    QString AutoFeed(QString text);


Q_SIGNALS:
};

#endif // INFORMATION_H
