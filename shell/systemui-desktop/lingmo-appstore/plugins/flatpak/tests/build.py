#!/usr/bin/python3

import subprocess
import os
import shutil
import configparser

def build_flatpak(appid, srcdir, repodir, branch='master', cleanrepodir=True):
    print('Building %s from %s into %s' % (appid, srcdir, repodir))

    # delete repodir
    if cleanrepodir and os.path.exists(repodir):
        print("Deleting %s" % repodir)
        shutil.rmtree(repodir)

    # delete exportdir
    exportdir = os.path.join(srcdir, appid, 'export')
    if os.path.exists(exportdir):
        print("Deleting %s" % exportdir)
        shutil.rmtree(exportdir)

    metadata_path = os.path.join(srcdir, appid, 'metadata')
    metadata = configparser.ConfigParser()
    metadata.read(metadata_path)
    is_runtime = True if 'Runtime' in metadata.sections() else False
    is_extension = True if 'ExtensionOf' in metadata.sections() else False

    # runtimes have different defaults
    if is_runtime and not is_extension:
        prefix = 'usr'
    else:
        prefix = 'files'

    # finish the build
    argv = ['flatpak', 'build-finish']
    argv.append(os.path.join(srcdir, appid))
    subprocess.call(argv)

    # compose AppStream data
    argv = ['appstream-compose']
    argv.append('--origin=flatpak')
    argv.append('--basename=%s' % appid)
    argv.append('--prefix=%s' % os.path.join(srcdir, appid, prefix))
    argv.append('--output-dir=%s' % os.path.join(srcdir, appid, prefix, 'share/app-info/xmls'))
    argv.append(appid)
    subprocess.call(argv)

    # export into repo
    argv = ['flatpak', 'build-export']
    argv.append(repodir)
    argv.append(os.path.join(srcdir, appid))
    argv.append(branch)
    argv.append('--update-appstream')
    argv.append('--timestamp=2016-09-15T01:02:03')
    if is_runtime:
        argv.append('--runtime')
    subprocess.call(argv)

def build_flatpak_bundle(appid, srcdir, repodir, filename, branch='master'):
    argv = ['flatpak', 'build-bundle']
    argv.append(repodir)
    argv.append(filename)
    argv.append(appid)
    argv.append(branch)
    subprocess.call(argv)

def copy_repo(srcdir, destdir):
    srcdir_repo = os.path.join(srcdir, 'repo')
    destdir_repo = os.path.join(destdir, 'repo')
    print("Copying %s to %s" % (srcdir_repo, destdir_repo))
    if os.path.exists(destdir_repo):
        shutil.rmtree(destdir_repo)
    shutil.copytree(srcdir_repo, destdir_repo)

# normal app with runtime in same remote
build_flatpak('org.test.Chiron',
              'app-with-runtime',
              'app-with-runtime/repo')
build_flatpak('org.test.Runtime',
              'app-with-runtime',
              'app-with-runtime/repo',
              cleanrepodir=False)

# build a flatpak bundle for the app
build_flatpak_bundle('org.test.Chiron',
                     'app-with-runtime',
                     'app-with-runtime/repo',
                     'chiron.flatpak')

# app referencing runtime that cannot be found
build_flatpak('org.test.Chiron',
              'app-with-runtime',
              'app-missing-runtime/repo')

# app with an update
build_flatpak('org.test.Runtime',
              'app-with-runtime',
              'app-update/repo',
              branch='new_master',
              cleanrepodir=True)
build_flatpak('org.test.Chiron',
              'app-update',
              'app-update/repo',
              cleanrepodir=False)

# just a runtime present
build_flatpak('org.test.Runtime',
              'only-runtime',
              'only-runtime/repo')

# app with an extension
copy_repo('only-runtime', 'app-extension')
build_flatpak('org.test.Chiron',
              'app-extension',
              'app-extension/repo',
              cleanrepodir=False)
build_flatpak('org.test.Chiron.Extension',
              'app-extension',
              'app-extension/repo',
              cleanrepodir=False)
copy_repo('app-extension', 'app-extension-update')
build_flatpak('org.test.Chiron.Extension',
              'app-extension-update',
              'app-extension-update/repo',
              cleanrepodir=False)
