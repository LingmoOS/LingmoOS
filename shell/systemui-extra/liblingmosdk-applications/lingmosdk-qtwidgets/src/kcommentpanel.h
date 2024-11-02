/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KCOMMENTPANEL_H
#define KCOMMENTPANEL_H

#include <QWidget>
#include "gui_g.h"

namespace kdk
{

/**
 * @defgroup CommentsPanel
 */
class KCommentPanelPrivate;

/**
 * @brief 提供六种评论等级
 */
enum StarLevel
{
    LevelZero = 0,
    LevelOne,
    LevelTwo,
    LevelThree,
    LevelFour,
    LevelFive
};

/**
 * @brief 显示用户评论的详细信息
 */
class GUI_EXPORT KCommentPanel : public QWidget
{
    Q_OBJECT

public:
    explicit KCommentPanel(QWidget *parent = nullptr);

    /**
     * @brief 设置评论用户头像
     */
    void setIcon(const QIcon&);

    /**
     * @brief 设置评论时间
     */
    void setTime(const QString&);

    /**
     * @brief 设置评论用户昵称
     */
    void setName(const QString&);

    /**
     * @brief 设置评论文本
     */
    void setText(const QString&);

    /**
     * @brief 设置等级
     * @param level
     */
    void setGrade(StarLevel level);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Q_DECLARE_PRIVATE(KCommentPanel)
    KCommentPanelPrivate * const d_ptr;
};
}

#endif // KCOMMENTPANEL_H
