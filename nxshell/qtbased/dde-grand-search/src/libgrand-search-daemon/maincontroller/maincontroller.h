// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>

namespace GrandSearch {

class MainControllerPrivate;
class MainController : public QObject
{
    Q_OBJECT
    friend class MainControllerPrivate;
public:
    explicit MainController(QObject *parent = nullptr);
    bool init();

    bool newSearch(const QString &key);
    void terminate();
    QByteArray getResults() const;
    QByteArray readBuffer() const;
    bool isEmptyBuffer() const;
    bool searcherAction(const QString &name, const QString &action, const QString &item);
signals:
    void matched();
    void searchCompleted();
public slots:
private:
    MainControllerPrivate *d = nullptr;
};

}

#endif // MAINCONTROLLER_H
