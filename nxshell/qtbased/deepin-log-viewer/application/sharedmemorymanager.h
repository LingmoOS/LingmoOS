// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREDMEMORYMANAGER_H
#define SHAREDMEMORYMANAGER_H

#include <QObject>

#include <mutex>
struct ShareMemoryInfo {
    bool isStart = true ;
};
class QSharedMemory;
class SharedMemoryManager : public QObject
{
public:

    static SharedMemoryManager *instance()
    {
        SharedMemoryManager *sin = m_instance.load();
        if (!sin) {
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = m_instance.load();
            if (!sin) {
                sin = new SharedMemoryManager();
                m_instance.store(sin);
            }
        }
        return sin;
    }
    static SharedMemoryManager *getInstance();
    void setRunnableTag(ShareMemoryInfo iShareInfo);
    QString getRunnableKey();
    bool isAttached();
    void releaseMemory();

protected:
    explicit SharedMemoryManager(QObject *parent = nullptr);

    void init();
private:
    static std::atomic<SharedMemoryManager *> m_instance;
    static std::mutex m_mutex;
    QSharedMemory  *m_commondM = nullptr;
    ShareMemoryInfo *m_pShareMemoryInfo = nullptr;
};

#endif // SHAREDMEMORYMANAGER_H
