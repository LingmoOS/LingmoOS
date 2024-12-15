// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ICONDFRAME_H
#define ICONDFRAME_H

#include <DFrame>

class QSvgRenderer;
class Reply;
DWIDGET_USE_NAMESPACE
class IconDFrame : public DFrame
{
    Q_OBJECT
public:
    explicit IconDFrame(QWidget *parent = nullptr);
    ~IconDFrame() override;

    void setCenterLayout(QLayout *layout);

    QColor TitleColor() const;
    void setTitleColor(const QColor &TitleColor);

private:
    QFont TitleFont();

protected:
    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void setTheMe(const int type);
    void paintPixmap(QPainter &painter);
    void paintTitle(QPainter &painter);
signals:
    void signaleSendMessage(QString text);
    /**
     * @brief widgetIsHide      窗口隐藏信号
     */
    void widgetIsHide();
public slots:
    virtual void slotReceivce(QVariant data, Reply *reply);

private:
    QSvgRenderer *m_Dayrenderer;
    QSvgRenderer *m_Weekrenderer;
    QSvgRenderer *m_Monthrenderer;
    QSvgRenderer *m_backgroundrenderer;

    QColor m_TitleColor;
    QFont m_TitleFont;
    bool m_tabFocusIn {false}; //获取tab焦点标记
};

#endif // ICONDFRAME_H
