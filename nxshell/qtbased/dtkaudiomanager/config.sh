#/usr/bin/bash
# -n name
# -h help

ChangeName() {
    echo "begin config name: $1..."
    name=$1
    sed -i "s/dtkdemo/${name}/g" .reuse/dep5
    # modify CMakeLists.txt
    sed -i "s/dtkdemo/${name}/g" CMakeLists.txt
    sed -i "s/dtkdemo/${name}/g" tests/CMakeLists.txt
    # rename config file
    mv misc/dtkdemo.pc.in "misc/${name}.pc.in"
    mv misc/dtkdemoConfig.cmake.in "misc/${name}Config.cmake.in"
    mv misc/qt_lib_dtkdemo.pri.in "misc/qt_lib_${name}.pri.in"
    # rename debian file
    mv debian/libdtkdemo.install "debian/lib${name}.install"
    mv debian/libdtkdemo-dev.install "debian/lib${name}-dev.install"
    # modify debian file content
    sed -i "s/dtkdemo/${name}/g" debian/{control,changelog,copyright}
    # modify test script
    sed -i "s/dtkdemo/${name}/g" test-recoverage.sh
    echo "end config name"
}

ChangeNamespace() {
    echo "begin config namespace: $1..."
    name=$1
    sed -i "s/Demo/${name}/g" include/namespace.h docs/CMakeLists.txt
    sed -i "s/DDEMO/D${name^^}/g" include/*.h src/*.cpp tests/*.cpp docs/CMakeLists.txt
    echo "end config namespace"
}

Print_help() {
    cat << EOF
config.sh <options> value

config the project for custom.

eg:bash config.sh -n dtktest
   bash config.sh -N Test

Options:
  -n project name
  -N namespace
  -h print help
EOF
}

while getopts ":n:N:h" optname
do
    case "$optname" in
      "n")
        ChangeName $OPTARG
        ;;
      "N")
        ChangeNamespace $OPTARG
        ;;
      "h")
        Print_help
        ;;
      ":")
        echo "No argument value for option $OPTARG"
        ;;
      "?")
        echo "Unknown option $OPTARG"
        ;;
      *)
        echo "Unknown error while processing options"
        ;;
    esac
    #echo "option index is $OPTIND"
done

if [ $# -eq 0 ];then
    Print_help
fi
