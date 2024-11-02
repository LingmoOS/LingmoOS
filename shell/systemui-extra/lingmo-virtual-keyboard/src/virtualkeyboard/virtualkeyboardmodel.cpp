/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "virtualkeyboardmodel.h"

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingReply>

VirtualKeyboardModel::VirtualKeyboardModel(QObject *parent) : QObject(parent) {
    initFcitx5Controller();
    initDBusServiceWatcher();
}

void VirtualKeyboardModel::updateCandidateArea(
    const QVariant &candidateTextList, bool /*hasPrev*/, bool /*hasNext*/,
    int /*pageIndex*/, int globalCursorIndex) {
    emit updateCandidateArea(candidateTextList, globalCursorIndex);
}

void VirtualKeyboardModel::selectCandidate(int index) {
    virtualKeyboardBackendInterface_->asyncCall("SelectCandidate", index);
}

void VirtualKeyboardModel::setCurrentIM(const QString &imName) {
    fcitx5Controller_->SetCurrentIM(imName);
}

void VirtualKeyboardModel::processKeyEvent(int keysym, int keycode, int state,
                                           bool isRelease, int time) {
    virtualKeyboardBackendInterface_->asyncCall("ProcessKeyEvent", (uint)keysym,
                                                (uint)keycode, (uint)state,
                                                isRelease, (uint)time);
}

void VirtualKeyboardModel::initFcitx5Controller() {
    fcitx::registerFcitxQtDBusTypes();

    fcitx5Controller_.reset(new fcitx::FcitxQtControllerProxy(
        "org.fcitx.Fcitx5", "/controller", QDBusConnection::sessionBus()));
}

void VirtualKeyboardModel::initDBusServiceWatcher() {
    serviceWatcher_.reset(new QDBusServiceWatcher(this));
    serviceWatcher_->setConnection(QDBusConnection::sessionBus());
    serviceWatcher_->addWatchedService(virtualKeyboardBackendService);
    serviceWatcher_->setWatchMode(QDBusServiceWatcher::WatchForRegistration |
                                  QDBusServiceWatcher::WatchForUnregistration);
    connect(serviceWatcher_.get(), SIGNAL(serviceRegistered(const QString &)),
            this, SLOT(backendServiceRegistered(const QString &)));
    connect(serviceWatcher_.get(), SIGNAL(serviceUnregistered(const QString &)),
            this, SLOT(backendServiceUnregistered(const QString &)));
}

void VirtualKeyboardModel::initVirtualKeyboardBackendInterface() {
    virtualKeyboardBackendInterface_.reset(new QDBusInterface(
        virtualKeyboardBackendService, virtualKeyboardBackendServicePath,
        virtualKeyboardBackendServiceInterface, QDBusConnection::sessionBus(),
        this));
}

void VirtualKeyboardModel::backendServiceRegistered(
    const QString &serviceName) {
    if (serviceName != virtualKeyboardBackendService) {
        return;
    }
    initVirtualKeyboardBackendInterface();

    syncUniqueName();

    syncCurrentIMList();
}

void VirtualKeyboardModel::backendServiceUnregistered(
    const QString &serviceName) {
    if (serviceName != virtualKeyboardBackendService) {
        return;
    }

    emit backendConnectionDisconnected();

    virtualKeyboardBackendInterface_.reset();
}

QString VirtualKeyboardModel::getUniqueName() const { return uniqueName_; }

void VirtualKeyboardModel::setUniqueName(const QString &uniqueName) {
    if (uniqueName_ == uniqueName) {
        return;
    }

    uniqueName_ = uniqueName;

    emit uniqueNameChanged();
}

void VirtualKeyboardModel::syncUniqueName() {
    QDBusPendingReply<QString> reply = fcitx5Controller_->CurrentInputMethod();
    reply.waitForFinished();

    setUniqueName(reply.value());
}

QVariant VirtualKeyboardModel::getCurrentIMList() const {
    return currentIMList_;
}

void VirtualKeyboardModel::setCurrentIMList(
    const QVariant &currentInputMethodList) {
    if (currentIMList_ == currentInputMethodList) {
        return;
    }

    currentIMList_ = currentInputMethodList;

    emit currentIMListChanged();
}

int VirtualKeyboardModel::getPreeditCaret() const { return preeditCaret_; }

void VirtualKeyboardModel::setPreeditCaret(int preeditCaret) {
    preeditCaret_ = preeditCaret;

    emit preeditCaretChanged();
}

QString VirtualKeyboardModel::getPreeditText() const { return preeditText_; }

void VirtualKeyboardModel::setPreeditText(const QString &preeditText) {
    preeditText_ = preeditText;

    emit preeditTextChanged();
}

void VirtualKeyboardModel::syncCurrentIMList() {
    auto reply = fcitx5Controller_->FullInputMethodGroupInfo("");
    reply.waitForFinished();
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return;
    }

    QStringList stringList;
    auto inputMethodEntryList = reply.argumentAt<4>();
    for (const auto &inputMethodEntry : inputMethodEntryList) {
        stringList.append(inputMethodEntry.uniqueName() + "|" +
                          inputMethodEntry.name() + "|" +
                          inputMethodEntry.label());
    }

    setCurrentIMList(QVariant(stringList));
}
