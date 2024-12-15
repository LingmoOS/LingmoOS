// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXHIBITIONWIDGET_H
#define EXHIBITIONWIDGET_H

#include "global/matcheditem.h"

#include <DWidget>

#include <QScopedPointer>

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;

namespace GrandSearch {

class ExhibitionWidgetPrivate;
class MatchWidget;
class PreviewWidget;

class ExhibitionWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit ExhibitionWidget(QWidget *parent = nullptr);
    ~ExhibitionWidget() override;

    void clearData();

public slots:
    void onSelectNextItem();
    void onSelectPreviousItem();
    void onHandleItem();

    void appendMatchedData(const MatchedItemMap &matchedData);//追加显示匹配数据
    void onSearchCompleted();
    void previewItem(const QString &searchGroupName, const MatchedItem &item);

signals:
    void sigCurrentItemChanged(const QString &searchGroupName, const MatchedItem &item);
    void sigShowNoContent(bool noContent);
    void sigCloseWindow();
    void sigPreviewStateChanged(bool priview);

protected:
    void initUi();
    void initConnect();

    void paintEvent(QPaintEvent *event) override;

private:
    QScopedPointer<ExhibitionWidgetPrivate> d_p;

    QHBoxLayout *m_hLayout = nullptr;

    QVBoxLayout *m_vLayout = nullptr;
    QPushButton *m_btnSearch = nullptr;
    QPushButton *m_btnClear = nullptr;
    MatchWidget *m_matchWidget = nullptr;
    PreviewWidget *m_previewWidget = nullptr;
};

}

#endif // EXHIBITIONWIDGET_H
