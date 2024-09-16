// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIM_H
#define DIM_H

#include "InputMethodEntry.h"

#include <QObject>

#include <set>

#ifdef Dtk6Core_FOUND
#  include <DConfig>
#endif

namespace org {
namespace deepin {
namespace dim {

enum AddonType {
    Frontend,
    InputMethod,
};

class InputContext;
class InputState;
class InputMethodAddon;
class ProxyAddon;
class FrontendAddon;
class Addon;
class Event;
class InputContextEvent;
class InputContextKeyEvent;
class InputContextCursorRectChangeEvent;
class InputContextSetSurroundingTextEvent;
class ProxyEvent;

struct AddonDesc;

class Dim : public QObject
{
    Q_OBJECT

public:
    explicit Dim(QObject *parent = nullptr);
    ~Dim();

    bool postEvent(Event &event);

    const std::unordered_map<uint32_t, InputContext *> &getInputContexts() const
    {
        return inputContexts_;
    }

    InputContext *getFocusedIC(uint32_t id) const;

    const std::unordered_map<std::string, Addon *> &addons() const { return addons_; }

    const auto &activeInputMethodEntries() const { return activeInputMethodEntries_; }

    const auto &getCurrentActiveInputMethod() const { return currentActiveIM_; }

    const std::vector<InputMethodEntry> &imEntries() const { return imEntries_; }

    int focusedInputContext() const { return focusedInputContext_; }

    void addInputMethod(const std::string &addon, const std::string &name);
    void removeInputMethod(const std::string &addon, const std::string &name);

Q_SIGNALS:
    void focusedInputContextChanged(int focusedInputContext);
    void inputMethodEntryChanged();

public Q_SLOTS:
    void switchIM(const std::pair<std::string, std::string> &imIndex);
    bool requestSwitchIM(const std::string &addon, const std::string &name);
    void toggle();

private:
    void loadAddons();
    void loadAddon(const AddonDesc &infoFile);
    void initInputMethodAddon(InputMethodAddon *imAddon);
    void postInputContextCreated(InputContextEvent &event);
    void postInputContextDestroyed(InputContextEvent &event);
    void postInputContextFocused(InputContextEvent &event);
    void postInputContextUnfocused(InputContextEvent &event);
    bool postInputContextKeyEvent(InputContextKeyEvent &event);
    void postInputContextCursorRectChanged(InputContextCursorRectChangeEvent &event);
    void postInputContextUpdateContentType(InputContextEvent &event);
    void postInputContextUpdateSurroundingTextEvent(InputContextEvent &event);
    void postInputContextDone(InputContextEvent &event);
    void postProxyActivateInputMethodChanged(ProxyEvent &event);
    void addActiveInputMethodEntry(const std::string &addon, const std::string &entry);
    InputMethodAddon *getInputMethodAddon(const InputState &state);
    void loopProxyAddon(const std::function<void(ProxyAddon *addon)> callback);
    QString indexToKey(const std::pair<std::string, std::string> &imIndex) const;
    const std::pair<std::string, std::string> keyToIndex(const QString &imKey) const;
#ifdef Dtk6Core_FOUND
    void initDConfig();
    void updateDconfInputMethodEntries() const;
#endif

private:
    std::unordered_map<uint32_t, InputContext *> inputContexts_;
    uint32_t focusedInputContext_;
    std::unordered_map<std::string, Addon *> addons_;
    std::vector<InputMethodEntry> imEntries_;
    std::set<std::pair<std::string, std::string>> activeInputMethodEntries_;
    std::pair<std::string, std::string> currentActiveIM_;
#ifdef Dtk6Core_FOUND
    DTK_CORE_NAMESPACE::DConfig *dimConf_;
#endif
};

} // namespace dim

} // namespace deepin

} // namespace org

#endif // !DIM_H
