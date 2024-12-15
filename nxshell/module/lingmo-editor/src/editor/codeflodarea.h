// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEFLODAREA_H
#define CODEFLODAREA_H
#include <QWidget>
class LeftAreaTextEdit;


class CodeFlodArea: public QWidget
{
    Q_OBJECT
public:
    explicit CodeFlodArea(LeftAreaTextEdit *leftAreaWidget);
    ~CodeFlodArea() override;
    void paintEvent(QPaintEvent *e) override;

private:
    LeftAreaTextEdit* m_pLeftAreaWidget = nullptr;
};

#endif // CODEFLODAREA_H
