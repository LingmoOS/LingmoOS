// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CCUTWIDGET_H
#define CCUTWIDGET_H

#include "widgets/seperatorline.h"
#include "globaldefine.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"

#include <DWidget>
#include <DLineEdit>
#include <QAction>

DWIDGET_USE_NAMESPACE

class QButtonGroup;
class CCutWidget : public DrawAttribution::CAttriBaseOverallWgt
{
    Q_OBJECT
public:
    explicit CCutWidget(DWidget *parent = nullptr);
    ~CCutWidget();

    /**
     * @brief updateCutSize 更新裁剪尺寸
     */
    void setCutSize(const QSize &sz, bool emitSig = true);
    QSize cutSize();

    /**
     * @brief setCutType　设置裁剪比例的类型
     */
    void setCutType(ECutType current, bool emitSig = true, bool adjustSz = true);
    ECutType cutType();

    /**
     * @brief setCutType　设置裁剪比例的类型
     */
    //void  setDefualtRaidoBaseSize(const QSize &sz);


    void  setAutoCalSizeIfRadioChanged(bool b);

    int   totalNeedWidth() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
signals:
    void cutSizeChanged(const QSize &sz);
    void cutTypeChanged(ECutType tp);
    void finshed(bool accept);

public slots:
    /**
     * @brief changeButtonTheme 根据主题改变按钮主题
     */
    //void changeButtonTheme();

    /**
     * @brief clearAllChecked　清除所有按钮选中
     */
    void clearAllChecked();

private:
    void adjustSize(bool emitSig = true);

private:
    QPushButton *m_scaleBtn1_1;
    QPushButton *m_scaleBtn2_3;
    QPushButton *m_scaleBtn8_5;
    QPushButton *m_scaleBtn16_9;
    QPushButton *m_freeBtn;
    QPushButton *m_originalBtn;
    QButtonGroup *m_scaleBtnGroup = nullptr;

    bool          m_autoCal = true;
    const qreal Radio[cut_count] {1.0, 2.0 / 3.0, 8.0 / 5.0, 16.0 / 9.0, -1, -2};

    ECutType m_curCutType;
    QSize m_cutCutSize;

    QSize m_defultRadioSize;

    DLineEdit *m_widthEdit;
    DLineEdit *m_heightEdit;

    QPushButton *m_doneBtn;
    QPushButton *m_cancelBtn;
    SeperatorLine *m_sepLine;

    QAction *m_SizeAddAction;
    QAction *m_SizeReduceAction;

    QWidget     *m_sizeWidget;
    QHBoxLayout *m_sizeLayout;

private:
    /**
     * @brief initUI 初始化ＵＩ
     */
    void initUI();
    /**
     * @brief initConnection 初始化连接
     */
    void initConnection();
//    /**
//     * @brief activeFreeMode 激活自由按钮
//     */
//    void activeFreeMode();
};

#endif // CCUTWIDGET_H
