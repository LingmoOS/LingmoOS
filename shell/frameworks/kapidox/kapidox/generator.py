# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
# SPDX-FileCopyrightText: 2014 Alex Turbov <i.zaufi@gmail.com>
# SPDX-FileCopyrightText: 2016 Olivier Churlaud <olivier@churlaud.com>
#
# SPDX-License-Identifier: BSD-2-Clause

import codecs
import datetime
import os
import logging
from os import environ
import shutil
import subprocess
import tempfile
import sys
import pathlib
from typing import Any, Dict
import xml.etree.ElementTree as ET
import re
import glob
from pathlib import Path

import jinja2

from urllib.parse import urljoin

import xml.etree.ElementTree as xmlET
import json

from jinja2.environment import Template

from kapidox import utils
try:
    from kapidox import depdiagram
    DEPDIAGRAM_AVAILABLE = True
except ImportError:
    DEPDIAGRAM_AVAILABLE = False

from .doxyfilewriter import DoxyfileWriter


# @package kapidox.generator
#
# The generator

__all__ = (
    "Context",
    "generate_apidocs",
    "search_for_tagfiles",
    "WARN_LOGFILE",
    "build_classmap",
    "postprocess",
    "create_dirs",
    "create_jinja_environment",
    )

WARN_LOGFILE = 'doxygen-warnings.log'

HTML_SUBDIR = 'html'


class Context(object):
    """
    Holds parameters used by the various functions of the generator
    """
    __slots__ = (
        # Names
        'modulename',
        'fancyname',
        'title',
        'fwinfo',
        # KApidox files
        'doxdatadir',
        'resourcedir',
        # Input
        'srcdir',
        'tagfiles',
        'dependency_diagram',
        'copyright',
        'is_qdoc',
        # Output
        'outputdir',
        'htmldir',
        'tagfile',
        # Output options
        'man_pages',
        'qhp',
        # Binaries
        'doxygen',
        'qhelpgenerator',
    )

    def __init__(self, args, **kwargs):
        # Names
        self.title = args.title
        # KApidox files
        self.doxdatadir = args.doxdatadir
        # Output options
        self.man_pages = args.man_pages
        self.qhp = args.qhp
        # Binaries
        self.doxygen = args.doxygen
        self.qhelpgenerator = args.qhelpgenerator

        for key in self.__slots__:
            if not hasattr(self, key):
                setattr(self, key, kwargs.get(key))


def create_jinja_environment(doxdatadir):
    loader = jinja2.FileSystemLoader(os.path.join(doxdatadir, 'templates'))
    return jinja2.Environment(loader=loader)


def process_toplevel_html_file(outputfile, doxdatadir, products, title, qch_enabled=False):

    def sort_product(product):
        print(product.fancyname)
        if product.fancyname == "The KDE Frameworks":
            return 'aa'
        if product.fancyname == "KDE PIM":
            return 'ab'
        return product.fancyname.lower()

    products.sort(key=sort_product)
    mapping = {
            'resources': './resources',
            # steal the doxygen css from one of the frameworks
            # this means that all the doxygen-provided images etc. will be found
            'title': title,
            'qch': qch_enabled,
            'breadcrumbs': {
                'entries': [
                    {
                        'href': './index.html',
                        'text': 'KDE API Reference'
                    }
                    ]
                },
            'product_list': products,
        }
    tmpl = create_jinja_environment(doxdatadir).get_template('frontpage.html')
    with codecs.open(outputfile, 'w', 'utf-8') as outf:
        outf.write(tmpl.render(mapping))

    tmpl2 = create_jinja_environment(doxdatadir).get_template('search.html')
    search_output = "search.html"
    with codecs.open(search_output, 'w', 'utf-8') as outf:
        outf.write(tmpl2.render(mapping))


def process_subgroup_html_files(outputfile, doxdatadir, groups, available_platforms, title, qch_enabled=False):

    for group in groups:
        mapping = {
            'resources': '../resources',
            'title': title,
            'qch': qch_enabled,
            'breadcrumbs': {
                'entries': [
                    {
                        'href': '../index.html',
                        'text': 'KDE API Reference'
                    },
                    {
                        'href': './index.html',
                        'text': group.fancyname
                    }
                    ]
                },
            'group': group,
            'available_platforms': sorted(available_platforms),
        }

        if not os.path.isdir(group.name):
            os.mkdir(group.name)
        outputfile = group.name + '/index.html'
        tmpl = create_jinja_environment(doxdatadir).get_template('subgroup.html')
        with codecs.open(outputfile, 'w', 'utf-8') as outf:
            outf.write(tmpl.render(mapping))

        tmpl2 = create_jinja_environment(doxdatadir).get_template('search.html')
        search_output = group.name + "/search.html"
        with codecs.open(search_output, 'w', 'utf-8') as outf:
            outf.write(tmpl2.render(mapping))


def create_dirs(ctx):
    ctx.htmldir = os.path.join(ctx.outputdir, HTML_SUBDIR)
    ctx.tagfile = os.path.join(ctx.htmldir, ctx.fwinfo.fancyname + '.tags')

    if not os.path.exists(ctx.outputdir):
        os.makedirs(ctx.outputdir)

    if os.path.exists(ctx.htmldir):
        # If we have files left there from a previous run but which are no
        # longer generated (for example because a C++ class has been removed)
        # then postprocess will fail because the left-over file has already been
        # processed. To avoid that, we delete the html dir.
        shutil.rmtree(ctx.htmldir)
    os.makedirs(ctx.htmldir)


def load_template(path):
    # Set errors to 'ignore' because we don't want weird characters in Doxygen
    # output (for example source code listing) to cause a failure
    content = codecs.open(path, encoding='utf-8', errors='ignore').read()
    try:
        return jinja2.Template(content)
    except jinja2.exceptions.TemplateSyntaxError:
        logging.error('Failed to parse template {}'.format(path))
        raise


def find_tagfiles(docdir, doclink=None, flattenlinks=False, exclude=None, _depth=0):
    """Find Doxygen-generated tag files in a directory.

    The tag files must have the extension .tags, and must be in the listed
    directory, a subdirectory or a subdirectory named html of a subdirectory.

    Args:
        docdir:       (string) the directory to search.
        doclink:      (string) the path or URL to use when creating the
                      documentation links; if None, this will default to
                      docdir. (optional, default None)
        flattenlinks: (bool) if True, generated links will assume all the html
                      files are directly under doclink; else the html files are
                      assumed to be at the same relative location to doclink as
                      the tag file is to docdir; ignored if doclink is not set.
                      (optional, default False)

    Returns:
        A list of pairs of (tag_file,link_path).
    """

    if not os.path.isdir(docdir):
        return []

    if doclink is None:
        doclink = docdir
        flattenlinks = False

    def smartjoin(pathorurl1, *args):
        """Join paths or URLS

        It figures out which it is from whether the first contains a "://"
        """
        if '://' in pathorurl1:
            if not pathorurl1.endswith('/'):
                pathorurl1 += '/'
            return urljoin(pathorurl1, *args)
        else:
            return os.path.join(pathorurl1, *args)

    def nestedlink(subdir):
        if flattenlinks:
            return doclink
        else:
            return smartjoin(doclink, subdir)

    tagfiles = []

    entries = os.listdir(docdir)
    for e in entries:
        if e == exclude:
            continue
        path = os.path.join(docdir, e)
        if os.path.isfile(path) and e.endswith('.tags'):
            tagfiles.append((path, doclink))
        elif (_depth == 0 or (_depth == 1 and e == 'html')) and os.path.isdir(path):
            tagfiles += find_tagfiles(path, nestedlink(e),
                                      flattenlinks=flattenlinks,
                                      _depth=_depth+1,
                                      exclude=exclude)

    return tagfiles


def search_for_tagfiles(suggestion=None, doclink=None, flattenlinks=False, searchpaths=[], exclude=None):
    """Find Doxygen-generated tag files

    See the find_tagfiles documentation for how the search is carried out in
    each directory; this just allows a list of directories to be searched.

    At least one of docdir or searchpaths must be given for it to find anything.

    Args:
        suggestion:   the first place to look (will complain if there are no
                      documentation tag files there)
        doclink:      the path or URL to use when creating the documentation
                      links; if None, this will default to docdir
        flattenlinks: if this is True, generated links will assume all the html
                      files are directly under doclink; if False (the default),
                      the html files are assumed to be at the same relative
                      location to doclink as the tag file is to docdir; ignored
                      if doclink is not set
        searchpaths:  other places to look for documentation tag files

    Returns:
        A list of pairs of (tag_file,link_path)
    """

    if suggestion is not None:
        if not os.path.isdir(suggestion):
            logging.warning(suggestion + " is not a directory")
        else:
            tagfiles = find_tagfiles(suggestion, doclink, flattenlinks, exclude)
            if len(tagfiles) == 0:
                logging.warning(suggestion + " does not contain any tag files")
            else:
                return tagfiles

    for d in searchpaths:
        tagfiles = find_tagfiles(d, doclink, flattenlinks, exclude)
        if len(tagfiles) > 0:
            logging.info("Documentation tag files found at " + d)
            return tagfiles

    return []


def menu_items(htmldir, modulename):
    """Menu items for standard Doxygen files

    Looks for a set of standard Doxygen files (like namespaces.html) and
    provides menu text for those it finds in htmldir.

    Args:
        htmldir:    (string) the directory the HTML files are contained in.
        modulename: (string) the name of the library

    Returns:
        A list of maps with 'text' and 'href' keys.
    """
    entries = [
            {'text': 'Main Page', 'href': 'index.html'},
            {'text': 'Namespace List', 'href': 'namespaces.html'},
            {'text': 'Namespace Members', 'href': 'namespacemembers.html'},
            {'text': 'Alphabetical List', 'href': 'classes.html'},
            {'text': 'Class List', 'href': 'annotated.html'},
            {'text': 'Class Hierarchy', 'href': 'hierarchy.html'},
            {'text': 'File List', 'href': 'files.html'},
            {'text': 'File Members', 'href': 'globals.html'},
            {'text': 'Modules', 'href': 'modules.html'},
            {'text': 'Directories', 'href': 'dirs.html'},
            {'text': 'Dependencies', 'href': modulename + '-dependencies.html'},
            {'text': 'Related Pages', 'href': 'pages.html'},
            ]
    # NOTE In Python 3 filter() builtin returns an iterable, but not a list
    #      type, so explicit conversion is here!
    return list(filter(
            lambda e: os.path.isfile(os.path.join(htmldir, e['href'])),
            entries))


def parse_dox_html(stream):
    """Parse the HTML files produced by Doxygen, extract the key/value block we
    add through header.html and return a dict ready for the Jinja template.

    The HTML files produced by Doxygen with our custom header and footer files
    look like this:

    @code
    <!--
    key1: value1
    key2: value2
    ...
    -->
    <html>
    <head>
    ...
    </head>
    <body>
    ...
    </body>
    </html>
    @endcode

    The parser fills the dict from the top key/value block, and add the content
    of the body to the dict using the "content" key.

    We do not use an XML parser because the HTML file might not be well-formed,
    for example if the documentation contains raw HTML.

    The key/value block is kept in a comment so that it does not appear in Qt
    Compressed Help output, which is not post processed by us.
    """
    dct = {}
    body = []

    def parse_key_value_block(line):
        if line == "<!--":
            return parse_key_value_block
        if line == "-->":
            return skip_head
        if line.startswith("<!DOCTYPE html"):
            return skip_head
        key, value = line.split(': ', 1)
        dct[key] = value
        return parse_key_value_block

    def skip_head(line):
        if line == "<body>":
            return extract_body
        else:
            return skip_head

    def extract_body(line):
        if line == "</body>":
            return None
        body.append(line)
        return extract_body

    parser = parse_key_value_block
    while parser is not None:
        line = stream.readline().rstrip()
        parser = parser(line)

    dct['content'] = '\n'.join(body)
    return dct


def postprocess_internal_qdoc(htmldir: str, tmpl: Template, env: Dict[str, Any]):
    """Substitute text in HTML files

    Performs text substitutions on each line in each .html file in a directory.

    Args:
        htmldir: (string) the directory containing the .html files.
        mapping: (dict) a dict of mappings.

    """
    for path in glob.glob(os.path.join(htmldir, "*.html")):
        newpath = f"{path}.new"

        txt = Path(path).read_text()
        env['docs'] = txt.partition('body')[2].partition('</body>')[0]

        with codecs.open(newpath, 'w', 'utf-8') as outf:
            try:
                html = tmpl.render(env)
            except BaseException:
                logging.error(f"Postprocessing {path} failed")
                raise

            outf.write(html)

        os.remove(path)
        os.rename(newpath, path)


def postprocess_internal(htmldir, tmpl, mapping):
    """Substitute text in HTML files

    Performs text substitutions on each line in each .html file in a directory.

    Args:
        htmldir: (string) the directory containing the .html files.
        mapping: (dict) a dict of mappings.

    """
    for name in os.listdir(htmldir):
        if name.endswith('.html'):
            path = os.path.join(htmldir, name)
            newpath = path + '.new'

            if name != 'classes.html' and name.startswith('class'):
                mapping['classname'] = name[5:-5].split('_1_1')[-1]
                mapping['fullname'] = name[5:-5].replace('_1_1', '::')
            elif name.startswith('namespace') and name != 'namespaces.html' and not name.startswith('namespacemembers'):
                mapping['classname'] = None
                mapping['fullname'] = name[9:-5].replace('_1_1', '::')
            else:
                mapping['classname'] = None
                mapping['fullname'] = None

            with codecs.open(path, 'r', 'utf-8', errors='ignore') as f:
                mapping['dox'] = parse_dox_html(f)

            with codecs.open(newpath, 'w', 'utf-8') as outf:
                try:
                    html = tmpl.render(mapping)
                except Exception:
                    logging.error('postprocessing {} failed'.format(path))
                    raise
                outf.write(html)
            os.remove(path)
            os.rename(newpath, path)


def build_classmap(tagfile):
    """Parses a tagfile to get a map from classes to files

    Args:
        tagfile: the Doxygen-generated tagfile to parse.

    Returns:
        A list of maps (keys: classname and filename).
    """
    import xml.etree.ElementTree as ET
    tree = ET.parse(tagfile)
    tagfile_root = tree.getroot()
    mapping = []
    for compound in tagfile_root:
        kind = compound.get('kind')
        if kind == 'class' or kind == 'namespace':
            name_el = compound.find('name')
            filename_el = compound.find('filename')
            mapping.append({'classname': name_el.text,
                            'filename': filename_el.text})
    return mapping


def generate_dependencies_page(tmp_dir, doxdatadir, modulename, dependency_diagram):
    """Create `modulename`-dependencies.md in `tmp_dir`"""
    template_path = os.path.join(doxdatadir, 'dependencies.md.tmpl')
    out_path = os.path.join(tmp_dir, modulename + '-dependencies.md')
    tmpl = load_template(template_path)
    with codecs.open(out_path, 'w', 'utf-8') as outf:
        txt = tmpl.render({
                'modulename': modulename,
                'diagramname': os.path.basename(dependency_diagram),
                })
        outf.write(txt)
    return out_path


def generate_apidocs_qdoc(ctx: Context, tmp_dir: str, doxyfile_entries=None, keep_temp_dirs=False):
    absolute = pathlib.Path(os.path.join(ctx.outputdir, 'html')).absolute()

    environ['KAPIDOX_DIR'] = ctx.doxdatadir

    logging.info(f'Running QDoc (qdoc {ctx.fwinfo.path}/.qdocconf --outputdir={absolute}')
    ret = subprocess.call(['qdoc', ctx.fwinfo.path + "/.qdocconf", f"--outputdir={absolute}"])
    if ret != 0:
        raise Exception("QDoc exited with a non-zero status code")


def generate_apidocs(ctx: Context, tmp_dir, doxyfile_entries=None, keep_temp_dirs=False):
    """Generate the API documentation for a single directory"""

    if ctx.is_qdoc:
        return generate_apidocs_qdoc(ctx, tmp_dir, doxyfile_entries, keep_temp_dirs)

    def find_src_subdir(dirlist, deeper_subd=None):
        returnlist = []
        for d in dirlist:
            pth = os.path.join(ctx.fwinfo.path, d)
            if deeper_subd is not None:
                pth = os.path.join(pth, deeper_subd)
            if os.path.isdir(pth) or os.path.isfile(pth):
                returnlist.append(pth)
            else:
                pass  # We drop it
        return returnlist

    input_list = []
    if os.path.isfile(ctx.fwinfo.path + "/Mainpage.dox"):
        input_list.append(ctx.fwinfo.path + "/Mainpage.dox")
    elif os.path.isfile(ctx.fwinfo.path + "/README.md"):
        input_list.append(ctx.fwinfo.path + "/README.md")

    input_list.extend(find_src_subdir(ctx.fwinfo.srcdirs))
    input_list.extend(find_src_subdir(ctx.fwinfo.docdir))
    image_path_list = []

    if ctx.dependency_diagram:
        input_list.append(generate_dependencies_page(tmp_dir,
                                                     ctx.doxdatadir,
                                                     ctx.modulename,
                                                     ctx.dependency_diagram))
        image_path_list.append(ctx.dependency_diagram)

    doxyfile_path = os.path.join(tmp_dir, 'Doxyfile')
    with codecs.open(doxyfile_path, 'w', 'utf-8') as doxyfile:
        # Global defaults
        with codecs.open(os.path.join(ctx.doxdatadir, 'Doxyfile.global'),
                         'r', 'utf-8') as f:
            for line in f:
                doxyfile.write(line)

        writer = DoxyfileWriter(doxyfile)
        writer.write_entry('PROJECT_NAME', ctx.fancyname)
        # FIXME: can we get the project version from CMake? No from GIT TAGS!

        # Input locations
        image_path_list.extend(find_src_subdir(ctx.fwinfo.docdir, 'pics'))
        writer.write_entries(
                INPUT=input_list,
                DOTFILE_DIRS=find_src_subdir(ctx.fwinfo.docdir, 'dot'),
                EXAMPLE_PATH=find_src_subdir(ctx.fwinfo.exampledirs),
                IMAGE_PATH=image_path_list)

        # Other input settings
        writer.write_entry('TAGFILES', [f + '=' + loc for f, loc in ctx.tagfiles])

        # Output locations
        writer.write_entries(
                OUTPUT_DIRECTORY=ctx.outputdir,
                GENERATE_TAGFILE=ctx.tagfile,
                HTML_OUTPUT=HTML_SUBDIR,
                WARN_LOGFILE=os.path.join(ctx.outputdir, WARN_LOGFILE))

        # Other output settings
        writer.write_entries(
                HTML_HEADER=ctx.doxdatadir + '/header.html',
                HTML_FOOTER=ctx.doxdatadir + '/footer.html'
                )

        # Set a layout so that properties are first
        writer.write_entries(
            LAYOUT_FILE=ctx.doxdatadir + '/DoxygenLayout.xml'
            )

        # Always write these, even if QHP is disabled, in case Doxygen.local
        # overrides it
        writer.write_entries(
                QHP_VIRTUAL_FOLDER=ctx.modulename,
                QHP_NAMESPACE="org.kde." + ctx.modulename,
                QHG_LOCATION=ctx.qhelpgenerator)

        writer.write_entries(
                GENERATE_MAN=ctx.man_pages,
                GENERATE_QHP=ctx.qhp)

        if doxyfile_entries:
            writer.write_entries(**doxyfile_entries)

        # Module-specific overrides
        if find_src_subdir(ctx.fwinfo.docdir):
            localdoxyfile = os.path.join(find_src_subdir(ctx.fwinfo.docdir)[0], 'Doxyfile.local')
            if os.path.isfile(localdoxyfile):
                with codecs.open(localdoxyfile, 'r', 'utf-8') as f:
                    for line in f:
                        doxyfile.write(line)

    logging.info('Running Doxygen')
    subprocess.call([ctx.doxygen, doxyfile_path])


def generate_diagram(png_path, fancyname, dot_files, tmp_dir):
    """Generate a dependency diagram for a framework.
    """
    def run_cmd(cmd, **kwargs):
        try:
            subprocess.check_call(cmd, **kwargs)
        except subprocess.CalledProcessError as exc:
            logging.error(f'Command {exc.cmd} failed with error code {exc.returncode}.')
            return False
        return True

    logging.info('Generating dependency diagram')
    dot_path = os.path.join(tmp_dir, fancyname + '.dot')

    with open(dot_path, 'w') as f:
        with_qt = False
        ok = depdiagram.generate(f, dot_files, framework=fancyname,
                                 with_qt=with_qt)
        if not ok:
            logging.error('Generating diagram failed')
            return False

    logging.info('- Simplifying diagram')
    simplified_dot_path = os.path.join(tmp_dir, fancyname + '-simplified.dot')
    with open(simplified_dot_path, 'w') as f:
        if not run_cmd(['tred', dot_path], stdout=f):
            return False

    logging.info('- Generating diagram png')
    if not run_cmd(['dot', '-Tpng', '-o' + png_path, simplified_dot_path]):
        return False

    # These os.unlink() calls are not in a 'finally' block on purpose.
    # Keeping the dot files around makes it possible to inspect their content
    # when running with the --keep-temp-dirs option. If generation fails and
    # --keep-temp-dirs is not set, the files will be removed when the program
    # ends because they were created in `tmp_dir`.
    os.unlink(dot_path)
    os.unlink(simplified_dot_path)
    return True


def create_fw_context(args, lib, tagfiles, copyright=''):

    # There is one more level for groups
    if lib.part_of_group:
        corrected_tagfiles = []
        for k in range(len(tagfiles)):
            # tagfiles are tuples like:
            # ('/usr/share/doc/qt/KF5Completion.tags', '/usr/share/doc/qt')
            # ('/where/the/tagfile/is/Name.tags', '/where/the/root/folder/is')
            if tagfiles[k][1].startswith("http://") or tagfiles[k][1].startswith("https://"):
                corrected_tagfiles.append(tagfiles[k])
            else:
                corrected_tagfiles.append((tagfiles[k][0], '../' + tagfiles[k][1]))
    else:
        corrected_tagfiles = tagfiles

    return Context(args,
                   # Names
                   modulename=lib.name,
                   fancyname=lib.fancyname,
                   fwinfo=lib,
                   # KApidox files
                   resourcedir=('../../../resources' if lib.part_of_group
                                else '../../resources'),
                   # Input
                   copyright=copyright,
                   tagfiles=corrected_tagfiles,
                   dependency_diagram=lib.dependency_diagram,
                   # Output
                   outputdir=lib.outputdir,
                   is_qdoc=lib.metainfo['qdoc'],
                   )


def gen_fw_apidocs(ctx, tmp_base_dir):
    create_dirs(ctx)
    # tmp_dir is deleted when tmp_base_dir is
    tmp_dir = tempfile.mkdtemp(prefix=ctx.modulename + '-', dir=tmp_base_dir)
    generate_apidocs(ctx, tmp_dir,
                     doxyfile_entries=dict(WARN_IF_UNDOCUMENTED=True)
                     )


def create_fw_tagfile_tuple(lib):
    tagfile = os.path.abspath(
                os.path.join(
                    lib.outputdir,
                    'html',
                    lib.fancyname+'.tags'))
    if lib.part_of_group:
        prefix = '../../'
    else:
        prefix = '../../'
    return tagfile, prefix + lib.outputdir + '/html/'


def finish_fw_apidocs_doxygen(ctx: Context, env: Dict[str, Any]):
    tmpl = create_jinja_environment(ctx.doxdatadir).get_template('library.html')
    postprocess_internal(ctx.htmldir, tmpl, env)

    tmpl2 = create_jinja_environment(ctx.doxdatadir).get_template('search.html')
    search_output = ctx.fwinfo.outputdir + "/html/search.html"
    with codecs.open(search_output, 'w', 'utf-8') as outf:
        outf.write(tmpl2.render(env))


def finish_fw_apidocs_qdoc(ctx: Context, env: Dict[str, Any]):
    tmpl = create_jinja_environment(ctx.doxdatadir).get_template('qdoc-wrapper.html')
    postprocess_internal_qdoc(ctx.htmldir, tmpl, env)


def gen_template_environment(ctx: Context) -> Dict[str, Any]:
    classmap = build_classmap(ctx.tagfile)

    entries = [{
        'href': '../../index.html',
        'text': 'KDE API Reference'
    }]

    if ctx.fwinfo.part_of_group:
        entries[0]['href'] = '../' + entries[0]['href']
        entries.append({'href': '../../index.html', 'text': ctx.fwinfo.product.fancyname })

    entries.append({'href': 'index.html', 'text': ctx.fancyname })

    mapping = {
        'qch': ctx.qhp,
        'doxygencss': 'doxygen.css',
        'resources': ctx.resourcedir,
        'title': ctx.title,
        'fwinfo': ctx.fwinfo,
        'copyright': f"1996-{datetime.date.today().year} The KDE developers",
        'doxygen_menu': {'entries': menu_items(ctx.htmldir, ctx.modulename)},
        'class_map': {'classes': classmap},
        'kapidox_version': utils.get_kapidox_version(),
        'breadcrumbs': {
            'entries': entries
        },
    }

    return mapping


def finish_fw_apidocs(ctx: Context):
    env = gen_template_environment(ctx)

    if ctx.is_qdoc:
        logging.info('Postprocessing QtDoc...')

        finish_fw_apidocs_qdoc(ctx, env)

    else:
        logging.info('Postprocessing Doxygen...')

        finish_fw_apidocs_doxygen(ctx, env)


def indexer(lib):
    """ Create json index from xml
      <add>
        <doc>
          <field name="type">source</field>
          <field name="name">kcmodule.cpp</field>
          <field name="url">kcmodule_8cpp_source.html#l00001</field>
          <field name="keywords"></field>
          <field name="text"></field>
        </doc>
      </add>
    """

    doclist = []
    tree = xmlET.parse(lib.outputdir + '/searchdata.xml')
    for doc_child in tree.getroot():
        field = {}
        for child in doc_child:
            if child.attrib['name'] == "type":
                if child.text == 'source':
                    field = None
                    break  # We go to next <doc>
                field['type'] = child.text
            elif child.attrib['name'] == "name":
                field['name'] = child.text
            elif child.attrib['name'] == "url":
                field['url'] = child.text
            elif child.attrib['name'] == "keywords":
                field['keyword'] = child.text
            elif child.attrib['name'] == "text":
                field['text'] = "" if child.text is None else child.text
        if field is not None:
            doclist.append(field)

    indexdic = {
        'name': lib.name,
        'fancyname': lib.fancyname,
        'docfields': doclist
        }

    with open(lib.outputdir + '/html/searchdata.json', 'w') as f:
        for chunk in json.JSONEncoder().iterencode(indexdic):
            f.write(chunk)


def create_product_index(product):
    doclist = []
    for lib in product.libraries:
        with open(lib.outputdir+'/html/searchdata.json', 'r') as f:
            libindex = json.load(f)
            for item in libindex['docfields']:
                if lib.part_of_group:
                    item['url'] = lib.name.lower() + '/html/' + item['url']
                else:
                    item['url'] = 'html/' + item['url']
            doclist.append(libindex)

    indexdic = {
        'name': product.name,
        'fancyname': product.fancyname,
        'libraries': doclist
        }

    with open(product.outputdir + '/searchdata.json', 'w') as f:
        for chunk in json.JSONEncoder().iterencode(indexdic):
            f.write(chunk)


def create_global_index(products):
    doclist = []
    for product in products:
        if product.metainfo['qdoc']:
            continue

        with open(product.outputdir+'/searchdata.json', 'r') as f:
            prodindex = json.load(f)
            for proditem in prodindex['libraries']:
                for item in proditem['docfields']:
                    item['url'] = os.path.join(product.name, item['url'])
            doclist.append(prodindex)

    indexdic = {
        'all': doclist
        }
    with open('searchdata.json', 'w') as f:
        for chunk in json.JSONEncoder().iterencode(indexdic):
            f.write(chunk)


def create_qch(products, tagfiles):
    tag_root = "QtHelpProject"
    tag_files = "files"
    tag_filter_section = "filterSection"
    tag_keywords = "keywords"
    tag_toc = "toc"
    for product in products:
        tree_out = ET.ElementTree(ET.Element(tag_root))
        root_out = tree_out.getroot()
        root_out.set("version", "1.0")
        namespace = ET.SubElement(root_out, "namespace")
        namespace.text = "org.kde." + product.name
        virtual_folder = ET.SubElement(root_out, "virtualFolder")
        virtual_folder.text = product.name
        filter_section = ET.SubElement(root_out, tag_filter_section)
        filter_attribute = ET.SubElement(filter_section, "filterAttribute")
        filter_attribute.text = "doxygen"
        toc = ET.SubElement(filter_section, "toc")
        keywords = ET.SubElement(filter_section, tag_keywords)
        if len(product.libraries) > 0:
            if product.libraries[0].part_of_group:
                product_index_section = ET.SubElement(toc, "section", {'ref': product.name + "/index.html", 'title': product.fancyname})
        files = ET.SubElement(filter_section, tag_files)

        for lib in sorted(product.libraries, key=lambda lib: lib.name):
            tree = ET.parse(lib.outputdir + '/html/index.qhp')
            root = tree.getroot()
            for child in root.findall(".//*[@ref]"):
                if lib.part_of_group:
                    child.attrib['ref'] = lib.name + "/html/" + child.attrib['ref']
                else:
                    child.attrib['ref'] = "html/" + child.attrib['ref']
                child.attrib['ref'] = product.name + '/' + child.attrib['ref']

            for child in root.find(".//"+tag_toc):
                if lib.part_of_group:
                    product_index_section.append(child)
                else:
                    toc.append(child)

            for child in root.find(".//keywords"):
                keywords.append(child)

            resources = [
                "*.json",
                product.name + "/*.json",
                "resources/css/*.css",
                "resources/3rd-party/bootstrap/css/*.css",
                "resources/3rd-party/jquery/jquery-3.1.0.min.js",
                "resources/*.svg",
                "resources/js/*.js",
                "resources/icons/*",
            ]
            if product.part_of_group:
                resources.extend([
                    product.name + "/*.html",
                    product.name + "/" + lib.name + "/html/*.html",
                    product.name + "/" + lib.name + "/html/*.png",
                    product.name + "/" + lib.name + "/html/*.css",
                    product.name + "/" + lib.name + "/html/*.js",
                    product.name + "/" + lib.name + "/html/*.json"
                    ])

            else:
                resources.extend([
                    product.name + "/html/*.html",
                    product.name + "/html/*.png",
                    product.name + "/html/*.css",
                    product.name + "/html/*.js"
                    ])

            for resource in resources:
                file_elem = ET.SubElement(files, "file")
                file_elem.text = resource

        if not os.path.isdir('qch'):
            os.mkdir('qch')

        name = product.name+".qhp"
        outname = product.name+".qch"
        tree_out.write(name, encoding="utf-8", xml_declaration=True)

        # On many distributions, qhelpgenerator from Qt5 is suffixed with
        # "-qt5". Look for it first, and fall back to unsuffixed one if
        # not found.
        qhelpgenerator = shutil.which("qhelpgenerator-qt5")

        if qhelpgenerator is None:
            qhelpgenerator = "qhelpgenerator"

        subprocess.call([qhelpgenerator, name, '-o', 'qch/'+outname])
        os.remove(name)
