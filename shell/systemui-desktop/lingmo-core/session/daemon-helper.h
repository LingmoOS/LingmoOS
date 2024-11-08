#ifndef __DAEMON_HELPER_
#define __DAEMON_HELPER_

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QList>

namespace LINGMO_SESSION {
  class Daemon : public QObject {
    Q_OBJECT

  public:
    /**
     * Start all the passed process using daemon.
     * @param processList Process list to start
     * @param parent
     */
    explicit Daemon(const QList<QPair<QString, QStringList>>& processList, bool _enableAutoStart = true, QObject* parent = nullptr);

  public slots:

    /**
     *  Handle the case when the progarm has some errors (i.e. crashed)
     * @param error
     */
    void onProcessError(QProcess::ProcessError error);

  private:
    /**
     *  Start a given process using daemon helper
     * @brief startProcess
     * @param processInfo
     */
    void startProcess(const QPair<QString, QStringList>& processInfo);

    QList<QPair<QString, QStringList>> m_processList;

    /**
     * @brief Whether to enable auto reload when process exited.
     */
    bool m_enableAutoRestart;
  };
}
#endif
