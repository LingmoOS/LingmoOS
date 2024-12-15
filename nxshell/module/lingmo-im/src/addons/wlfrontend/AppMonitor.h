#ifndef APPMONITOR_H
#define APPMONITOR_H

#include <QObject>

namespace org::lingmo::dim {

class AppMonitor : public QObject
{
    Q_OBJECT

public:
    AppMonitor();
    virtual ~AppMonitor();

signals:
    void appUpdated(const std::unordered_map<std::string, std::string> &appState, const std::string &focus);
};

} // namespace org::lingmo::dim

#endif // !APPMONITOR_H
