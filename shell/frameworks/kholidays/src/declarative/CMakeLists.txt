ecm_add_qml_module(kholidaysdeclarativeplugin URI "org.kde.kholidays")

target_sources(kholidaysdeclarativeplugin PRIVATE
  kholidaysdeclarativeplugin.cpp
  holidayregionsmodel.cpp
  kholidaysdeclarativeplugin.h
  holidayregionsmodel.h
)

target_link_libraries(kholidaysdeclarativeplugin PRIVATE
  Qt6::Qml
  Qt6::Core
  KF6Holidays
)

ecm_finalize_qml_module(kholidaysdeclarativeplugin DESTINATION ${KDE_INSTALL_QMLDIR})
