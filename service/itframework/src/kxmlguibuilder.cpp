/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguibuilder.h"

#include "debug.h"
#include "kmainwindow.h"
#include "kmenumenuhandler_p.h"
#include "ktoolbar.h"
#include "kxmlguiclient.h"
#include "kxmlguiwindow.h"

#include <KAuthorized>
#include <KLocalizedString>

#include <QAction>
#include <QDomElement>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QStatusBar>

using namespace KDEPrivate;

class KXMLGUIBuilderPrivate
{
public:
    KXMLGUIBuilderPrivate()
    {
    }
    ~KXMLGUIBuilderPrivate()
    {
    }

    QWidget *m_widget = nullptr;

    QString tagMainWindow;
    QString tagMenuBar;
    QString tagMenu;
    QString tagToolBar;
    QString tagStatusBar;

    QString tagSeparator;
    QString tagSpacer;
    QString tagTearOffHandle;
    QString tagMenuTitle;

    QString attrName;
    QString attrLineSeparator;

    QString attrDomain;
    QString attrText1;
    QString attrText2;
    QString attrContext;

    QString attrIcon;

    KXMLGUIClient *m_client = nullptr;

    KMenuMenuHandler *m_menumenuhandler = nullptr;
};

KXMLGUIBuilder::KXMLGUIBuilder(QWidget *widget)
    : d(new KXMLGUIBuilderPrivate)
{
    d->m_widget = widget;

    d->tagMainWindow = QStringLiteral("mainwindow");
    d->tagMenuBar = QStringLiteral("menubar");
    d->tagMenu = QStringLiteral("menu");
    d->tagToolBar = QStringLiteral("toolbar");
    d->tagStatusBar = QStringLiteral("statusbar");

    d->tagSeparator = QStringLiteral("separator");
    d->tagSpacer = QStringLiteral("spacer");
    d->tagTearOffHandle = QStringLiteral("tearoffhandle");
    d->tagMenuTitle = QStringLiteral("title");

    d->attrName = QStringLiteral("name");
    d->attrLineSeparator = QStringLiteral("lineseparator");

    d->attrDomain = QStringLiteral("translationDomain");
    d->attrText1 = QStringLiteral("text");
    d->attrText2 = QStringLiteral("Text");
    d->attrContext = QStringLiteral("context");

    d->attrIcon = QStringLiteral("icon");

    d->m_menumenuhandler = new KMenuMenuHandler(this);
}

KXMLGUIBuilder::~KXMLGUIBuilder()
{
    delete d->m_menumenuhandler;
}

QWidget *KXMLGUIBuilder::widget()
{
    return d->m_widget;
}

QStringList KXMLGUIBuilder::containerTags() const
{
    QStringList res;
    res << d->tagMenu << d->tagToolBar << d->tagMainWindow << d->tagMenuBar << d->tagStatusBar;

    return res;
}

QWidget *KXMLGUIBuilder::createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction)
{
    containerAction = nullptr;

    if (element.attribute(QStringLiteral("deleted")).toLower() == QLatin1String("true")) {
        return nullptr;
    }

    const QString tagName = element.tagName().toLower();
    if (tagName == d->tagMainWindow) {
        KMainWindow *mainwindow = qobject_cast<KMainWindow *>(d->m_widget); // could be 0
        return mainwindow;
    }

    if (tagName == d->tagMenuBar) {
        KMainWindow *mainWin = qobject_cast<KMainWindow *>(d->m_widget);
        QMenuBar *bar = nullptr;
        if (mainWin) {
            bar = mainWin->menuBar();
        }
        if (!bar) {
            bar = new QMenuBar(d->m_widget);
        }
        bar->show();
        return bar;
    }

    if (tagName == d->tagMenu) {
        // Look up to see if we are inside a mainwindow. If yes, then
        // use it as parent widget (to get kaction to plug itself into the
        // mainwindow). Don't use a popupmenu as parent widget, otherwise
        // the popup won't be hidden if it is used as a standalone menu as well.
        // Note: menus with a parent of 0, coming from child clients, can be
        // leaked if the child client is deleted without a proper removeClient call, though.
        QWidget *p = parent;

        if (!p && qobject_cast<QMainWindow *>(d->m_widget)) {
            p = d->m_widget;
        }

        while (p && !qobject_cast<QMainWindow *>(p)) {
            p = p->parentWidget();
        }

        QString name = element.attribute(d->attrName);

        if (!KAuthorized::authorizeAction(name)) {
            return nullptr;
        }

        QMenu *popup = new QMenu(p);
        popup->setObjectName(name);

        d->m_menumenuhandler->insertMenu(popup);

        QString i18nText;
        QDomElement textElem = element.namedItem(d->attrText1).toElement();
        if (textElem.isNull()) { // try with capital T
            textElem = element.namedItem(d->attrText2).toElement();
        }
        const QString text = textElem.text();
        const QString context = textElem.attribute(d->attrContext);

        // qCDebug(DEBUG_KXMLGUI) << "DOMAIN" << KLocalizedString::applicationDomain();
        // qCDebug(DEBUG_KXMLGUI) << "ELEMENT TEXT:" << text;

        if (text.isEmpty()) { // still no luck
            i18nText = i18n("No text");
        } else {
            QByteArray domain = textElem.attribute(d->attrDomain).toUtf8();
            if (domain.isEmpty()) {
                domain = element.ownerDocument().documentElement().attribute(d->attrDomain).toUtf8();
                if (domain.isEmpty()) {
                    domain = KLocalizedString::applicationDomain();
                }
            }
            if (context.isEmpty()) {
                i18nText = i18nd(domain.constData(), text.toUtf8().constData());
            } else {
                i18nText = i18ndc(domain.constData(), context.toUtf8().constData(), text.toUtf8().constData());
            }
        }

        // qCDebug(DEBUG_KXMLGUI) << "ELEMENT i18n TEXT:" << i18nText;

        const QString icon = element.attribute(d->attrIcon);
        QIcon pix;
        if (!icon.isEmpty()) {
            pix = QIcon::fromTheme(icon);
        }

        if (parent) {
            QAction *act = popup->menuAction();
            if (!icon.isEmpty()) {
                act->setIcon(pix);
            }
            act->setText(i18nText);
            if (index == -1 || index >= parent->actions().count()) {
                parent->addAction(act);
            } else {
                parent->insertAction(parent->actions().value(index), act);
            }
            containerAction = act;
            containerAction->setObjectName(name);
        }

        return popup;
    }

    if (tagName == d->tagToolBar) {
        QString name = element.attribute(d->attrName);

        KToolBar *bar = static_cast<KToolBar *>(d->m_widget->findChild<KToolBar *>(name));
        if (!bar) {
            bar = new KToolBar(name, d->m_widget, false);
        }

        if (qobject_cast<KMainWindow *>(d->m_widget)) {
            if (d->m_client && !d->m_client->xmlFile().isEmpty()) {
                bar->addXMLGUIClient(d->m_client);
            }
        }
        if (!bar->mainWindow()) {
            bar->show();
        }

        bar->loadState(element);

        return bar;
    }

    if (tagName == d->tagStatusBar) {
        KMainWindow *mainWin = qobject_cast<KMainWindow *>(d->m_widget);
        if (mainWin) {
            mainWin->statusBar()->show();
            return mainWin->statusBar();
        }
        QStatusBar *bar = new QStatusBar(d->m_widget);
        return bar;
    }

    return nullptr;
}

void KXMLGUIBuilder::removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction)
{
    // Warning parent can be 0L

    if (qobject_cast<QMenu *>(container)) {
        if (parent) {
            parent->removeAction(containerAction);
        }

        delete container;
    } else if (qobject_cast<KToolBar *>(container)) {
        KToolBar *tb = static_cast<KToolBar *>(container);

        tb->saveState(element);
        if (tb->mainWindow()) {
            delete tb;
        } else {
            tb->clear();
            tb->hide();
        }
    } else if (qobject_cast<QMenuBar *>(container)) {
        QMenuBar *mb = static_cast<QMenuBar *>(container);
        mb->hide();
        // Don't delete menubar - it can be reused by createContainer.
        // If you decide that you do need to delete the menubar, make
        // sure that QMainWindow::d->mb does not point to a deleted
        // menubar object.
    } else if (qobject_cast<QStatusBar *>(container)) {
        if (qobject_cast<KMainWindow *>(d->m_widget)) {
            container->hide();
        } else {
            delete static_cast<QStatusBar *>(container);
        }
    } else {
        qCWarning(DEBUG_KXMLGUI) << "Unhandled container to remove : " << container->metaObject()->className();
    }
}

QStringList KXMLGUIBuilder::customTags() const
{
    QStringList res;
    res << d->tagSeparator << d->tagSpacer << d->tagTearOffHandle << d->tagMenuTitle;
    return res;
}

QAction *KXMLGUIBuilder::createCustomElement(QWidget *parent, int index, const QDomElement &element)
{
    QAction *before = nullptr;
    if (index > 0 && index < parent->actions().count()) {
        before = parent->actions().at(index);
    }

    const QString tagName = element.tagName().toLower();
    if (tagName == d->tagSeparator) {
        if (QMenu *menu = qobject_cast<QMenu *>(parent)) {
            // QMenu already cares for leading/trailing/repeated separators
            // no need to check anything
            return menu->insertSeparator(before);
        } else if (QMenuBar *bar = qobject_cast<QMenuBar *>(parent)) {
            QAction *separatorAction = new QAction(bar);
            separatorAction->setSeparator(true);
            bar->insertAction(before, separatorAction);
            return separatorAction;
        } else if (KToolBar *bar = qobject_cast<KToolBar *>(parent)) {
            /* FIXME KAction port - any need to provide a replacement for lineSeparator/normal separator?
            bool isLineSep = true;

            QDomNamedNodeMap attributes = element.attributes();
            unsigned int i = 0;
            for (; i < attributes.length(); i++ )
            {
              QDomAttr attr = attributes.item( i ).toAttr();

              if ( attr.name().toLower() == d->attrLineSeparator &&
                   attr.value().toLower() == QLatin1String("false") )
              {
                isLineSep = false;
                break;
              }
            }

            if ( isLineSep )
                return bar->insertSeparator( index ? bar->actions()[index - 1] : 0L );
            else*/

            return bar->insertSeparator(before);
        }
    } else if (tagName == d->tagSpacer) {
        if (QToolBar *bar = qobject_cast<QToolBar *>(parent)) {
            // Create the simple spacer widget
            QWidget *spacer = new QWidget(parent);
            spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            return bar->insertWidget(before, spacer);
        }
    } else if (tagName == d->tagTearOffHandle) {
        static_cast<QMenu *>(parent)->setTearOffEnabled(true);
    } else if (tagName == d->tagMenuTitle) {
        if (QMenu *m = qobject_cast<QMenu *>(parent)) {
            QString i18nText;
            const QString text = element.text();

            if (text.isEmpty()) {
                i18nText = i18n("No text");
            } else {
                QByteArray domain = element.attribute(d->attrDomain).toUtf8();
                if (domain.isEmpty()) {
                    domain = element.ownerDocument().documentElement().attribute(d->attrDomain).toUtf8();
                    if (domain.isEmpty()) {
                        domain = KLocalizedString::applicationDomain();
                    }
                }
                i18nText = i18nd(domain.constData(), qPrintable(text));
            }

            QString icon = element.attribute(d->attrIcon);
            QIcon pix;

            if (!icon.isEmpty()) {
                pix = QIcon::fromTheme(icon);
            }

            if (!icon.isEmpty()) {
                return m->insertSection(before, pix, i18nText);
            } else {
                return m->insertSection(before, i18nText);
            }
        }
    }

    QAction *blank = new QAction(parent);
    blank->setVisible(false);
    parent->insertAction(before, blank);
    return blank;
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KXMLGUIBuilder::removeCustomElement(QWidget *parent, QAction *action)
{
    parent->removeAction(action);
}
#endif

KXMLGUIClient *KXMLGUIBuilder::builderClient() const
{
    return d->m_client;
}

void KXMLGUIBuilder::setBuilderClient(KXMLGUIClient *client)
{
    d->m_client = client;
}

void KXMLGUIBuilder::finalizeGUI(KXMLGUIClient *)
{
    KXmlGuiWindow *window = qobject_cast<KXmlGuiWindow *>(d->m_widget);
    if (window) {
        window->finalizeGUI(false);
    }
}

void KXMLGUIBuilder::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}
