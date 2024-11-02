#ifndef INTERACTIVEQML_H
#define INTERACTIVEQML_H

#include <QObject>
#include "global/variable.h"
#include <QDebug>
class InteractiveQml : public QObject
{
    Q_OBJECT
public:
    explicit InteractiveQml(QObject *parent = nullptr);
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
    void setImageFormat(QString format)
    {
        m_imageFormat = format;
        Q_EMIT imageFormatChanged(m_imageFormat);
    }
    static InteractiveQml *getInstance();

    void setPainterType(int type)
    {
        m_painterType = type;
        Q_EMIT painterTypeChanged(m_painterType);
    }
    void setPainterColor(QColor color)
    {
        m_painterColor = color;
        Q_EMIT painterColorChanged(color);
    }
    void setPainterThickness(int thickness)
    {
        m_painterThickness = thickness;
        Q_EMIT painterThicknessChanged(m_painterThickness);
    }
    void setPainterTextBold(bool isBold)
    {
        m_isBold = isBold;
        Q_EMIT textBoldChanged(m_isBold);
    }
    void setPainterTextDeleteLine(bool isDeleteLine)
    {
        m_isDeleteLine = isDeleteLine;
        Q_EMIT textDeleteLineChanged(m_isDeleteLine);
    }
    void setPainterTextUnderLine(bool isUnderLine)
    {
        m_isUnderLine = isUnderLine;
        Q_EMIT textUnderLineChanged(m_isUnderLine);
    }
    void setPainterTextItalics(bool isItalics)
    {
        m_isItalics = isItalics;
        Q_EMIT textItalicsChanged(m_isItalics);
    }
    void setPainterTextFontType(QString isFontType)
    {
        m_fontType = isFontType;
        Q_EMIT textFontTypeChanged(m_fontType);
    }
    void setPainterTextFontSize(int isFontSize)
    {
        m_fontSize = isFontSize;
        Q_EMIT textFontSizeChanged(m_fontSize);
    }

    //-story 19807
    void setImagePathId(QString currentPath)
    {
        m_currentPath = currentPath;
        Q_EMIT imagePathChanged(m_currentPath);
    }
    void setImageOrigScale(double origScale)
    {
        m_origScale = origScale;
        Q_EMIT imageOrigScaledChanged(m_origScale);
    }
    void setLoadImageList(QStringList loadImageList)
    {
        m_loadImageList = loadImageList;
        Q_EMIT updateLoadImageList(m_loadImageList);
    }
    void setCurrentImageIndex(int currentImageIndex)
    {
        m_currentImageIndex = currentImageIndex;
        Q_EMIT updateCurrentImageIndex(m_currentImageIndex);
    }
    void setImageDelPath(QString imageDelPath)
    {
        m_imageDelPath = imageDelPath;
        Q_EMIT updateImageDelPath(m_imageDelPath);
    }
    void setShowImageIndex(QList<int> showImageIndex)
    {
        m_showImageIndex = showImageIndex;
        QVariant tempList;
        tempList.setValue<QList<int>>(m_showImageIndex);
        Q_EMIT updateShowImageIndex(tempList);
    }
    void setImageTypeMovieOrNormal(QList<int> imageTypeMOvieOrNormal)
    {
        m_imageTypeMovieOrNormal = imageTypeMOvieOrNormal;
        QVariant tempList;
        tempList.setValue<QList<int>>(m_imageTypeMovieOrNormal);
        Q_EMIT updateImageTypeMovieOrNormal(tempList);
    }
    void setImageList(QStringList imageList, QList<int> imageTypeList, bool nullImage, int tiffOperate)
    {
        m_imagelist = imageList;
        m_imageTypelist = imageTypeList;
        m_imageIsNull = nullImage;
        m_tiffOperate = tiffOperate;
        QVariant tempList;
        tempList.setValue<QList<int>>(m_imageTypelist);
        Q_EMIT updateImageList(m_imagelist, tempList, m_imageIsNull, m_tiffOperate);
    }
    void setImageNum(int imageNum)
    {
        m_imageNum = imageNum;
        Q_EMIT updateImageNum(m_imageNum);
    }
    //-story 19807

    int getPainterType()
    {
        return m_painterType;
    }
    QColor getPainterColor()
    {
        return m_painterColor;
    }
    int getPainterThickness()
    {
        return m_painterThickness;
    }
    bool getPainterTextBold()
    {
        return m_isBold;
    }
    bool getPainterTextDeleteLine()
    {
        return m_isDeleteLine;
    }
    bool getPainterTextUnderLine()
    {
        return m_isUnderLine;
    }
    bool getPainterTextItalics()
    {
        return m_isItalics;
    }
    QString getPainterTextFontType()
    {
        return m_fontType;
    }
    int getPainterTextFontSize()
    {
        return m_fontSize;
    }

    void setThemeIcon(QString themeIcon)
    {
        m_themeIcon = themeIcon;
        Q_EMIT themeIconChanged(m_themeIcon);
    }

private:
    int m_operateWay = 0;
    QString m_imageFormat = "";
    int m_painterType = -1;                    //记录画笔类型
    int m_painterThickness = 12;               //记录画笔粗细
    QColor m_painterColor = QColor(255, 0, 0); //记录画笔颜色
    bool m_isBold = false;                     //加粗
    bool m_isDeleteLine = false;               //删除
    bool m_isUnderLine = false;                //下划线
    bool m_isItalics = false;                  //斜体
    QString m_fontType = "";                   //字体
    int m_fontSize = 12;                       //字号
    static InteractiveQml *m_interactionQml;   //单例指针

    //-story 19807
    QString m_currentPath = ""; //当前图片
    double m_origScale = 1;     //自适应-初始的缩放比

    QStringList m_loadImageList; //每次需要加载的图片
                                 //    int m_imageTotalNum = 0;     //图片总数量
    int m_currentImageIndex = 0; //当前图片的位置
    QString m_imageDelPath = ""; //需要删掉的图片路径
    QList<int> m_showImageIndex; //记录当前展示的5个item的所有index，用来加载图片用，与loadImageList是一一对应的
    QList<int> m_imageTypeMovieOrNormal; //记录图片是否是动静图
    QStringList m_imagelist;             //所有图片的list
    QList<int> m_imageTypelist;          //所有图片的list
    int m_imageNum = 0;                  //每次图片增加的数目
    bool m_imageIsNull = false;
    bool m_tiffOperate = false;
    //-story 19807

    QString m_themeIcon = "";

Q_SIGNALS:

    void operateWayChanged(QVariant);
    void imageFormatChanged(QVariant);
    // sign
    void painterTypeChanged(QVariant);
    void painterThicknessChanged(QVariant);
    void painterColorChanged(QVariant);
    void textBoldChanged(QVariant);
    void textDeleteLineChanged(QVariant);
    void textUnderLineChanged(QVariant);
    void textItalicsChanged(QVariant);
    void textFontTypeChanged(QVariant);
    void textFontSizeChanged(QVariant);

    //-story 19807
    void imagePathChanged(QVariant);
    void imageOrigScaledChanged(QVariant);

    void updateLoadImageList(QVariant);
    //    void updateImageTotalNum(QVariant);
    void updateCurrentImageIndex(QVariant);
    void updateImageDelPath(QVariant);
    void updateShowImageIndex(QVariant);
    void updateImageTypeMovieOrNormal(QVariant);
    void updateImageList(QVariant, QVariant, QVariant, QVariant);
    void updateImageNum(QVariant);
    //-story 19807

    void themeIconChanged(QVariant);
};

#endif // INTERACTIVE_H
