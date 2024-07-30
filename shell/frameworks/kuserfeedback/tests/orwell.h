/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef ORWELL_H
#define ORWELL_H

#include <QMainWindow>

#include <memory>

namespace Ui { class Orwell; }

class Orwell : public QMainWindow
{
public:
    explicit Orwell(QWidget *parent = nullptr);
    ~Orwell() override;

private:
    void loadStats();
    void writeStats();

    std::unique_ptr<Ui::Orwell> ui;
};

#endif
