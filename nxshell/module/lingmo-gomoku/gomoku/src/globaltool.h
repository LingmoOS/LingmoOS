// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALTOOL_H
#define GLOBALTOOL_H

#include <QPixmap>
#include <QWidget>
#include <QIcon>
#include <QApplication>
#include <QImageReader>

/**
 * @brief The ProgramFontSize struct 五子棋中各中页面下的语句字体，为适配各种语言
 */
struct GomokuFontManagement {
    int functionButton; //功能按钮字体字号
    int welcomeText; //欢迎语句字体字号
    int chessStatement; //下棋提示语句字体字号
    int gameOverText; //游戏结束字体字号
    int dialogLabel; //弹窗标签字体字号
    int dialogButton; //弹窗按钮字体字号
    int dialogOffset; //弹窗上的偏移量，藏语情况下三个弹窗的标签大小和按钮大小各自都不相同，用一个偏移量计算
};

class Globaltool
{

public:
    /**
     * @brief instacne 单例
     * @return
     */
    static Globaltool *instacne();

    /**
     * @brief getFontSize
     * @return
     */
    const GomokuFontManagement &getFontSize() {return this->gomokuFontManagement;}

    //去除图片锯齿, 可以拉伸大小, 可留做全局功能函数
    /**
     * @brief getDpiPixmap 去除图片锯齿,
     * @param size 需要的图片大小
     * @param filename 图片路径
     * @param w 父窗口
     * @return 图片
     */
    QPixmap getDpiPixmap(QSize size, const QString &filename, QWidget *w);

    /**
     * @brief AutoFeed 对字符串进行手动换行
     * @param text 字符串
     * @param fontSize 字体大小
     * @param textWidth 容纳字符的最大宽度
     * @return
     */
    QString AutoFeed(const QString &text, const int fontSize, const int textWidth);

    /**
     * @brief loadFontFamilyFromFiles 输入字体文件路径，输出字体family
     * @param fontFileName 字体文件路径
     * @return 字体family
     */
    QString loadFontFamilyFromFiles(const QString &fontFileName);

    /**
     * @brief loadSystemLanguage加载系统语言
     */
    void loadSystemLanguage();

private:
    Globaltool();
    ~Globaltool();

    GomokuFontManagement gomokuFontManagement = {functionButton : 22, welcomeText : 30
                            , chessStatement : 24, gameOverText : 24
                            , dialogLabel : 25, dialogButton : 20, dialogOffset : 0};
};

#endif // GLOBALTOOL_H
