/*
 * Copyright (c) 2023 by LingmoOS, All Rights Reserved. 
 */
#ifndef MODULEINTERFACE_H
#define MODULEINTERFACE_H

#include <QObject>
#include <QColor>
#include <QString>

class ModuleInterface
{
    public:
        ModuleInterface() = default;
        ///
        /// \brief name
        /// your module name
        /// \return
        ///
        virtual const QString name() const = 0;

        ///
        /// \brief title
        /// 模块名，用于显示
        /// \return
        ///
        virtual const QString title() const = 0;

        /**
         * @brief enabled
         * @return 插件是否处于可用状态
         */
        virtual bool enabled() const {
            return true;
        }

        virtual QString qmlPath() const =0;

        virtual QString category() const =0;

        virtual QString iconPath() const =0;

        virtual QColor iconColor() const =0;
};
Q_DECLARE_INTERFACE(ModuleInterface, "com.lingmo.settings.module")

#endif // MODULEINTERFACE_H