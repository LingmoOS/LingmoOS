# Sub build: invoker
add_subdirectory(invoker)

# Sub build: launcher library
add_subdirectory(launcherlib)

# Sub build: generic booster plugin
#add_subdirectory(booster-generic)

# Sub build: Deepin Tool Kit widget booster plugin
add_subdirectory(booster-dtkwidget)

# Sub build: single-instance binary / library
add_subdirectory(single-instance)

# Sub build: desktop booster plugin
#add_subdirectory(booster-desktop)

# Sub build: Deepin Tool Kit declarative booster plugin
add_subdirectory(booster-dtkdeclarative)