// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Fcitx5Proxy.h"

#include "DBusProvider.h"
#include "wladdonsbase/InputMethodV2.h"
#include "wladdonsbase/Keyboard.h"
#include "addons/waylandserver/WaylandServer_public.h"
#include "addons/wlfrontend/WLFrontend_public.h"
#include "dimcore/Dim.h"
#include "dimcore/Events.h"
#include "dimcore/InputContext.h"
#include "dimcore/InputMethodEntry.h"
#include "wl/client/Compositor.h"
#include "wl/client/ConnectionBase.h"
#include "wl/client/ConnectionRaw.h"
#include "wl/client/ZwpInputMethodV2.h"

#include <QGuiApplication>

using namespace org::deepin::dim;
WL_ADDONS_BASE_USE_NAMESPACE

static const QString DIM_IM_GROUP = "dim";
static const std::string KEYBOARD_PREFIX = "keyboard-";

static const char *SOCKET_NAME = "dim";

DIM_ADDON_FACTORY(Fcitx5Proxy);

Fcitx5Proxy::Fcitx5Proxy(Dim *dim)
    : ProxyAddon(dim, "fcitx5", "org.fcitx.Fcitx5")
    , focusedId_(0)
    , fcitx5Proc_(new QProcess(this))
{
    Addon *wls = dim->addons().at("waylandserver");
    auto wl = waylandserver::getServer(wls);
    auto *surface = waylandserver::getSurface(wls);

    wl_ = wl;

    wl_->setVirtualKeyboardCallback([this](Keyboard *keyboard) {
        keyboard->setKeyEventCallback([this](wlr_keyboard_key_event *event) {
            auto *ic = getFocusedIC(focusedId_);
            if (!ic) {
                return;
            }

            ic->forwardKey(event->keycode, event->state);
        });
    });

    wl_->setInputMethodCallback([this, surface]() {
        auto *im = wl_->inputMethodV2(IMType::FCITX5);

        im->setCommitCallback([this, im]() {
            auto *ic = getFocusedIC(focusedId_);
            if (!ic) {
                return;
            }

            auto &preeditInfo = im->preeditString();
            if (preeditInfo.cursor_begin || preeditInfo.cursor_end) {
                ic->updatePreedit(preeditInfo.text,
                                  preeditInfo.cursor_begin,
                                  preeditInfo.cursor_end);
            }
            if (im->commitText()) {
                ic->commitString(im->commitText());
            }
            ic->commit();
        });

        im->setPopupCreateCallback([this, surface]() {
            popup_.reset();

            auto *ic = getFocusedIC(focusedId_);
            assert(ic);

            auto *im = wlfrontend::getInputMethodV2(ic);
            auto *pop = im->get_input_popup_surface(surface);
            if (!pop) {
                qWarning() << "failed to get popup surface";
            }
            popup_.reset(new InputPopupSurfaceV2(pop));
        });
        im->setPopupDestroyCallback([this]() {
            popup_.reset();
        });
    });

    launchDaemon();
}

void Fcitx5Proxy::initDBusConn()
{
    dbusProvider_ = new DBusProvider(this);
    available_ = dbusProvider_->available();

    connect(dbusProvider_, &DBusProvider::availabilityChanged, this, [this](bool available) {
        if (available_ != available) {
            available_ = available;

            updateInputMethods();
        }
    });

    updateInputMethods();
}

void Fcitx5Proxy::initInputMethods()
{
    updateInputMethods();
}

Fcitx5Proxy::~Fcitx5Proxy() { }

const QList<InputMethodEntry> &Fcitx5Proxy::getInputMethods()
{
    return inputMethods_;
}

void Fcitx5Proxy::focusIn(uint32_t id)
{
    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    focusedId_ = id;
    im->sendActivate();
}

void Fcitx5Proxy::focusOut(uint32_t id)
{
    if (focusedId_ != id) {
        return;
    }

    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    im->sendDeactivate();
}

void Fcitx5Proxy::destroyed(uint32_t id) { }

void Fcitx5Proxy::done()
{
    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    im->sendDone();
}

void Fcitx5Proxy::contentType(uint32_t hint, uint32_t purpose)
{
    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    im->sendContentType(hint, purpose);
}

void Fcitx5Proxy::setCurrentIM(const std::string &im)
{
    if (available_ && dbusProvider_) {
        dbusProvider_->controller()->SetCurrentIM(QString::fromStdString(im));
    }
}

bool Fcitx5Proxy::keyEvent([[maybe_unused]] const InputMethodEntry &entry,
                           InputContextKeyEvent &keyEvent)
{
    auto id = keyEvent.ic()->id();
    if (focusedId_ != id) {
        return false;
    }

    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return false;
    }

    im->sendKey(keyEvent.keycode(), keyEvent.isRelease());

    return true;
}

void Fcitx5Proxy::cursorRectangleChangeEvent(InputContextCursorRectChangeEvent &event)
{
    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    im->setCursorRectangle(event.x, event.y, event.w, event.h);
}

void Fcitx5Proxy::updateSurroundingText(InputContextEvent &event)
{
    auto &surroundingText = event.ic()->surroundingText();

    auto *im = wl_->inputMethodV2(IMType::FCITX5);
    if (!im) {
        return;
    }

    im->sendSurroundingText(surroundingText.text().toUtf8().data(),
                            surroundingText.cursor(),
                            surroundingText.anchor());
}

void Fcitx5Proxy::updateInputMethods()
{
    if (!available_ || !dbusProvider_) {
        inputMethods_.clear();
        return;
    }

    auto *controller = dbusProvider_->controller();

    {
        auto call = controller->AvailableInputMethods();
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher,
                &QDBusPendingCallWatcher::finished,
                this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();

                    QDBusPendingReply<FcitxQtInputMethodEntryList> reply = *watcher;

                    QList<InputMethodEntry> inputMethods;
                    for (auto &im : reply.value()) {
                        // 过滤掉键盘布局
                        std::string uniqueName = im.uniqueName().toStdString();
                        if (shouldBeIgnored(uniqueName)) {
                            continue;
                        }

                        inputMethods.append(InputMethodEntry(key(),
                                                             uniqueName,
                                                             im.name().toStdString(),
                                                             im.nativeName().toStdString(),
                                                             im.label().toStdString(),
                                                             im.icon().toStdString()));
                    }

                    inputMethods_.swap(inputMethods);
                    Q_EMIT addonInitFinished(this);
                });
    }

    {
        auto call = controller->InputMethodGroups();
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher,
                &QDBusPendingCallWatcher::finished,
                this,
                [this, controller](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();

                    QDBusPendingReply<QStringList> reply = *watcher;
                    auto groups = reply.value();
                    auto group = groups[0];

                    auto groupInfoCall = controller->InputMethodGroupInfo(group);
                    groupInfoCall.waitForFinished();

                    std::vector<std::string> activeInputMethods;
                    auto ims = groupInfoCall.argumentAt<1>();
                    for (auto &im : ims) {
                        auto uniqueName = im.key().toStdString();
                        if (shouldBeIgnored(uniqueName)) {
                            continue;
                        }
                        activeInputMethods.emplace_back(uniqueName);
                    }

                    updateActiveInputMethods(activeInputMethods);
                });
    }
}

bool Fcitx5Proxy::shouldBeIgnored(const std::string &uniqueName) const
{
    return std::mismatch(KEYBOARD_PREFIX.begin(), KEYBOARD_PREFIX.end(), uniqueName.begin()).first
        == KEYBOARD_PREFIX.end();
}

void Fcitx5Proxy::launchDaemon()
{
    if (!isExecutableExisted(QStringLiteral("fcitx5"))) {
        qDebug() << "can not find fcitx5 executable, maybe it should be installed";
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("WAYLAND_DISPLAY", SOCKET_NAME);

    fcitx5Proc_->setProgram(QStringLiteral("fcitx5"));
    fcitx5Proc_->setArguments(QStringList{ QStringLiteral("--disable"),
                                           QStringLiteral("fcitx4frontend,ibusfrontend,xim,notificationitem"),
                                           QStringLiteral("-r") });
    fcitx5Proc_->setProcessEnvironment(env);
    fcitx5Proc_->setStandardOutputFile("/tmp/fcitx5.log");
    fcitx5Proc_->setStandardErrorFile("/tmp/fcitx5.log");
    fcitx5Proc_->start();

    connect(fcitx5Proc_, &QProcess::started, this, [this] {
        qDebug() << "launch fcitx5 success" << fcitx5Proc_->processId();
        initDBusConn();
    });
}

InputContext *Fcitx5Proxy::getFocusedIC(uint32_t id) const
{
    return dim()->getFocusedIC(id);
}
