/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX5_CATEGORYHELPER_H_
#define _KCM_FCITX5_CATEGORYHELPER_H_

#include <QPainter>
#include <QStyleOptionViewItem>

namespace fcitx {
namespace kcm {

void paintCategoryHeader(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index);

QSize categoryHeaderSizeHint();

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_CATEGORYHELPER_H_
