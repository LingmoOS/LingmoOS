#include "kthread.h"
#include <QThread>

namespace KServer {

void quitThreadRun(QThread* m_thread)
{
    Q_ASSERT(m_thread);
    if(m_thread){
        m_thread->quit();
        if(!m_thread->wait(3)) {
           m_thread->terminate();
           m_thread->wait();
        }

    }
}
}
