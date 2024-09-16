// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSELECTEDTEXTTOOLTIP_H
#define DSELECTEDTEXTTOOLTIP_H

#include "global.h"

#include <QRasterWindow>
#include <QVector>

DPP_BEGIN_NAMESPACE

class DSelectedTextTooltip : public QRasterWindow
{
    Q_OBJECT

public:
    enum OptionType {
        None = 0,
        Cut = 1, //剪切
        Copy = 2, //复制
        Paste = 3, //粘贴
        SelectAll = 4 //全选
    };

    DSelectedTextTooltip();
    ~DSelectedTextTooltip() override;

protected:
    void paintEvent(QPaintEvent *pe) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onFontChanged();

signals:
    void optAction(OptionType type);

private:
    void updateColor();
    OptionType getOptionType(const QPoint &pos) const;

    struct OptionTextInfo {
        OptionType optType;
        int textWidth;
        QString optName;
    };

    QVector<OptionTextInfo> m_textInfoVec;

    QColor m_backgroundColor;
    QColor m_dividerColor;
    QColor m_borderColor;

};

DPP_END_NAMESPACE

#endif // DSELECTEDTEXTTOOLTIP_H
