#include "qhotkey.h"
#include "qhotkey_p.h"
#include "xdgshortcut.h"

#include <QAction>
#include <QCoreApplication>
#include <cmath>
#include <kglobalaccel.h>
#include <qcoreapplication.h>
#include <qkeysequence.h>
#include <qloggingcategory.h>
#include <qnamespace.h>
#include <unordered_map>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QGuiApplication>
#else
#include <QDebug>
#include <QX11Info>
#endif

// KWindowSystem
#include <KWindowSystem>
// KDE Shortcut support
#include <KGlobalAccel>

// MOC generated headers
#include "kglobalaccel_component_interface.h"
#include "kglobalaccel_interface.h"

#include <QThreadStorage>
#include <QTimer>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

// compatibility to pre Qt 5.8
#ifndef Q_FALLTHROUGH
#define Q_FALLTHROUGH() (void)0
#endif

Q_LOGGING_CATEGORY(logQHotkey_Linux, "QHotkey-Linux")

// For DBus

/// sa{sv}
///("org.example.app", {"description": "xxx", "trigger_description": "Ctrl, Shift, A"})
using Shortcut = QPair<QString, QVariantMap>;

/// a(sa{sv})
using Shortcuts = QList<Shortcut>;

Q_DECLARE_METATYPE(Shortcuts)

// Definitions for KWin KGlobalAccel Interface
#define KGlobalAccel_BUS_NAME "org.kde.KWin"
#define KGlobalAccel_OBJECT_PATH "/kglobalaccel"
#define KGlobalAccel_INTERFACE "org.kde.KGlobalAccel"

class KGlobalAccelInterface;
class KGlobalAccelComponentInterface;

class QHotkeyPrivateLinux : public QHotkeyPrivate {
public:
    QHotkeyPrivateLinux();
    ~QHotkeyPrivateLinux();
    // QAbstractNativeEventFilter interface
    bool nativeEventFilter(const QByteArray& eventType, void* message, _NATIVE_EVENT_RESULT* result) override;

    QString componentName() const
    {
        return m_appId + m_token;
    }

protected:
    // QHotkeyPrivate interface
    quint32 nativeKeycode(Qt::Key keycode, bool& ok) Q_DECL_OVERRIDE;
    quint32 nativeModifiers(Qt::KeyboardModifiers modifiers, bool& ok) Q_DECL_OVERRIDE;
    static QString getX11String(Qt::Key keycode);
    bool registerShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;
    bool unregisterShortcut(QHotkey::NativeShortcut shortcut) Q_DECL_OVERRIDE;

private:
    static const QVector<quint32> specialModifiers;
    static const quint32 validModsMask;
    xcb_key_press_event_t prevHandledEvent;
    xcb_key_press_event_t prevEvent;

    bool isX11;
    bool isWayland;

    // Used by KGlobalAccel
    const QString m_token;
    // This appid is used by KDE to identify the application that registers the shortcuts
    // Therefore it is important that it is unique and constant for each application
    const QString m_appId;

    // For KDE KGlobalAccel
    std::unordered_map<QString, QHotkey::NativeShortcut> m_registerdShortcutMapping;
    std::unordered_map<QString, std::unique_ptr<QAction>> m_shortcuts;
    KGlobalAccelInterface* const m_globalAccelInterface;
    KGlobalAccelComponentInterface* const m_component;

    void loadActionsFromAccel();

    void setActionsInAccel(const Shortcuts& shortcuts);

    QString getShorctIdentifier(const QString& shorctStr);

    // For X11
    static QString formatX11Error(Display* display, int errorCode);

    class HotkeyErrorHandler {
    public:
        HotkeyErrorHandler();
        ~HotkeyErrorHandler();

        static bool hasError;
        static QString errorString;

    private:
        XErrorHandler prevHandler;

        static int handleError(Display* display, XErrorEvent* error);
    };
};
NATIVE_INSTANCE(QHotkeyPrivateLinux)

bool QHotkeyPrivate::isPlatformSupported()
{
    if (KWindowSystem::isPlatformX11()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        return qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
#else
        return QX11Info::isPlatformX11();
#endif
    } else {
        return KWindowSystem::isPlatformWayland();
    }
}

const QVector<quint32> QHotkeyPrivateLinux::specialModifiers = { 0, Mod2Mask, LockMask, (Mod2Mask | LockMask) };
const quint32 QHotkeyPrivateLinux::validModsMask = ShiftMask | ControlMask | Mod1Mask | Mod4Mask;

QHotkeyPrivateLinux::~QHotkeyPrivateLinux()
{
    if (isWayland) {
        qCDebug(logQHotkey_Linux) << "Unregistering shortcuts";
        // We can forget the shortcuts that aren't around anymore
        const QList<KGlobalShortcutInfo> shortcutInfos = m_component->allShortcutInfos();
        QHash<QString, KGlobalShortcutInfo> shortcutInfosByName;
        shortcutInfosByName.reserve(shortcutInfos.size());
        for (const auto& shortcutInfo : shortcutInfos) {
            shortcutInfosByName[shortcutInfo.uniqueName()] = shortcutInfo;
        }
        while (!shortcutInfosByName.isEmpty()) {
            const QString shortcutName = shortcutInfosByName.begin().key();
            auto it = m_shortcuts.find(shortcutName);
            if (it != m_shortcuts.end()) {
                KGlobalAccel::self()->removeAllShortcuts(it->second.get());
                m_shortcuts.erase(it);
            }
            shortcutInfosByName.erase(shortcutInfosByName.begin());
        }
    }
}

QHotkeyPrivateLinux::QHotkeyPrivateLinux()
    : QHotkeyPrivate()
    , isX11(KWindowSystem::isPlatformX11())
    , isWayland(KWindowSystem::isPlatformWayland())
    , m_token("/org/lingmoui/ShortcutService/" + QCoreApplication::applicationFilePath())
    , m_appId("org.lingmoui.ShortcutService.ThirdParty." + QCoreApplication::organizationDomain() + QCoreApplication::applicationName())
    , m_globalAccelInterface(
          new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus(), this))
    , m_component(new KGlobalAccelComponentInterface(m_globalAccelInterface->service(),
          m_globalAccelInterface->getComponent(componentName()).value().path(),
          m_globalAccelInterface->connection(),
          this))
{
    qCDebug(logQHotkey_Linux) << "Called by " << QCoreApplication::applicationFilePath();
    qCDebug(logQHotkey_Linux) << "appID:" << m_appId;
    if (isWayland) {
        qCDebug(logQHotkey_Linux) << "Wayland detected";
        qDBusRegisterMetaType<KGlobalShortcutInfo>();
        qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
        qDBusRegisterMetaType<QKeySequence>();
        qDBusRegisterMetaType<QList<QKeySequence>>();

        connect(m_globalAccelInterface,
            &KGlobalAccelInterface::yourShortcutsChanged,
            this,
            [this](const QStringList& actionId, const QList<QKeySequence>& newKeys) {
                Q_UNUSED(newKeys);
                if (actionId[KGlobalAccel::ComponentUnique] == componentName()) {
                    if (auto it = m_shortcuts.find(actionId[KGlobalAccel::ActionUnique]); it != m_shortcuts.end()) {
                        it->second->setShortcuts(newKeys);
                        qCInfo(logQHotkey_Linux) << "Shortcut " << actionId[KGlobalAccel::ActionUnique] << " to " << newKeys;
                    }
                }
            });
        connect(m_component,
            &KGlobalAccelComponentInterface::globalShortcutPressed,
            this,
            [this](const QString& componentUnique, const QString& actionUnique, qlonglong timestamp) {
                if (componentUnique != componentName()) {
                    return;
                }
                if (auto it = m_registerdShortcutMapping.find(actionUnique); it != m_registerdShortcutMapping.end()) {
                    this->activateShortcut(it->second);
                }
            });
        connect(m_component,
            &KGlobalAccelComponentInterface::globalShortcutReleased,
            this,
            [this](const QString& componentUnique, const QString& actionUnique, qlonglong timestamp) {
                if (componentUnique != componentName()) {
                    return;
                }
                if (auto it = m_registerdShortcutMapping.find(actionUnique); it != m_registerdShortcutMapping.end()) {
                    this->releaseShortcut(it->second);
                }
            });
        // Initialize global shortcuts
        loadActionsFromAccel();
    }
}

void QHotkeyPrivateLinux::loadActionsFromAccel()
{
    m_shortcuts.clear();

    const QList<KGlobalShortcutInfo> shortcutInfos = m_component->allShortcutInfos();
    QHash<QString, KGlobalShortcutInfo> shortcutInfosByName;
    shortcutInfosByName.reserve(shortcutInfos.size());
    for (const auto& shortcutInfo : shortcutInfos) {
        shortcutInfosByName[shortcutInfo.uniqueName()] = shortcutInfo;
    }

    for (const auto& [name, info] : shortcutInfosByName.asKeyValueRange()) {
        std::unique_ptr<QAction>& action = m_shortcuts[name];
        if (!action) {
            action = std::make_unique<QAction>();
        }
        action->setProperty("componentName", componentName());
        action->setProperty("componentDisplayName", componentName());
        action->setObjectName(name);
        action->setText(info.friendlyName());
        action->setShortcuts(info.keys());
        // Explicitly load existing global shortcut setting
        KGlobalAccel::self()->setShortcut(action.get(), action->shortcuts(), KGlobalAccel::Autoloading);
    }
}

bool QHotkeyPrivateLinux::nativeEventFilter(const QByteArray& eventType, void* message, _NATIVE_EVENT_RESULT* result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    if (isX11) {
        auto* genericEvent = static_cast<xcb_generic_event_t*>(message);
        if (genericEvent->response_type == XCB_KEY_PRESS) {
            xcb_key_press_event_t keyEvent = *static_cast<xcb_key_press_event_t*>(message);
            this->prevEvent = keyEvent;
            if (this->prevHandledEvent.response_type == XCB_KEY_RELEASE) {
                if (this->prevHandledEvent.time == keyEvent.time)
                    return false;
            }
            this->activateShortcut({ keyEvent.detail, keyEvent.state & QHotkeyPrivateLinux::validModsMask });
        } else if (genericEvent->response_type == XCB_KEY_RELEASE) {
            xcb_key_release_event_t keyEvent = *static_cast<xcb_key_release_event_t*>(message);
            this->prevEvent = keyEvent;
            QTimer::singleShot(50, [this, keyEvent] {
                if (this->prevEvent.time == keyEvent.time && this->prevEvent.response_type == keyEvent.response_type && this->prevEvent.detail == keyEvent.detail) {
                    this->releaseShortcut({ keyEvent.detail, keyEvent.state & QHotkeyPrivateLinux::validModsMask });
                }
            });
            this->prevHandledEvent = keyEvent;
        }
    }
    return false;
}

QString QHotkeyPrivateLinux::getX11String(Qt::Key keycode)
{
    switch (keycode) {

    case Qt::Key_MediaLast:
    case Qt::Key_MediaPrevious:
        return QStringLiteral("XF86AudioPrev");
    case Qt::Key_MediaNext:
        return QStringLiteral("XF86AudioNext");
    case Qt::Key_MediaPause:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaTogglePlayPause:
        return QStringLiteral("XF86AudioPlay");
    case Qt::Key_MediaRecord:
        return QStringLiteral("XF86AudioRecord");
    case Qt::Key_MediaStop:
        return QStringLiteral("XF86AudioStop");
    default:
        return QKeySequence(keycode).toString(QKeySequence::NativeText);
    }
}

quint32 QHotkeyPrivateLinux::nativeKeycode(Qt::Key keycode, bool& ok)
{
    if (isX11) {
        QString keyString = getX11String(keycode);

        KeySym keysym = XStringToKeysym(keyString.toLatin1().constData());
        if (keysym == NoSymbol) {
            // not found -> just use the key
            if (keycode <= 0xFFFF)
                keysym = keycode;
            else
                return 0;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        const QNativeInterface::QX11Application* x11Interface = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
#else
        const bool x11Interface = QX11Info::isPlatformX11();
#endif

        if (x11Interface) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
            Display* display = x11Interface->display();
#else
            Display* display = QX11Info::display();
#endif
            auto res = XKeysymToKeycode(display, keysym);
            if (res != 0)
                ok = true;
            return res;
        }
    }

    if (isWayland) {
        // Just return the keycode is okay
        ok = true;
        return keycode;
    }

    return 0;
}

quint32 QHotkeyPrivateLinux::nativeModifiers(Qt::KeyboardModifiers modifiers, bool& ok)
{
    quint32 nMods = 0;
    if (modifiers & Qt::ShiftModifier)
        nMods |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        nMods |= ControlMask;
    if (modifiers & Qt::AltModifier)
        nMods |= Mod1Mask;
    if (modifiers & Qt::MetaModifier)
        nMods |= Mod4Mask;
    ok = true;
    return nMods;
}

QString QHotkeyPrivateLinux::getShorctIdentifier(const QString& shorctStr)
{
    return m_appId + "." + shorctStr.toLower();
}

void QHotkeyPrivateLinux::setActionsInAccel(const Shortcuts& shortcuts)
{
    // m_shortcuts.clear();

    const QList<KGlobalShortcutInfo> shortcutInfos = m_component->allShortcutInfos();
    QHash<QString, KGlobalShortcutInfo> shortcutInfosByName;
    shortcutInfosByName.reserve(shortcutInfos.size());
    for (const auto& shortcutInfo : shortcutInfos) {
        shortcutInfosByName[shortcutInfo.uniqueName()] = shortcutInfo;
    }

    for (const auto& shortcut : shortcuts) {
        qCDebug(logQHotkey_Linux) << "Shortcut id: " << shortcut.first << "description:" << shortcut.second["description"].toString() << "preferred_trigger: " << shortcut.second["preferred_trigger"].toString();

        const QString description = shortcut.second["description"].toString();
        if (description.isEmpty() || shortcut.first.isEmpty()) {
            qCWarning(logQHotkey_Linux) << "Shortcut without name or description" << shortcut.first << "for" << componentName();
            continue;
        }

        std::unique_ptr<QAction>& action = m_shortcuts[shortcut.first];
        if (!action) {
            action = std::make_unique<QAction>();
        }
        action->setProperty("componentName", componentName());
        action->setProperty("componentDisplayName", componentName());
        action->setObjectName(shortcut.first);
        action->setText(description);
        const auto itShortcut = shortcutInfosByName.constFind(shortcut.first);
        if (itShortcut != shortcutInfosByName.constEnd()) {
            if (!itShortcut->keys().isEmpty()) {
                action->setShortcuts(itShortcut->keys());
            } else {
                qCDebug(logQHotkey_Linux) << "No previusly defined shortcuts found for" << shortcut.first;
                const auto preferredShortcut = XdgShortcut::parse(shortcut.second["preferred_trigger"].toString().toUpper());
                if (preferredShortcut) {
                    action->setShortcut(preferredShortcut.value());
                }
            }
        } else {
            const auto preferredShortcut = XdgShortcut::parse(shortcut.second["preferred_trigger"].toString().toUpper());
            if (preferredShortcut) {
                action->setShortcut(preferredShortcut.value());
            }
        }
        KGlobalAccel::self()->setGlobalShortcut(action.get(), action->shortcuts());

        shortcutInfosByName.remove(shortcut.first);
    }
}

bool QHotkeyPrivateLinux::registerShortcut(QHotkey::NativeShortcut shortcut)
{
    if (isX11) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        const QNativeInterface::QX11Application* x11Interface = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
        Display* display = x11Interface->display();
#else
        const bool x11Interface = QX11Info::isPlatformX11();
        Display* display = QX11Info::display();
#endif

        if (!display || !x11Interface)
            return false;

        HotkeyErrorHandler errorHandler;
        for (quint32 specialMod : QHotkeyPrivateLinux::specialModifiers) {
            XGrabKey(display,
                shortcut.key,
                shortcut.modifier | specialMod,
                DefaultRootWindow(display),
                True,
                GrabModeAsync,
                GrabModeAsync);
        }
        XSync(display, False);

        if (errorHandler.hasError) {
            error = errorHandler.errorString;
            this->unregisterShortcut(shortcut);
            return false;
        }
        return true;
    }

    if (isWayland) {
        // Convert the NativeKeycode back into a QKeySequence
        auto key = static_cast<Qt::Key>(shortcut.key);
        // Convert the modifier to a Qt::Keysequence
        Qt::KeyboardModifiers nMods = {};
        if (shortcut.modifier & ShiftMask)
            nMods |= Qt::ShiftModifier;
        if (shortcut.modifier & ControlMask)
            nMods |= Qt::ControlModifier;
        if (shortcut.modifier & Mod1Mask)
            nMods |= Qt::AltModifier;
        if (shortcut.modifier & Mod4Mask)
            nMods |= Qt::MetaModifier;

        // Convert into user readable format
        QKeySequence keySequence = QKeySequence(key | nMods);
        QString combination_description = keySequence.toString(QKeySequence::NativeText);

        qCDebug(logQHotkey_Linux) << "Registering: " << combination_description;

        Shortcut _converted_shortcut = {
            getShorctIdentifier(combination_description),
            {
                { "description", combination_description + " by " + QCoreApplication::applicationName() },
                { "preferred_trigger", combination_description },
            }
        };

        // Check if the required shortcut is available for us
        bool is_available = KGlobalAccel::self()->isGlobalShortcutAvailable(keySequence);
        if (is_available) {
            m_registerdShortcutMapping.insert({ getShorctIdentifier(combination_description), shortcut });
            setActionsInAccel({ _converted_shortcut });
            return true;
        } else {
            // Raise error
            error = "The shortcut " + combination_description + " is already in use by another application.";
            return false;
        }
    }

    return false;
}

bool QHotkeyPrivateLinux::unregisterShortcut(QHotkey::NativeShortcut shortcut)
{
    if (isX11) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        Display* display = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display();
#else
        Display* display = QX11Info::display();
#endif

        if (!display)
            return false;

        HotkeyErrorHandler errorHandler;
        for (quint32 specialMod : QHotkeyPrivateLinux::specialModifiers) {
            XUngrabKey(display,
                shortcut.key,
                shortcut.modifier | specialMod,
                XDefaultRootWindow(display));
        }
        XSync(display, False);

        if (HotkeyErrorHandler::hasError) {
            error = HotkeyErrorHandler::errorString;
            return false;
        }
        return true;
    }

    if (isWayland) {
        // Convert the NativeKeycode back into a QKeySequence
        auto key = static_cast<Qt::Key>(shortcut.key);
        // Convert the modifier to a Qt::Keysequence
        Qt::KeyboardModifiers nMods = {};
        if (shortcut.modifier & ShiftMask)
            nMods |= Qt::ShiftModifier;
        if (shortcut.modifier & ControlMask)
            nMods |= Qt::ControlModifier;
        if (shortcut.modifier & Mod1Mask)
            nMods |= Qt::AltModifier;
        if (shortcut.modifier & Mod4Mask)
            nMods |= Qt::MetaModifier;

        // Convert into user readable format
        QKeySequence keySequence = QKeySequence(key | nMods);
        QString combination_description = keySequence.toString(QKeySequence::NativeText);

        QString identifier = getShorctIdentifier(combination_description);

        if (auto it = m_shortcuts.find(identifier); it != m_shortcuts.end()) {
            KGlobalAccel::self()->removeAllShortcuts(it->second.get());
            m_shortcuts.erase(it);
        }

        m_registerdShortcutMapping.erase(identifier);

        return true;
    }

    return false;
}

QString QHotkeyPrivateLinux::formatX11Error(Display* display, int errorCode)
{
    char errStr[256];
    XGetErrorText(display, errorCode, errStr, 256);
    return QString::fromLatin1(errStr);
}

// ---------- HotkeyErrorHandler::HotkeyErrorHandler implementation ----------

bool QHotkeyPrivateLinux::HotkeyErrorHandler::hasError = false;
QString QHotkeyPrivateLinux::HotkeyErrorHandler::errorString;

QHotkeyPrivateLinux::HotkeyErrorHandler::HotkeyErrorHandler()
{
    prevHandler = XSetErrorHandler(&HotkeyErrorHandler::handleError);
}

QHotkeyPrivateLinux::HotkeyErrorHandler::~HotkeyErrorHandler()
{
    XSetErrorHandler(prevHandler);
    hasError = false;
    errorString.clear();
}

int QHotkeyPrivateLinux::HotkeyErrorHandler::handleError(Display* display, XErrorEvent* error)
{
    switch (error->error_code) {
    case BadAccess:
    case BadValue:
    case BadWindow:
        if (error->request_code == 33 || // grab key
            error->request_code == 34) { // ungrab key
            hasError = true;
            errorString = QHotkeyPrivateLinux::formatX11Error(display, error->error_code);
            return 1;
        }
        Q_FALLTHROUGH();
        // fall through
    default:
        return 0;
    }
}
