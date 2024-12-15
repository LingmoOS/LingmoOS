// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include "DocSheet.h"
#include "BaseWidget.h"

#include <DPushButton>
#include <DIconButton>

#include <QPointer>
#include <QKeyEvent>

class ScaleWidget;
class DocSheet;

/**
 * @brief The TitleWidget class
 * 窗体自定义标题栏
 */
class TitleWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(TitleWidget)

public:
    explicit TitleWidget(DWidget *parent = nullptr);

    ~TitleWidget() override;

public:
    /**
     * @brief setControlEnabled
     * 设置控件知否可用单元
     * @param enable
     */
    void setControlEnabled(const bool &enable);

public slots:
    /**
     * @brief onCurSheetChanged
     * 文档窗口切换单元
     */
    void onCurSheetChanged(DocSheet *);

    /**
     * @brief onFindOperation
     * 搜索列表控制单元
     */
    void onFindOperation(const int &);

    /**
     * @brief onThumbnailBtnClicked
     * 缩略图列表显隐按钮
     * @param checked
     */
    void onThumbnailBtnClicked(bool checked);

private:
    /**
     * @brief setBtnDisable
     * 设置按钮是否可用
     * @param bAble
     */
    void setBtnDisable(const bool &bAble);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QStringList shortKeyList;

    ScaleWidget *m_pSw = nullptr;
    DIconButton *m_pThumbnailBtn = nullptr;
    QPointer<DocSheet> m_curSheet = nullptr;
};

#endif  // TITLEWIDGET_H
