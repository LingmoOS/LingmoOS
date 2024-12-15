// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALKBINSTANCE_H
#define VIRTUALKBINSTANCE_H

#include <QObject>

class QProcess;
class VirtualKBInstance : public QObject
{
    Q_OBJECT
public:
    static VirtualKBInstance &Instance();
    ~VirtualKBInstance() override;

    void init();
    void stopVirtualKBProcess();

public slots:
    void showKeyboardWidget(QWidget *parent);
    void hideKeyboardWidget();
    void onVirtualKBProcessFinished(int exitCode);

private:
    explicit VirtualKBInstance(QObject *parent = nullptr);
    VirtualKBInstance(const VirtualKBInstance &) = delete;

private:
    QWidget *m_parentWidget;
    QWidget *m_virtualKBWidget;
    QProcess *m_virtualKBProcess;
};

#endif // VIRTUALKBINSTANCE_H
