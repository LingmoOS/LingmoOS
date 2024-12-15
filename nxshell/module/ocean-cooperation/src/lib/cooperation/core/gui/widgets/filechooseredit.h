// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECHOOSEREDIT_H
#define FILECHOOSEREDIT_H

#include "global_defines.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPainter>

namespace cooperation_core {

class FileChooserEdit : public QWidget
{
    Q_OBJECT
public:
    explicit FileChooserEdit(QWidget *parent = nullptr);

    void setText(const QString &text);

Q_SIGNALS:
    void fileChoosed(const QString &fileName);

private Q_SLOTS:
    void onButtonClicked();
    void updateSizeMode();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();

#ifdef linux
    CooperationLineEdit *pathLabel { nullptr };
#else
    QLabel *pathLabel { nullptr };
#endif
    CooperationSuggestButton *fileChooserBtn { nullptr };
};

class FileChooserBtn : public QPushButton {
public:
    FileChooserBtn(QWidget *parent = nullptr) : QPushButton(parent) {}

protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        // 获取按钮的矩形区域
        QRect rect = this->rect();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::white);
        QPointF center(rect.center().x() + 1 , rect.center().y() + 1);

        painter.drawEllipse(QPointF(center.x(), center.y()), 1, 1);
        painter.drawEllipse(QPointF(center.x() - 5, center.y()), 1, 1);
        painter.drawEllipse(QPointF(center.x() + 5, center.y()), 1, 1);
    }
};

class InformationDialog : public CooperationDialog
{
    Q_OBJECT

public:
    explicit InformationDialog(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();

private:
    CooperationLabel *iconLabel { nullptr };
    CooperationLabel *msgLabel { nullptr };
    CooperationLabel *titleLabel { nullptr };
};
}   // namespace cooperation_core

#endif   // FILECHOOSEREDIT_H
