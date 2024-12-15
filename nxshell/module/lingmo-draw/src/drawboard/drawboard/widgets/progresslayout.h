// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSLAYOUT_H
#define PROGRESSLAYOUT_H
#include "adaptivedef.h"
#include <QHBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QWidget>

#ifdef USE_DTK
#define FATHERWGT DBlurEffectWidget
#include <DBlurEffectWidget>
#include <DLabel>
DWIDGET_USE_NAMESPACE
#else
#define FATHERWGT QWidget
#endif
class ProgressLayout: public FATHERWGT

{
    Q_OBJECT
public:
    explicit ProgressLayout(QWidget *parent = nullptr);
    ~ProgressLayout();
    void setRange(int start, int end);
    void setProgressValue(int value);

    void showInCenter(QWidget *w);

    void setText(const QString &str);
    void delayClose();

    void reset();

    void setAutoFillSubText(bool b);
    bool isAutoFillSubText() const;
    void setSubText(const QString &str);


    QVBoxLayout *m_progressVBoxLayout;
    DLabel *m_label;
    DLabel *m_progressLabel;
    QProgressBar *m_progressbar;
    int m_start;
    int m_end;
    QString     m_subText;
    bool        m_autoFillSubText = true;

};

#endif // PROGRESSLAYOUT_H
