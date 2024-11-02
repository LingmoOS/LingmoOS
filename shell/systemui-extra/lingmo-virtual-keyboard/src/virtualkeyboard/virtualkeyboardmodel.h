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

#ifndef VIRTUALKEYBOARDMODEL_H
#define VIRTUALKEYBOARDMODEL_H

#include <memory>

#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QObject>

#include <Fcitx5Qt5/Fcitx5Qt5DBusAddons/fcitxqtcontrollerproxy.h>

class VirtualKeyboardModel : public QObject {
    Q_OBJECT
public:
    explicit VirtualKeyboardModel(QObject *parent = nullptr);
    ~VirtualKeyboardModel() override = default;

public:
    Q_PROPERTY(QString uniqueName READ getUniqueName NOTIFY uniqueNameChanged);
    Q_PROPERTY(QVariant currentIMList READ getCurrentIMList NOTIFY
                   currentIMListChanged);
    Q_PROPERTY(int preeditCaret READ getPreeditCaret NOTIFY preeditCaretChanged)
    Q_PROPERTY(
        QString preeditText READ getPreeditText NOTIFY preeditTextChanged);

signals:
    void uniqueNameChanged();

    void currentIMListChanged();

    void preeditCaretChanged();
    void preeditTextChanged();

public:
    void setUniqueName(const QString &uniqueName);

    void syncCurrentIMList();

    void updateCandidateArea(const QVariant &candidateTextList, bool hasPrev,
                             bool hasNext, int pageIndex,
                             int globalCursorIndex);

    void setPreeditCaret(int preeditCaret);
    void setPreeditText(const QString &preeditText);

public:
    Q_INVOKABLE void selectCandidate(int index);
    Q_INVOKABLE void setCurrentIM(const QString &imName);
    Q_INVOKABLE void processKeyEvent(int keysym, int keycode, int state,
                                     bool isRelease, int time);

signals:
    // TODO(linyuxuan): 使用更准确的类型替换QVariant
    void updateCandidateArea(const QVariant &candidateTextList,
                             int globalCursorIndex);
    void imDeactivated();

    void backendConnectionDisconnected();

private slots:
    void backendServiceRegistered(const QString &serviceName);
    void backendServiceUnregistered(const QString &serviceName);

private:
    void initFcitx5Controller();
    void initDBusServiceWatcher();
    void initVirtualKeyboardBackendInterface();

    QString getUniqueName() const;
    void syncUniqueName();

    QVariant getCurrentIMList() const;
    void setCurrentIMList(const QVariant &currentIMList);

    int getPreeditCaret() const;
    QString getPreeditText() const;

private:
    std::unique_ptr<QDBusServiceWatcher> serviceWatcher_ = nullptr;
    std::unique_ptr<QDBusInterface> virtualKeyboardBackendInterface_ = nullptr;
    std::unique_ptr<fcitx::FcitxQtControllerProxy> fcitx5Controller_ = nullptr;

    QString uniqueName_;

    QVariant currentIMList_;

    int preeditCaret_ = 0;
    QString preeditText_;

    QString virtualKeyboardBackendService =
        "org.fcitx.Fcitx5.VirtualKeyboardBackend";
    QString virtualKeyboardBackendServicePath = "/virtualkeyboard";
    QString virtualKeyboardBackendServiceInterface =
        "org.fcitx.Fcitx5.VirtualKeyboardBackend1";
};

#endif // VIRTUALKEYBOARDMODEL_H
