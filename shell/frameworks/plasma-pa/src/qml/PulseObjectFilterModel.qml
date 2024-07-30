import org.kde.kitemmodels as KItemModels

import org.kde.plasma.private.volume

KItemModels.KSortFilterProxyModel {
    property var filters: []
    property bool filterOutInactiveDevices: false
    property bool filterVirtualDevices: false

    onFilterVirtualDevicesChanged: {
        invalidate()
    }

    filterRowCallback: function(source_row, source_parent) {
        var idx = sourceModel.index(source_row, 0);

        // Don't ever show the dummy output, that's silly
        var dummyOutputName = "auto_null"
        if (sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role("Name")) === dummyOutputName) {
            return false;
        }

        // Optionally run the role-based filters
        if (filters.length > 0) {
            for (var i = 0; i < filters.length; ++i) {
                var filter = filters[i];
                if (sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role(filter.role)) !== filter.value) {
                    return false;
                }
            }
        }

        // Optionally exclude inactive devices
        if (filterOutInactiveDevices) {
            var ports = sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role("PulseObject")).ports;
            if (ports.length === 1 && ports[0].availability === Port.Unavailable) {
                return false;
            }
        }

        if (filterVirtualDevices && sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role("PulseObject")).virtualDevice) {
            return false;
        }

        return true;
    }
}
