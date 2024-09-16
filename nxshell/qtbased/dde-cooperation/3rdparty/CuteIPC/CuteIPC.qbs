import qbs

Project {

    DynamicLibrary {
        name: "CuteIPC"

        files: [ "src/*", "include/*" ]        

        cpp.includePaths: "include"
        cpp.defines: ["CUTEIPC_LIBRARY" ]

        Depends { name: "cpp" }
        Depends {
            name: "Qt";
            submodules: [ "core", "gui", "network" ]
        }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: "include"
        }

        Group {
            qbs.install: true
            qbs.installDir: "lib"
            fileTagsFilter: "dynamiclibrary"
        }
    }

    references: [
        "test/test.qbs",
    ]
}
