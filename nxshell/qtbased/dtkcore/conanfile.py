# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

from conans import ConanFile, tools


class DtkcoreConan(ConanFile):
    name = 'dtkcore'
    version = '2.0.9'
    license = 'GPL'
    author = 'Iceyer me@iceyer.net'
    url = 'https://github.com/linuxdeepin/dtkcore'
    description = 'cross platform ui library'
    topics = ('qt', 'dtk')
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {'shared': [True, False]}
    default_options = 'shared=False'
    generators = 'qmake'
    exports_sources = '*'
    requires = 'jom_installer/1.1.2@bincrafters/stable', 'qt/5.6.3@iceyer/stable'

    def extend_include_path(self):
        return '%s/include/libdtk-%s/DCore' % (self.package_folder, self.version)

    # def source(self):
    #     self.run('git clone https://github.com/linuxdeepin/dtkcore.git source')
    #     self.run('cd source && git checkout 2.0.9.9')

    def build(self):
        outdir = self.build_folder
        # includedir = outdir + '/include'
        mkspecsdir = outdir + '/mkspecs'
        # libdir = outdir + '/lib'

        env_vars = tools.vcvars_dict(self.settings)
        env_vars['_CL_'] = '/utf-8'
        with tools.environment_append(env_vars):
            command = 'qmake -r'
            command += ' VERSION=%s' % self.version
            # command += ' CONFIG-=debug_and_release'
            # command += ' CONFIG-=debug_and_release_target'
            command += ' CONFIG+=release'
            command += ' PREFIX=%s' % outdir
            command += ' MKSPECS_INSTALL_DIR=%s' % mkspecsdir
            command += ' DTK_STATIC_LIB=YES'
            command += ' DTK_STATIC_TRANSLATION=YES'
            command += ' DTK_NO_MULTIMEDIA=YES'
            command += ' %s' % self.source_folder
            self.run(command)
            self.run('jom clean')
            self.run('jom')
            self.run('jom install')

    def package(self):
        self.deploy()

        outdir = self.build_folder
        self.copy('*', dst='include', src=outdir+'/include')
        self.copy('*.lib', dst='lib', src=outdir+'/lib')
        self.copy('*', dst='mkspecs', src=outdir+'/mkspecs')

    def package_info(self):
        self.cpp_info.libs = ['dtkcore']
        self.cpp_info.includedirs.append(self.extend_include_path())
        self.env_info.QMAKEPATH = self.cpp_info.rootpath
        self.env_info.QMAKEFEATURES = self.cpp_info.rootpath + '/mkspecs/features'

    def deploy(self):
        try:
            content = []
            module_pri = self.build_folder + '/mkspecs/modules/qt_lib_dtkcore.pri'
            s = open(module_pri)
            for line in s.readlines():
                if line.startswith('QT.dtkcore.tools'):
                    line = 'QT.dtkcore.tools = %s\n' % (
                        self.package_folder + '/bin')
                elif line.startswith('QT.dtkcore.libs'):
                    line = 'QT.dtkcore.libs = %s\n' % (
                        self.package_folder + '/lib')
                elif line.startswith('QT.dtkcore.includes'):
                    line = 'QT.dtkcore.includes = %s\n' % (
                        self.extend_include_path())
                content.append(line)
            s.close()

            # print('create module file', content)
            s = open(module_pri, 'w')
            s.writelines(content)
        except FileNotFoundError:
            print('skip update qt module file')
