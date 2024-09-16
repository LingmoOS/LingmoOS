// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H
#include <QObject>

namespace QApt {
class Backend;
class Transaction;
}
class DebInstaller: public QObject
{
    Q_OBJECT
public:
    explicit DebInstaller(QObject *parent = nullptr);
    //判断当前DebInstaller对象是否生效，如果无效继续调用可能导致crash
    bool isValid();

    /**
     * @brief isArchMatched 判断架构是否匹配
     * @param path
     * @return
     */
    bool isArchMatched(const QString &path);

    /**
     * @brief isDebValid 判断包是否有效
     * @param path
     * @return
     */
    bool isDebValid(const QString &path);

private:
    void initBackend();
    void reset();
    void doOperate(const QString &package, bool binstall);


public slots:
    void installPackage(const QString &filepath);
    void uninstallPackage(const QString &packagename);

signals:
    void installFinished(bool bsuccess);
    void errorOccurred(const QString &errmsg);
    void progressChanged(int progress);

private:
    QApt::Backend *m_backend = nullptr;
    QApt::Transaction *m_pTrans = nullptr;
    bool m_bValid = false;
    int m_iRuningTestCount = 0;
};

#endif // DEBINSTALLER_H
#endif // DISABLE_DRIVER
