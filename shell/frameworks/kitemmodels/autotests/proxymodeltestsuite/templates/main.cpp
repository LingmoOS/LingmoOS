
#include <QApplication>
#include <dynamictreemodel.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    DynamicTreeModel rootModel;
    {
        % if initEvent %
    }
    { % with initEvent as event % } // Include {{ "init.cpp" }}: {% include "init.cpp" %}
    { % endwith % } { % endif % }

    // Create and connect the proxy model here.
    // eg:

    // QSortFilterProxyModel proxy;
    // proxy.setDynamicSortFilter(true);
    // proxy.setSourceModel(&rootModel);

    // Have {{ events|length }} Events.
    {% for event in events %
    }
    { % with event.type | lower | stringformat : "%1.cpp" as eventtemplate % } // Include {{ eventtemplate }}: {% include eventtemplate %}
    { % endwith % } {
        % endfor %
    }

    return app.exec();
}
