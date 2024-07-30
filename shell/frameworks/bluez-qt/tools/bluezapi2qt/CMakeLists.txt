add_executable(bluezapi2qt
    BluezApiParser.cpp
    Comment.cpp
    CppGenerator.cpp
    Interface.cpp
    Method.cpp
    Methods.cpp
    Parameter.cpp
    Properties.cpp
    Property.cpp
    TypeAnnotation.cpp
    XmlGenerator.cpp
    main.cpp
)

target_link_libraries(bluezapi2qt
    Qt6::Core
)
