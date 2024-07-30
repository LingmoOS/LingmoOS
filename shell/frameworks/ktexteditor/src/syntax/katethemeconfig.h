/*
    SPDX-FileCopyrightText: 2001-2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCHEMA_CONFIG_H
#define KATE_SCHEMA_CONFIG_H

#include "kateconfigpage.h"
#include "kateextendedattribute.h"

#include <map>
#include <unordered_map>

class KateStyleTreeWidget;
class KateColorTreeWidget;
class KMessageWidget;
class KateColorItem;

namespace KTextEditor
{
class ViewPrivate;
class DocumentPrivate;
}

class QComboBox;

class KateThemeConfigColorTab : public QWidget
{
    Q_OBJECT

public:
    KateThemeConfigColorTab();

    QColor backgroundColor() const;
    QColor selectionColor() const;

public Q_SLOTS:
    void apply();
    void reload();
    void schemaChanged(const QString &newSchema);

Q_SIGNALS:
    void changed();

private:
    // multiple shemas may be edited. Hence, we need one ColorList for each schema
    std::map<QString, QList<KateColorItem>> m_schemas;
    QString m_currentSchema;
    KateColorTreeWidget *ui;
};

class KateThemeConfigDefaultStylesTab : public QWidget
{
    Q_OBJECT

public:
    explicit KateThemeConfigDefaultStylesTab(KateThemeConfigColorTab *colorTab);

Q_SIGNALS:
    void changed();

public:
    void schemaChanged(const QString &schema);
    void reload();
    void apply();

    KateAttributeList *attributeList(const QString &schema);

protected:
    void showEvent(QShowEvent *event) override;
    void updateColorPalette(const QColor &textColor);

private:
    KateStyleTreeWidget *m_defaultStyles;
    std::unordered_map<QString, KateAttributeList> m_defaultStyleLists;
    KateThemeConfigColorTab *m_colorTab;
    QString m_currentSchema;
};

class KateThemeConfigHighlightTab : public QWidget
{
    Q_OBJECT

public:
    explicit KateThemeConfigHighlightTab(KateThemeConfigDefaultStylesTab *page, KateThemeConfigColorTab *colorTab);

    void schemaChanged(const QString &schema);
    void reload();
    void apply();

Q_SIGNALS:
    void changed();

protected Q_SLOTS:
    void hlChanged(int z);

protected:
    void showEvent(QShowEvent *event) override;
    void updateColorPalette(const QColor &textColor);

private:
    KateThemeConfigDefaultStylesTab *m_defaults;
    KateThemeConfigColorTab *m_colorTab;

    QComboBox *hlCombo;
    KateStyleTreeWidget *m_styles;

    QString m_schema;
    int m_hl;

    QHash<QString, QHash<int, QList<KTextEditor::Attribute::Ptr>>> m_hlDict;

    /**
     * store attribute we modify
     * this unifies them to be unique (aka in all highlighting's the embedded stuff is shared!)
     *
     * theme => highlighting => attribute => pair of value + default
     */
    std::map<QString, std::map<QString, std::map<QString, std::pair<KTextEditor::Attribute::Ptr, KTextEditor::Attribute::Ptr>>>> m_uniqueAttributes;

public:
    QList<int> hlsForSchema(const QString &schema);
};

class KateThemeConfigPage : public KateConfigPage
{
public:
    explicit KateThemeConfigPage(QWidget *parent);
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public:
    void apply() override;
    void reload() override;
    void reset() override;
    void defaults() override;
    void exportFullSchema();
    void importFullSchema();

private:
    void layoutThemeChooserTab(QWidget *tab);
    void layoutThemeEditorTab(QWidget *tab);
    void deleteSchema();
    bool copyTheme();
    void schemaChanged(const QString &schema);
    void comboBoxIndexChanged(int currentIndex);

private:
    void refillCombos(const QString &schemaName, const QString &defaultSchemaName);

private:
    QString m_currentSchema;

    KMessageWidget *m_readOnlyThemeLabel = nullptr;
    class QPushButton *btndel;
    class QComboBox *defaultSchemaCombo;
    class QComboBox *schemaCombo;
    KateThemeConfigColorTab *m_colorTab;
    KateThemeConfigDefaultStylesTab *m_defaultStylesTab;
    KateThemeConfigHighlightTab *m_highlightTab;
    KTextEditor::DocumentPrivate *m_doc = nullptr;
    KTextEditor::ViewPrivate *m_themePreview = nullptr;
};

#endif
