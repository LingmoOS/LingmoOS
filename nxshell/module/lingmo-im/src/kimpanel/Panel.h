// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_H
#define PANEL_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include "ImpanelInputmethod.h"

class ImpanelAdaptor;
class Impanel2Adaptor;

class Panel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool enable MEMBER enable_ NOTIFY enableChanged)
    Q_PROPERTY(QVariantList properties MEMBER properties_ NOTIFY propertiesChanged)
    Q_PROPERTY(bool showAux MEMBER showAux_ NOTIFY showAuxChanged)
    Q_PROPERTY(bool showLookupTable MEMBER showLookupTable_ NOTIFY showLookupTableChanged)
    Q_PROPERTY(bool showPreedit MEMBER showPreedit_ NOTIFY showPreeditChanged)
    Q_PROPERTY(QVariantMap aux MEMBER aux_ NOTIFY auxChanged)
    Q_PROPERTY(QVariantMap preedit MEMBER preedit_ NOTIFY preeditChanged)
    Q_PROPERTY(int preeditCaretPos MEMBER preeditCaretPos_ NOTIFY preeditCaretPosChanged)

    // lookup table
    Q_PROPERTY(QPoint pos MEMBER pos_ NOTIFY posChanged)
    Q_PROPERTY(QVariantList lookupTable MEMBER lookupTable_ NOTIFY lookupTableChanged)
    Q_PROPERTY(bool hasPrev MEMBER hasPrev_ NOTIFY lookupTableChanged)
    Q_PROPERTY(bool hasNext MEMBER hasNext_ NOTIFY lookupTableChanged)
    Q_PROPERTY(qint32 cursor MEMBER cursor_ NOTIFY lookupTableChanged)
    Q_PROPERTY(qint32 layout MEMBER layout_ NOTIFY lookupTableChanged)

public:
    explicit Panel(QObject *parent = nullptr);

public:
    Q_INVOKABLE void menuTriggered(const QString &key);
    Q_INVOKABLE void reloadTriggered();
    Q_INVOKABLE void configureTriggered();

signals: // properties signal
    void enableChanged(bool enable);
    void propertiesChanged(const QVariantList &properties);
    void showAuxChanged(bool showAux);
    void showLookupTableChanged(bool showLookupTable);
    void showPreeditChanged(bool showPreedit);
    void auxChanged(const QVariantMap &aux);
    void preeditChanged(const QVariantMap &preedit);
    void preeditCaretPosChanged(int pos);

    void posChanged(const QPoint &pos);
    void lookupTableChanged();

private slots: // impanel methods
    void onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void onEnable(bool enable);
    void onExecDialog(const QString &prop);
    void onExecMenu(const QStringList &actions);
    void onRegisterProperties(const QStringList &prop);
    void onRemoveProperty(const QString &key);
    void onShowAux(bool toshow);
    void onShowLookupTable(bool toshow);
    void onShowPreedit(bool toshow);
    void onUpdateAux(const QString &text, const QString &attr);
    void onUpdateLookupTable(const QStringList &labels,
                             const QStringList &candidates,
                             const QStringList &attrs,
                             bool hasprev,
                             bool hasnext);
    void onUpdateLookupTableCursor(int pos);
    void onUpdatePreeditCaret(int pos);
    void onUpdatePreeditText(const QString &text, const QString &attr);
    void onUpdateProperty(const QString &prop);
    void onUpdateSpotLocation(int x, int y);

public: // impanel2 signal callbacks
    void SetSpotRect(qint32 x, qint32 y, qint32 w, qint32 h);
    void SetRelativeSpotRect(qint32 x, qint32 y, qint32 w, qint32 h);
    void SetRelativeSpotRectV2(qint32 x, qint32 y, qint32 w, qint32 h, double scale);
    void SetLookupTable(const QStringList &label,
                        const QStringList &text,
                        const QStringList &attr,
                        bool hasPrev,
                        bool hasNext,
                        qint32 cursor,
                        qint32 layout);

private:
    QDBusConnection bus_;
    QDBusServiceWatcher *watcher_;
    ImpanelAdaptor *kimpanelAdaptor_;
    Impanel2Adaptor *kimpanel2Adaptor_;
    org::kde::kimpanel::inputmethod *inputmethodIface_;

private: // properties
    bool enable_;
    QVariantList properties_;
    bool showAux_;
    bool showLookupTable_;
    bool showPreedit_;
    QVariantMap aux_;
    QVariantMap preedit_;
    int preeditCaretPos_;

    QPoint pos_;
    QVariantList lookupTable_;
    bool hasPrev_;
    bool hasNext_;
    qint32 cursor_;
    qint32 layout_;
};

#endif // !PANEL_H
