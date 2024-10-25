#include "containmentactions.h"
#include <KPluginFactory>

class DummyContainmentAction : public Lingmo::ContainmentActions
{
    Q_OBJECT

public:
    explicit DummyContainmentAction(QObject *parent)
        : Lingmo::ContainmentActions(parent)
    {
    }
};

K_PLUGIN_CLASS_WITH_JSON(DummyContainmentAction, "dummycontainmentaction.json")

#include "dummycontainmentaction.moc"
