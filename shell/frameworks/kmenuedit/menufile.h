/*
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only
 *
 */

#ifndef menufile_h
#define menufile_h

#include <qdom.h>

#include <QList>
#include <QStringList>

class MenuFile
{
public:
    static const QString MF_MENU, MF_PUBLIC_ID, MF_SYSTEM_ID, MF_NAME, MF_INCLUDE, MF_EXCLUDE, MF_FILENAME, MF_DELETED, MF_NOTDELETED, MF_MOVE, MF_OLD, MF_NEW,
        MF_DIRECTORY, MF_LAYOUT, MF_MENUNAME, MF_SEPARATOR, MF_MERGE;

    explicit MenuFile(const QString &file);
    ~MenuFile();

    bool load();
    bool save();
    void create();
    QString error() const
    {
        return m_error;
    } // Returns the last error message

    void restoreMenuSystem(const QString &);

    enum ActionType {
        ADD_ENTRY = 0,
        REMOVE_ENTRY,
        ADD_MENU,
        REMOVE_MENU,
        MOVE_MENU,
    };

    struct ActionAtom {
        ActionType action;
        QString arg1;
        QString arg2;
    };

    /**
     * Create action atom and push it on the stack
     */
    ActionAtom *pushAction(ActionType action, const QString &arg1, const QString &arg2);

    /**
     * Pop @p atom from the stack.
     * @p atom must be last item on the stack
     */
    void popAction(ActionAtom *atom);

    /**
     * Perform the specified action
     */
    void performAction(const ActionAtom *);

    /**
     * Perform all actions currently on the stack, remove them from the stack and
     * save result
     * @return whether save was successful
     */
    bool performAllActions();

    /**
     * Returns whether the stack contains any actions
     */
    bool dirty() const;

    void addEntry(const QString &menuName, const QString &menuId);
    void removeEntry(const QString &menuName, const QString &menuId);

    void addMenu(const QString &menuName, const QString &menuFile);
    void moveMenu(const QString &oldMenu, const QString &newMenu);
    void removeMenu(const QString &menuName);

    void setLayout(const QString &menuName, const QStringList &layout);

    /**
     * Returns a unique menu-name for a new menu under @p menuName
     * inspired by @p newMenu and not part of @p excludeList
     */
    QString uniqueMenuName(const QString &menuName, const QString &newMenu, const QStringList &excludeList);

protected:
    /**
     * Finds menu @p menuName in @p elem.
     * If @p create is true, the menu is created if it doesn't exist yet.
     * @return The menu dom-node of @p menuName
     */
    QDomElement findMenu(QDomElement elem, const QString &menuName, bool create);

private:
    QString m_error;
    QString m_fileName;

    QDomDocument m_doc;
    bool m_bDirty;

    QList<ActionAtom *> m_actionList;
    QStringList m_removedEntries;
};

#endif
