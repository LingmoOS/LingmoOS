prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE_INSTALL_BINDIR}
libdir=${KDE_INSTALL_LIBDIR}
includedir=${KDE_INSTALL_INCLUDEDIR}

Name: libKActivitiesStats
Description: libKActivitiesStats is a C++ library for using KDE activities
URL: http://www.kde.org
Requires: Qt6Core
Version: ${KACTIVITIESSTATS_VERSION}
Libs: -L${KDE_INSTALL_LIBDIR} -lKF6ActivitiesStats
Cflags: -I${KDE_INSTALL_INCLUDEDIR}
