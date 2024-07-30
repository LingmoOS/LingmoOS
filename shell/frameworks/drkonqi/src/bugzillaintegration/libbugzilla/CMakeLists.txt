set(lib_SRCS
    apijob.cpp
    bugzilla.cpp
    connection.cpp
    exceptions.cpp
    query.cpp
    clients/attachmentclient.cpp
    clients/bugclient.cpp
    clients/bugfieldclient.cpp
    clients/commentclient.cpp
    clients/clientbase.cpp
    clients/productclient.cpp
    clients/commands/bugsearch.cpp
    clients/commands/bugupdate.cpp
    clients/commands/jsoncommand.cpp
    clients/commands/newattachment.cpp
    clients/commands/newbug.cpp
    clients/commands/querycommand.cpp
    models/bug.cpp
    models/bugfield.cpp
    models/comment.cpp
    models/logindetails.cpp
    models/product.cpp

    apijob.h
    bugzilla.h
    connection.h
    exceptions.h
    query.h
    clients/attachmentclient.h
    clients/bugclient.h
    clients/bugfieldclient.h
    clients/commentclient.h
    clients/clientbase.h
    clients/productclient.h
    clients/commands/bugsearch.h
    clients/commands/bugupdate.h
    clients/commands/jsoncommand.h
    clients/commands/newattachment.h
    clients/commands/newbug.h
    clients/commands/querycommand.h
    models/bug.h
    models/bugfield.h
    models/comment.h
    models/logindetails.h
    models/product.h
)

ecm_qt_declare_logging_category(
    lib_SRCS
    HEADER bugzilla_debug.h
    IDENTIFIER BUGZILLA_LOG
    CATEGORY_NAME org.kde.drkonqi.bugzilla
    DEFAULT_SEVERITY Warning
    DESCRIPTION "drkonqi bugzilla"
    EXPORT DRKONQI
)

add_library(qbugzilla STATIC ${lib_SRCS})
target_link_libraries(qbugzilla PUBLIC Qt::Core Qt::Network KF6::CoreAddons)
