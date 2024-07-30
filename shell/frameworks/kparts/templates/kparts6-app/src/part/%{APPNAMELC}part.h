/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef %{APPNAMEUC}PART_H
#define %{APPNAMEUC}PART_H

// KF headers
#include <KParts/ReadWritePart>

class QTextEdit;
class QWidget;
class QAction;
class QTextDocument;

/**
 * @short %{APPNAME} Part
 */
class %{APPNAME}Part : public KParts::ReadWritePart
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    %{APPNAME}Part(QWidget* parentWidget, QObject* parent,
                   const KPluginMetaData &metaData, const QVariantList& arg);

    /**
     * Destructor
     */
    ~%{APPNAME}Part() override;

    /**
     * Reimplemented to update the internal UI
     */
    void setReadWrite(bool rw) override;

    /**
     * Reimplemented to disable and enable Save action
     */
    void setModified(bool modified) override;

protected: // KParts::ReadWritePart API
    bool openFile() override;
    bool saveFile() override;

private:
    void setupActions();

private Q_SLOTS:
    void fileSave();
    void fileSaveAs();

private:
    QTextEdit* m_textEditWidget;
    QAction* m_saveAction;
    QTextDocument* m_textDocument;
};

#endif // %{APPNAMEUC}PART_H
