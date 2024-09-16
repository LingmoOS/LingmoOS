// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WMCHOOSER_H
#define WMCHOOSER_H

#include <QWidget>
#include <QPushButton>

class WMChooser : public QWidget
{
    Q_OBJECT

public:
    explicit WMChooser(QWidget *parent = nullptr);

    void setConfigPath(const QString &path);
    void createParentDir(const QString &fullfilepath);

signals:
    void screenChanged();

private slots:
    void saveConfig();

protected:
    void leaveEvent(QEvent *event) override;

private:
    QPushButton *m_gorgeousBtn;
    QPushButton *m_topSpeedBtn;
    QString m_configPath;
};

#endif // WMCHOOSER_H
