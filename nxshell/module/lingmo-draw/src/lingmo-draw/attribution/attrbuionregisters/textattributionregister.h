// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTATTRIBUTIONREGISTER_H
#define TEXTATTRIBUTIONREGISTER_H
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QComboBox>

#include "iattributionregister.h"

class IAttributionRegister;
class ComboBoxSettingWgt;
class SliderSpinBoxWidget;
class TextItem;
class ColorStyleWidget;

class TextAttributionRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
    QList<QWidget *> getAttriWidgets();
private:
    void initFontFamilyWidget(QComboBox *fontHeavy);
    void initFontWeightWidget();
    void reInitFontWeightComboxItems(const QString &family, QComboBox *fontHeavy);
    void initFontFontSizeWidget();
    bool isTextEnableUndoThisTime(bool considerRecorderEmpty = true);
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,                                        const QModelIndex &index) const;
    virtual bool eventFilter(QObject *o, QEvent *event)override;

    void resetItemsFontFamily();
    void cachedItemsFontFamily();
    void restoreItemsFontFamily();

private:
    QComboBox *m_fontComBox = nullptr;
    QComboBox *m_fontHeavy = nullptr;
    QComboBox *m_fontSize = nullptr;

    bool      _activePackup = false;
    bool      _fontViewShowOut = false;
    QString   _cachedFontWeightStyle;

    QStringList supWeightStyleList;

    struct Font {
        QString fontFamily;
        QString FontWeight;
    };

    QMap<TextItem *, Font> _cachedItemsFontFamily;
    QString     _cachedFontFamily;

    int        _currenFontSize = 14;
    QList<int> _defaultFontSizeSet;

    ComboBoxSettingWgt *m_fontAttri = nullptr;
    SliderSpinBoxWidget *m_fontSizeAttri = nullptr;
    ComboBoxSettingWgt *m_fontWeightAttri = nullptr;
    ColorStyleWidget *m_fontColorAttri = nullptr;
};


class QComboxMenuDelegate: public QAbstractItemDelegate
{
    Q_OBJECT
public:
    QComboxMenuDelegate(QObject *parent, QComboBox *cmb)
        : QAbstractItemDelegate(parent), mCombo(cmb), pressedIndex(-1)
    {}

    void dontShowCheckState(bool b) {showCheckStates = !b;}

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        if (!showCheckStates)
            opt.checked = false;
        painter->fillRect(option.rect, opt.palette.window());
        mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter, mCombo);
    }
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        return mCombo->style()->sizeFromContents(
                   QStyle::CT_MenuItem, &opt, option.rect.size(), mCombo);
    }
//    bool editorEvent(QEvent *event, QAbstractItemModel *model,
//                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const;
    QComboBox *mCombo;
    int pressedIndex;
    bool showCheckStates = false;
};
#endif // TEXTATTRIBUTIONREGISTER_H
