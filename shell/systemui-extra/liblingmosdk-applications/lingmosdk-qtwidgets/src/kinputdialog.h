/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KINPUTDIALOG_H
#define KINPUTDIALOG_H

#include <QLineEdit>

#include <QtWidgets/qtwidgetsglobal.h>
#include "kdialog.h"

namespace kdk
{

/** @defgroup DialogBoxModule
  * @{
  */

class KInputDialogPrivate;

/**
 * @brief 输入对话框，接口与 QInputDialog 相同。
 */
class GUI_EXPORT KInputDialog : public KDialog
{
    Q_OBJECT
    Q_ENUMS(InputMode InputDialogOption)
    QDOC_PROPERTY(InputMode inputMode READ inputMode WRITE setInputMode)
    QDOC_PROPERTY(QString labelText READ labelText WRITE setLabelText)
    QDOC_PROPERTY(InputDialogOptions options READ options WRITE setOptions)
    QDOC_PROPERTY(QString textValue READ textValue WRITE setTextValue NOTIFY textValueChanged)
    QDOC_PROPERTY(int intValue READ intValue WRITE setIntValue NOTIFY intValueChanged)
    QDOC_PROPERTY(int doubleValue READ doubleValue WRITE setDoubleValue NOTIFY doubleValueChanged)
    QDOC_PROPERTY(QLineEdit::EchoMode textEchoMode READ textEchoMode WRITE setTextEchoMode)
    QDOC_PROPERTY(bool comboBoxEditable READ isComboBoxEditable WRITE setComboBoxEditable)
    QDOC_PROPERTY(QStringList comboBoxItems READ comboBoxItems WRITE setComboBoxItems)
    QDOC_PROPERTY(int intMinimum READ intMinimum WRITE setIntMinimum)
    QDOC_PROPERTY(int intMaximum READ intMaximum WRITE setIntMaximum)
    QDOC_PROPERTY(int intStep READ intStep WRITE setIntStep)
    QDOC_PROPERTY(double doubleMinimum READ doubleMinimum WRITE setDoubleMinimum)
    QDOC_PROPERTY(double doubleMaximum READ doubleMaximum WRITE setDoubleMaximum)
    QDOC_PROPERTY(int doubleDecimals READ doubleDecimals WRITE setDoubleDecimals)
    QDOC_PROPERTY(QString okButtonText READ okButtonText WRITE setOkButtonText)
    QDOC_PROPERTY(QString cancelButtonText READ cancelButtonText WRITE setCancelButtonText)
    QDOC_PROPERTY(double doubleStep READ doubleStep WRITE setDoubleStep)

public:
    enum InputDialogOption {
        NoButtons                    = 0x00000001,
        UseListViewForComboBoxItems  = 0x00000002,
        UsePlainTextEditForTextInput = 0x00000004
    };

    Q_DECLARE_FLAGS(InputDialogOptions, InputDialogOption)

    enum InputMode {
        TextInput,
        IntInput,
        DoubleInput
    };

    KInputDialog(QWidget *parent = nullptr);
    ~KInputDialog();

    /**
     * @brief 设置输入模式
     * @param mode
     */
    void setInputMode(InputMode mode);

    /**
     * @brief 返回输入模式
     * @return InputMode
     */
    InputMode inputMode() const;

    /**
     * @brief 设置提示信息
     * @param text
     */
    void setLabelText(const QString &text);

    /**
     * @brief 获取提示信息
     * @return QString
     */
    QString labelText() const;

    /**
     * @brief 设置option类型
     * @param option
     * @param on
     */
    void setOption(InputDialogOption option, bool on = true);

    /**
     * @brief 返回是否设置了option类型
     * @param option
     * @return bool
     */
    bool testOption(InputDialogOption option) const;

    /**
     * @brief 设置options类型
     * @param options
     */
    void setOptions(InputDialogOptions options);
    /**
     * @brief 返回options类型
     * @return InputDialogOptions
     */
    InputDialogOptions options() const;

    /**
     * @brief 设置文本值
     * @param text
     */
    void setTextValue(const QString &text);
    /**
     * @brief 获取当前文本值
     * @return QString
     */
    QString textValue() const;

    /**
     * @brief 设置保存文本值的回声模式 QLineEdit::EchoMode
     * @param mode
     */
    void setTextEchoMode(QLineEdit::EchoMode mode);
    /**
     * @brief 返回保存文本值的回声模式
     * @return QLineEdit::EchoMode
     */
    QLineEdit::EchoMode textEchoMode() const;

    /**
     * @brief 设置输入对话框中使用的组合框是否可编辑
     * @param editable
     */
    void setComboBoxEditable(bool editable);

    /**
     * @brief 返回输入对话框中使用的组合框是否可编辑
     * @return bool
     */
    bool isComboBoxEditable() const;

    /**
     * @brief 设置输入对话框组合框中使用的项目
     * @param items
     */
    void setComboBoxItems(const QStringList &items);

    /**
     * @brief 返回输入对话框组合框中使用的项目
     * @return QStringList
     */
    QStringList comboBoxItems() const;

    /**
     * @brief 设置int类型值
     * @param value
     */
    void setIntValue(int value);

    /**
     * @brief 获取int类型值
     * @return int
     */
    int intValue() const;

    /**
     * @brief 设置int类型最小值
     * @param min
     */
    void setIntMinimum(int min);
    /**
     * @brief 获取int类型最小值
     * @return int
     */
    int intMinimum() const;

    /**
     * @brief 设置int类型最大值
     * @param max
     */
    void setIntMaximum(int max);
    /**
     * @brief 获取int类型最大值
     * @return int
     */
    int intMaximum() const;

    /**
     * @brief 设置int类型值范围
     * @param min
     * @param max
     */
    void setIntRange(int min, int max);

    /**
     * @brief 设置步长
     * @param step
     */
    void setIntStep(int step);
    /**
     * @brief 获取int类型步长
     * @return int
     */
    int intStep() const;

    /**
     * @brief 设置double类型值
     * @param value
     */
    void setDoubleValue(double value);
    /**
     * @brief 获取double类型值
     * @return double
     */
    double doubleValue() const;

    /**
     * @brief 设置double类型最小值
     * @param min
     */
    void setDoubleMinimum(double min);

    /**
     * @brief 获取double类型最小值
     * @return double
     */
    double doubleMinimum() const;

    /**
     * @brief 设置double类型最大值
     * @param max
     */
    void setDoubleMaximum(double max);

    /**
     * @brief 获取double类型最大值
     * @return  double
     */
    double doubleMaximum() const;

    /**
     * @brief 设置double类型值范围
     * @param min
     * @param max
     */
    void setDoubleRange(double min, double max);

    /**
     * @brief 以小数为单位设置双自旋盒的精度
     * @param decimals
     */
    void setDoubleDecimals(int decimals);

    /**
     * @brief 获取双自旋盒的精度
     * @return int
     */
    int doubleDecimals() const;

    /**
     * @brief 设置OK按钮的文本
     * @param text
     */
    void setOkButtonText(const QString &text);

    /**
     * @brief 获取OK按钮的文本
     * @return QString
     */
    QString okButtonText() const;

    /**
     * @brief 设置cancel按钮的文本
     * @param text
     */
    void setCancelButtonText(const QString &text);
    /**
     * @brief 获取cancel按钮的文本
     * @return
     */
    QString cancelButtonText() const;

    using QDialog::open;
    /**
     * @brief 将对话框显示为窗口模式对话框，并立即返回。
     * @param receiver
     * @param member
     */
    void open(QObject *receiver, const char *member);

    /**
     * @brief 获取部件的推荐最小尺寸
     * @return QSize
     */
    QSize minimumSizeHint() const override;

    /**
     * @brief 获取保存部件的推荐尺寸
     * @return
     */
    QSize sizeHint() const override;

    /**
     * @brief 设置窗口部件是否可见
     * @param visible
     */
    void setVisible(bool visible) override;

    /**
     * @brief 获取背景提示文本
     * @return
     * @since 1.2
     */
    QString	placeholderText() const;

    /**
     * @brief 设置背景提示文本
     * @since 1.2
     */
    void setPlaceholderText(const QString &);

    /**
     * @brief 静态便利函数，用于从用户处获取单行字符串。
     * @param parent
     * @param label
     * @param echo
     * @param text
     * @param ok
     * @param flags
     * @param inputMethodHints
     * @return
     */
    static QString getText(QWidget *parent,const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString(), bool *ok = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags(),
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    /**
     * @brief 静态便利函数，用于从用户处获取多行字符串。
     * @param parent
     * @param label
     * @param text
     * @param ok
     * @param flags
     * @param inputMethodHints
     * @return
     */
    static QString getMultiLineText(QWidget *parent,const QString &label,
                                    const QString &text = QString(), bool *ok = nullptr,
                                    Qt::WindowFlags flags = Qt::WindowFlags(),
                                    Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    /**
     * @brief 静态便利函数，用于让用户从字符串列表中选择一个项目。
     * @param parent
     * @param label
     * @param items
     * @param current
     * @param editable
     * @param ok
     * @param flags
     * @param inputMethodHints
     * @return
     */
    static QString getItem(QWidget *parent,const QString &label,
                           const QStringList &items, int current = 0, bool editable = true,
                           bool *ok = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(),
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    /**
     * @brief 静态便利函数，用于从用户处获取整数输入值。
     * @param parent
     * @param label
     * @param value
     * @param minValue
     * @param maxValue
     * @param step
     * @param ok
     * @param flags
     * @return
     */
    static int getInt(QWidget *parent,const QString &label, int value = 0,
                      int minValue = -2147483647, int maxValue = 2147483647,
                      int step = 1, bool *ok = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    /**
     * @brief 静态便利函数，用于从用户处获取浮点数。
     * @param parent
     * @param label
     * @param value
     * @param minValue
     * @param maxValue
     * @param decimals
     * @param ok
     * @param flags
     * @return
     */
    static double getDouble(QWidget *parent,const QString &label,
                            double value = 0, double minValue = -2147483647,
                            double maxValue = 2147483647, int decimals = 1, bool *ok = nullptr,
                            Qt::WindowFlags flags = Qt::WindowFlags());


    /**
     * @brief 设置doubel类型步长
     * @param step
     */
    void setDoubleStep(double step);
    /**
     * @brief 获取doubel类型步长
     * @return double
     */
    double doubleStep() const;

Q_SIGNALS:
    // ### emit signals!
    void textValueChanged(const QString &text);
    void textValueSelected(const QString &text);
    void intValueChanged(int value);
    void intValueSelected(int value);
    void doubleValueChanged(double value);
    void doubleValueSelected(double value);

public:
    void done(int result) override;

protected:
    void changeTheme();

private:
    KInputDialogPrivate* const d_ptr;
    Q_DISABLE_COPY(KInputDialog)
    Q_DECLARE_PRIVATE(KInputDialog)
    Q_PRIVATE_SLOT(d_ptr, void _q_textChanged(const QString&))
    Q_PRIVATE_SLOT(d_ptr, void _q_plainTextEditTextChanged())
    Q_PRIVATE_SLOT(d_ptr, void _q_currentRowChanged(const QModelIndex&, const QModelIndex&))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KInputDialog::InputDialogOptions)
}
/**
  * @example testinputdialog/widget.h
  * @example testinputdialog/widget.cpp
  * @}
  */
#endif // KINPUTDIALOG_H
