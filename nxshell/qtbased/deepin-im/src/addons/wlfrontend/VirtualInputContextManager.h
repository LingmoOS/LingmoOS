// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALINPUTCONTEXTMANAGER_H
#define VIRTUALINPUTCONTEXTMANAGER_H

#include <QObject>

#include <memory>

namespace org::deepin::dim {

class AppMonitor;
class VirtualInputContext;
class Dim;
class VirtualInputContextGlue;

class VirtualInputContextManager : public QObject
{
public:
    VirtualInputContextManager(VirtualInputContextGlue *parentIC,
                               const std::shared_ptr<AppMonitor> &appMonitor,
                               Dim *dim);
    ~VirtualInputContextManager();

    void setRealFocus(bool focus);
    VirtualInputContext *focusedVirtualIC();

private:
    Dim *dim_;
    VirtualInputContextGlue *parentIC_;
    std::shared_ptr<AppMonitor> appMonitor_;
    std::unordered_map<std::string, std::string> lastAppState_;
    std::unordered_map<std::string, std::unique_ptr<VirtualInputContext>> managed_;
    std::string focus_;

    void appUpdated(const std::unordered_map<std::string, std::string> &appState,
                    const std::string &focus);
    void updateFocus();
};

} // namespace org::deepin::dim

#endif // !VIRTUALINPUTCONTEXTMANAGER_H
