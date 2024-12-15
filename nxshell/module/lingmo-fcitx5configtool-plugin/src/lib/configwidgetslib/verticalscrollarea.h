/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX_VERTICALSCROLLAREA_H_
#define _KCM_FCITX_VERTICALSCROLLAREA_H_

#include <QScrollArea>
namespace fcitx {
namespace kcm {
class VerticalScrollArea : public QScrollArea {
    Q_OBJECT
public:
    explicit VerticalScrollArea(QWidget *parent = 0);
    void setWidget(QWidget *widget);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_VERTICALSCROLLAREA_H_
