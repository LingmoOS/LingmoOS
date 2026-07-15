#include "KCM.h"

#include <KPluginFactory>

namespace ShapeCorners {
    K_PLUGIN_FACTORY_WITH_JSON(ShapeCornersConfigFactory,
                               "metadata.json",
                               registerPlugin<KCM>();)
}

#include "plugin.moc"
