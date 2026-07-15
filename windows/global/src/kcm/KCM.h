#pragma once

#include <kcmodule.h>
#include "Config.h"

namespace Ui {
    class Form;
}

namespace ShapeCorners {
    class KCM final : public KCModule {
    Q_OBJECT

    public:
        explicit KCM(QObject *parent, const KPluginMetaData &args);

    public Q_SLOTS:
        void defaults() override;
        void load() override;
        void save() override;
        void update_colors();
        void update_windows() const;
        void outline_group_toggled(bool value) const;

    private:
        std::shared_ptr<Ui::Form> ui;
        Config config;

        void load_ui() const;

        QWidget *widget() final override { return KCModule::widget(); }
        const QPalette &palette() { return widget()->palette(); }
    };
}
