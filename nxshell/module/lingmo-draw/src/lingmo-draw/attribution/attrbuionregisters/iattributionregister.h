// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IATRRIBUTIONREGISTER_H
#define IATRRIBUTIONREGISTER_H
#include <QObject>

#include "drawboard.h"
#include "attributemanager.h"

class IAttributionRegister : public QObject
{
    Q_OBJECT
public:
    explicit IAttributionRegister(DrawBoard *drawBoard = nullptr, QObject *parent = nullptr):  QObject(parent), m_drawBoard(drawBoard) {}
    virtual void registe() = 0;

    void setDrawBoard(DrawBoard  *drawBoard)
    {
        m_drawBoard = drawBoard;
    }

    DrawBoard *drawBoard()
    {
        return m_drawBoard;
    }

private:
    DrawBoard     *m_drawBoard;
};

#endif // IATRRIBUTIONREGISTER_H
