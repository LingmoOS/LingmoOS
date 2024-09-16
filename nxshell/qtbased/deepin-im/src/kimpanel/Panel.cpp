// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Panel.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTransform>
#include <QGuiApplication>
#include <QScreen>

#include "ImpanelAdaptor.h"
#include "Impanel2Adaptor.h"

using namespace org::kde::kimpanel;

static const QString BUS_SERVICE = QStringLiteral("org.kde.impanel");
static const QString BUS_PATH = QStringLiteral("/org/kde/impanel");

static const QString INPUTMETHOD_BUS_SERVICE = QStringLiteral("org.kde.kimpanel.inputmethod");
static const QString INPUTMETHOD_BUS_PATH = QStringLiteral("/kimpanel");
static const QString INPUTMETHOD_BUS_INTERFACE = QStringLiteral("org.kde.kimpanel.inputmethod");

static const QVariantMap EMPTY_ELEMENT{{"text", ""}, {"attr", ""}};

static QVariantMap parseProperty(const QString &str) {
    const auto prop = str.split(':');
    return QVariantMap{
        {"key", prop[0]},
        {"label", prop[1]},
        {"icon", prop[2]},
        {"text", prop[3]},
        {"hints", prop[4].split(',')},
    };
}

static QPoint mapPosition(const QPoint &p) {
    auto screens = QGuiApplication::screens();
    for (auto *screen : screens) {
        auto geo = screen->geometry();
        auto ratio = screen->devicePixelRatio();

        QTransform logiToPhysTrans;
        logiToPhysTrans *= ratio;
        QRect physGeo = logiToPhysTrans.mapRect(geo);

        if (physGeo.contains(p)) {
            QTransform phisTologiTrans;
            phisTologiTrans /= ratio;
            QPoint logiPoint = phisTologiTrans.map(p);
            logiPoint.rx() += geo.x();
            logiPoint.ry() += geo.y();

            return logiPoint;
        }
    }

    return p;
}

Panel::Panel(QObject *parent)
    : QObject(parent)
    , bus_(QDBusConnection::sessionBus())
    , watcher_(new QDBusServiceWatcher(INPUTMETHOD_BUS_SERVICE, bus_, QDBusServiceWatcher::WatchForOwnerChange, this))
    , kimpanelAdaptor_(new ImpanelAdaptor(this))
    , kimpanel2Adaptor_(new Impanel2Adaptor(this))
    , inputmethodIface_(new org::kde::kimpanel::inputmethod(INPUTMETHOD_BUS_SERVICE, INPUTMETHOD_BUS_PATH, bus_, this))
    , enable_(false)
    , showAux_(false)
    , showLookupTable_(false)
    , showPreedit_(false)
    , aux_(EMPTY_ELEMENT)
    , preedit_(EMPTY_ELEMENT) {
    bus_.registerService(BUS_SERVICE);

    if (!bus_.registerObject(BUS_PATH, this)) {
        qWarning() << "Failed to register request object for" << bus_.lastError().message();
    }

    connect(watcher_, &QDBusServiceWatcher::serviceOwnerChanged, this, &Panel::onServiceOwnerChanged);

    connect(inputmethodIface_, &inputmethod::Enable, this, &Panel::onEnable);
    connect(inputmethodIface_, &inputmethod::ExecDialog, this, &Panel::onExecDialog);
    connect(inputmethodIface_, &inputmethod::ExecMenu, this, &Panel::onExecMenu);
    connect(inputmethodIface_, &inputmethod::RegisterProperties, this, &Panel::onRegisterProperties);
    connect(inputmethodIface_, &inputmethod::RemoveProperty, this, &Panel::onRemoveProperty);
    connect(inputmethodIface_, &inputmethod::ShowAux, this, &Panel::onShowAux);
    connect(inputmethodIface_, &inputmethod::ShowLookupTable, this, &Panel::onShowLookupTable);
    connect(inputmethodIface_, &inputmethod::ShowPreedit, this, &Panel::onShowPreedit);
    connect(inputmethodIface_, &inputmethod::UpdateAux, this, &Panel::onUpdateAux);
    connect(inputmethodIface_, &inputmethod::UpdateLookupTable, this, &Panel::onUpdateLookupTable);
    connect(inputmethodIface_, &inputmethod::UpdateLookupTableCursor, this, &Panel::onUpdateLookupTableCursor);
    connect(inputmethodIface_, &inputmethod::UpdatePreeditCaret, this, &Panel::onUpdatePreeditCaret);
    connect(inputmethodIface_, &inputmethod::UpdatePreeditText, this, &Panel::onUpdatePreeditText);
    connect(inputmethodIface_, &inputmethod::UpdateProperty, this, &Panel::onUpdateProperty);
    connect(inputmethodIface_, &inputmethod::UpdateSpotLocation, this, &Panel::onUpdateSpotLocation);

    emit kimpanelAdaptor_->PanelCreated();
    emit kimpanel2Adaptor_->PanelCreated2();
}

void Panel::menuTriggered(const QString &name) {
    emit kimpanelAdaptor_->TriggerProperty(name);
}

void Panel::reloadTriggered() {
    emit kimpanelAdaptor_->ReloadConfig();
}

void Panel::configureTriggered() {
    emit kimpanelAdaptor_->Configure();
}

void Panel::onServiceOwnerChanged([[maybe_unused]] const QString &service,
                                    [[maybe_unused]] const QString &oldOwner,
                                    const QString &newOwner) {
    if (newOwner.isEmpty()) {
        // TODO:
        onRegisterProperties({});
        return;
    }

    // TODO:
}

void Panel::onEnable(bool enable) {
    enable_ = enable;
    emit enableChanged(enable_);
}

void Panel::onExecDialog(const QString &prop) {
    Q_UNUSED(prop);
}

void Panel::onExecMenu(const QStringList &actions) {
    Q_UNUSED(actions);
}

void Panel::onRegisterProperties(const QStringList &prop) {
    QVariantList props;
    props.reserve(prop.length());
    for (const auto &p : prop) {
        props << parseProperty(p);
    }
    properties_ = props;
    emit propertiesChanged(properties_);
}

void Panel::onRemoveProperty(const QString &key) {
    properties_.erase(std::remove_if(properties_.begin(), properties_.end(), [key](const QVariant &p) {
        return p.toMap()["key"] == key;
    }));
    emit propertiesChanged(properties_);
}

void Panel::onShowAux(bool toshow) {
    qInfo() << "onShowAux" << toshow;
    showAux_ = toshow;
    emit showAuxChanged(showAux_);
}

void Panel::onShowLookupTable(bool toshow) {
    qInfo() << "onShowLookupTable" << toshow;
    showLookupTable_ = toshow;
    emit showLookupTableChanged(showLookupTable_);
}

void Panel::onShowPreedit(bool toshow) {
    qInfo() << "onShowPreedit" << toshow;
    showPreedit_ = toshow;
    emit showPreeditChanged(showPreedit_);
}

void Panel::onUpdateAux(const QString &text, const QString &attr) {
    aux_ = QVariantMap{{"text", text}, {"attr", attr}};
    emit auxChanged(aux_);
}

void Panel::onUpdateLookupTable(const QStringList &labels,
                                  const QStringList &candidates,
                                  const QStringList &attrs,
                                  bool hasprev,
                                  bool hasnext) {
    Q_UNUSED(labels);
    Q_UNUSED(candidates);
    Q_UNUSED(attrs);
    Q_UNUSED(hasprev);
    Q_UNUSED(hasnext);
}

void Panel::onUpdateLookupTableCursor(int pos) {
    Q_UNUSED(pos);
}

void Panel::onUpdatePreeditCaret(int pos) {
    preeditCaretPos_ = pos;
    emit preeditCaretPosChanged(preeditCaretPos_);
}

void Panel::onUpdatePreeditText(const QString &text, const QString &attr) {
    preedit_ = QVariantMap{{"text", text}, {"attr", attr}};
    emit preeditChanged(preedit_);
}

void Panel::onUpdateProperty(const QString &prop) {
    auto property = parseProperty(prop);
    std::replace_if(
        properties_.begin(),
        properties_.end(),
        [key = property["key"]](const QVariant &p) { return p.toMap()["key"] == key; },
        property);
    emit propertiesChanged(properties_);
}

void Panel::onUpdateSpotLocation(int x, int y) {
    qInfo() << "onUpdateSpotLocation" << QPoint(x, y);
}

void Panel::SetLookupTable(const QStringList &label,
                             const QStringList &text,
                             const QStringList &attr,
                             bool hasPrev,
                             bool hasNext,
                             qint32 cursor,
                             qint32 layout) {
    hasPrev_ = hasPrev;
    hasNext_ = hasNext;
    cursor_ = cursor;
    layout_ = layout;

    QVariantList list;
    list.reserve(label.size());
    for (int i = 0; i < label.size(); i++) {
        list << QVariantMap{
            {"label", label[i]},
            {"text", text[i]},
            {"attr", attr[i]},
        };
    }
    lookupTable_.swap(list);

    emit lookupTableChanged();
}

void Panel::SetSpotRect(qint32 x, qint32 y, qint32 w, qint32 h) {
    QPoint ori(x + w, y + h);
    pos_ = mapPosition(ori);

    emit posChanged(pos_);
}

void Panel::SetRelativeSpotRect(qint32 x, qint32 y, qint32 w, qint32 h) {
    SetRelativeSpotRectV2(x, y, w, h, 1);
}

void Panel::SetRelativeSpotRectV2(qint32 x, qint32 y, qint32 w, qint32 h, double scale) {
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(w);
    Q_UNUSED(h);
    Q_UNUSED(scale);
}
