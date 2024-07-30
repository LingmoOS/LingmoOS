
#ifndef KCONFIGVIEWSTATESAVER_H
#define KCONFIGVIEWSTATESAVER_H

#include <KViewStateSerializer>

#include "kconfigwidgets_export.h"

class KConfigGroup;

/**
 * @class KConfigViewStateSaver kconfigviewstatesaver.h KConfigViewStateSaver
 *
 * @brief Base class for saving and restoring state in QTreeViews and QItemSelectionModels using KConfig as storage
 */
class KCONFIGWIDGETS_EXPORT KConfigViewStateSaver : public KViewStateSerializer
{
    Q_OBJECT
public:
    explicit KConfigViewStateSaver(QObject *parent = nullptr);

    /**
      Saves the state to the @p configGroup
    */
    void saveState(KConfigGroup &configGroup);

    /**
      Restores the state from the @p configGroup
    */
    void restoreState(const KConfigGroup &configGroup);
};

#endif
