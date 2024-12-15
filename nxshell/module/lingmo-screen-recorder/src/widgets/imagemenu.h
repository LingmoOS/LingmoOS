// Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEMENU_H
#define IMAGEMENU_H

#include <DMenu>
#include <DIconButton>
#include <QButtonGroup>

DWIDGET_USE_NAMESPACE
class BorderProcessInterface;
class ImageMenu;
class ImageBorderHelper : public QObject
{
    Q_OBJECT
public:
    enum BorderType {       //边框类型
        Nothing = 0,
        External,           //外边框
        Prototype,          //样机
        Projection,         //投影
    };

    static ImageBorderHelper *instance();
    ImageMenu* getBorderMenu(const BorderType type, const QString title, QWidget *parent = nullptr);
    void setActionState(const BorderType type, const bool isChecked);
    int getBorderTypeDetail();
    void setBorderTypeDetail(const int typeDetail);

    QPixmap getPixmapAddBorder(QPixmap pix);

signals:
    void updateBorderState(bool hasBorderChecked);
public:
    QSize m_screenSize;
private:
    explicit ImageBorderHelper(QObject *parent = nullptr);
    ~ImageBorderHelper();
private:
    static ImageBorderHelper *m_imageBorderHelper;
    QMap<BorderType, ImageMenu*> m_allBorderMenu;
    BorderProcessInterface *m_borderhandle;
};




class ActionWidget: public DIconButton
{
    Q_OBJECT
public:
    explicit ActionWidget(QWidget *parent = nullptr);
    ~ActionWidget() override;

    void setPixmap(const QString rePath);
    void setActionState(const bool isChecked);
protected:


private:
    void paintEvent(QPaintEvent *event) override;
private:
    bool m_isCheck = false;
    QPixmap m_pixmap;
};



class ImageMenu : public DMenu
{
    Q_OBJECT
public:
    ImageMenu(ImageBorderHelper::BorderType type, QString title, QWidget *parent = nullptr);
    void AddAction();
    ImageBorderHelper::BorderType borderType() {return  m_borderType;}
    int getBorderTypeDetail();
    void setBorderTypeDetail(const int id);
public slots:
    void ActionChecked(QAbstractButton* button);
protected:
    void paintEvent(QPaintEvent *event) override;
private:

    QButtonGroup* m_actionGroup = nullptr;
    ImageBorderHelper::BorderType m_borderType;
};

#endif // IMAGEMENU_H
