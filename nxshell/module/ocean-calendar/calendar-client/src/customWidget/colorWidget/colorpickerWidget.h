// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COLORPICKERWIDGET_H
#define COLORPICKERWIDGET_H

#include "colorlabel.h"
#include "colorslider.h"

#include <DAbstractDialog>
#include <DLineEdit>
#include <DPushButton>
#include <QLabel>

DWIDGET_USE_NAMESPACE
class CColorPickerWidget : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit CColorPickerWidget(QWidget *parent = nullptr);
    ~CColorPickerWidget();

    QColor getSelectedColor();

private:
    void initUI();
    void setColorHexLineEdit();
    void setLabelText();

public slots:

    /**
     * @brief slotUpdateColor　画板颜色更新事件
     */
    void slotUpdateColor(const QColor &color = QColor());
    /**
     * @brief slotHexLineEditChange
     * 颜色输入框文本更改事件
     * @param text
     */
    void slotHexLineEditChange(const QString &text);

    void slotCancelBtnClicked();
    void slotEnterBtnClicked();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *e) override;
private:
    ColorLabel *m_colorLabel;
    ColorSlider *m_colorSlider;

    DLineEdit *m_colHexLineEdit;
    DLabel *m_wordLabel;
    DPushButton *m_cancelBtn;
    DPushButton *m_enterBtn;
    QString     m_strColorLabel;
    QColor curColor;
};

#endif // COLORPICKERWIDGET_H
