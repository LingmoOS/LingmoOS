/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */

#ifndef CREATEINDEXASKDIALOG_H
#define CREATEINDEXASKDIALOG_H

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QStyleOption>
#include <QApplication>
#include <QPainter>

namespace LingmoUISearch {
class CreateIndexAskDialog : public QDialog {
    Q_OBJECT
public:
    CreateIndexAskDialog(QWidget *parent = nullptr);
    ~CreateIndexAskDialog() = default;

private:
    void initUi();

    QVBoxLayout * m_mainLyt = nullptr;
    //标题栏
    QFrame * m_titleFrame = nullptr;
    QHBoxLayout * m_titleLyt = nullptr;
    QLabel * m_iconLabel = nullptr;
    QLabel * m_titleLabel = nullptr;
    QPushButton * m_closeBtn = nullptr;

    //内容区域
    QFrame * m_contentFrame = nullptr;
    QVBoxLayout * m_contentLyt = nullptr;
    QLabel * m_tipLabel = nullptr; //提示语
    QFrame * m_checkFrame = nullptr; //"不再提示"选项框区域
    QHBoxLayout * m_checkLyt = nullptr;
    QCheckBox * m_checkBox = nullptr;
    QFrame * m_btnFrame = nullptr; //底部按钮区域
    QHBoxLayout * m_btnLyt = nullptr;
    QPushButton * m_cancelBtn = nullptr;
    QPushButton * m_confirmBtn = nullptr;

    void paintEvent(QPaintEvent *);

protected:
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void closed();
    void btnClicked(const bool&, const bool&);
    void focusChanged();

};
}

#endif // CREATEINDEXASKDIALOG_H
