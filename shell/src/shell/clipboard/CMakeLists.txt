# The clipboard history without its picker: the store, the monitor on the seat's
# selection, the picker's model and org.lingmo.ClipboardHistory.
add_library(lingmo-shell-clipboard STATIC
    clipboard_history.cpp
    clipboard_history.h
    clipboard_monitor.cpp
    clipboard_monitor.h
    clipboard_store.cpp
    clipboard_store.h
)
target_link_libraries(lingmo-shell-clipboard PUBLIC lingmo-shell-common)
