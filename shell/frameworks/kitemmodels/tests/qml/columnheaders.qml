import QtQuick 2.0
import org.kde.kitemmodels 1.0

// This test is somewhat lame in that it should only display a single "1".
// ListModel only has one column and no way of adding more. And there is not
// really another model that is simple to create from QML that does have columns
// and column headers.

ListView {
    model: KColumnHeadersProxyModel {
        sourceModel: ListModel {
            ListElement { display: "test1" }
            ListElement { display: "test2" }
        }
    }

    delegate: Text {
        text: model.display
    }
}

