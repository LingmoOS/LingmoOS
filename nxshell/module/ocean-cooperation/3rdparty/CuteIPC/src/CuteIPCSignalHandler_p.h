#ifndef CUTEIPCSIGNALHANDLER_P_H
#define CUTEIPCSIGNALHANDLER_P_H

// Qt
#include <QObject>
class QMetaMethod;

// Local
class CuteIPCService;
class CuteIPCServiceConnection;


class CuteIPCSignalHandler : public QObject
{
  Q_OBJECT_FAKE

  public:
    explicit CuteIPCSignalHandler(const QString& signature, QObject* parent = 0);
    ~CuteIPCSignalHandler();

    //To send signals from client-side local objects, pass server's slot signature to constructor
    //and object's signal to setSignalParametersInfo.
    //Use this method to set server's signal owner also (with the same signature as passed in constructor)
    void setSignalParametersInfo(QObject* owner, const QString& signature);
    QString signature() const;

//  public slots:
    void relaySlot(void**);
    void addListener(CuteIPCServiceConnection* listener);
    void removeListener(CuteIPCServiceConnection* listener);
    void listenerDestroyed(QObject* listener);

  protected:
//  signals:
    void signalCaptured(const QByteArray& data);
    void destroyed(QString signature);

  private:
    QString m_signature;
    QList<QByteArray> m_signalParametersInfo;
    bool m_signalParametersInfoWasSet;
    QList<CuteIPCServiceConnection*> m_listeners;
};

#endif // CUTEIPCSIGNALHANDLER_P_H
