ecm_add_qml_module(KF6UserFeedbackQml URI "org.kde.userfeedback")

target_sources(KF6UserFeedbackQml PRIVATE
    qmlabstractdatasource.cpp
    qmldatasources.cpp
    qmlplugin.cpp
    qmlpropertyratiosource.cpp
    qmlproviderextension.cpp
    qmlpropertysource.cpp

    qmlabstractdatasource.h
    qmldatasources.h
    qmlplugin.h
    qmlpropertyratiosource.h
    qmlproviderextension.h
    qmlpropertysource.h
)

target_link_libraries(KF6UserFeedbackQml PRIVATE
    KF6UserFeedbackCore
    Qt6::Qml
)

ecm_finalize_qml_module(KF6UserFeedbackQml)
