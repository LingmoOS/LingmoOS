// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTRSCROLLWIDGET_H
#define ATTRSCROLLWIDGET_H

#include <DFrame>
#include <DWidget>

class DocSheet;
class QGridLayout;
/**
 * @brief The AttrScrollWidget class
 * 文件属性展示控件
 */
class AttrScrollWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(AttrScrollWidget)

public:
    explicit AttrScrollWidget(DocSheet *sheet, Dtk::Widget::DWidget *parent  = nullptr);

private:
    /**
     * @brief createLabel
     * 创建展示Label子控件
     * @param layout
     * @param index
     * @param objName
     * @param sData
     */
    void createLabel(QGridLayout *layout, const int &index, const QString &objName, const QString &sData);

    /**
     * @brief createLabel
     * 创建展示Label子控件
     * @param layout
     * @param index
     * @param objName
     * @param sData
     */
    void createLabel(QGridLayout *layout, const int &index, const QString &objName, const QDateTime &sData);

    /**
     * @brief createLabel
     * 创建展示Label子控件
     * @param layout
     * @param index
     * @param objName
     * @param bData
     */
    void createLabel(QGridLayout *layout, const int &index, const QString &objName, const bool &bData);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

    QList<QWidget *> m_leftWidgetlst;
    QList<QWidget *> m_rightWidgetlst;
};

#endif // ATTRSCROLLWIDGET_H
