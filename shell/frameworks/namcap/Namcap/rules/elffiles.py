# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from elftools.elf.elffile import ELFFile
from elftools.elf.dynamic import DynamicSection
from elftools.elf.enums import ENUM_GNU_PROPERTY_X86_FEATURE_1_FLAGS
from elftools.elf.sections import NoteSection, SymbolTableSection

from Namcap.util import is_elf
from Namcap.ruleclass import TarballRule

# Valid directories for ELF files
valid_dirs = ["bin/", "sbin/", "usr/bin/", "usr/sbin/", "lib/", "usr/lib/", "usr/lib32/"]
# Questionable directories for ELF files
# (Suppresses some output spam.)
questionable_dirs = ["opt/"]


def elf_files_from_tar(tar):
    for entry in tar:
        if not entry.isfile():
            continue
        fileobj = tar.extractfile(entry)
        if not fileobj or not is_elf(fileobj):
            continue
        yield ELFFile(fileobj), entry.name


class ELFPaths(TarballRule):
    name = "elfpaths"
    description = "Check about ELF files outside some standard paths."

    def analyze(self, pkginfo, tar):
        invalid_elffiles = []
        questionable_elffiles = []

        for entry in tar:
            # is it a regular file ?
            if not entry.isfile():
                continue
            # is it outside standard binary dirs ?
            in_std_dirs = any(entry.name.startswith(d) for d in valid_dirs)
            in_que_dirs = any(entry.name.startswith(d) for d in questionable_dirs)

            if in_std_dirs:
                continue
            # is it an ELF file ?
            f = tar.extractfile(entry)
            if is_elf(f):
                if in_que_dirs:
                    questionable_elffiles.append(entry.name)
                else:
                    invalid_elffiles.append(entry.name)

        que_elfdirs = [d for d in questionable_dirs if any(f.startswith(d) for f in questionable_elffiles)]
        self.errors = [("elffile-not-in-allowed-dirs %s", (i,)) for i in invalid_elffiles]
        self.errors.extend(("elffile-in-questionable-dirs %s", (i,)) for i in que_elfdirs)
        self.infos = [("elffile-not-in-allowed-dirs %s", (i,)) for i in questionable_elffiles]


class ELFTextRelocationRule(TarballRule):
    """
    Check for text relocations in ELF files.
    """

    name = "elftextrel"
    description = "Check for text relocations in ELF files."

    def analyze(self, pkginfo, tar):
        files_with_textrel = []

        for elffile, entry_name in elf_files_from_tar(tar):
            for section in elffile.iter_sections():
                if not isinstance(section, DynamicSection):
                    continue
                for tag in section.iter_tags("DT_TEXTREL"):
                    if tag.entry.d_tag == "DT_TEXTREL":
                        files_with_textrel.append(entry_name)

        if files_with_textrel:
            self.warnings = [("elffile-with-textrel %s", (i,)) for i in files_with_textrel]


class ELFExecStackRule(TarballRule):
    """
    Check for executable stacks in ELF files.

    Introduced by FS#26458. Uses pyelftools to read the GNU_STACK
    program header and ensure it does not have the executable bit
    set.
    """

    name = "elfexecstack"
    description = "Check for executable stacks in ELF files."

    def analyze(self, pkginfo, tar):
        exec_stacks = []

        for elffile, entry_name in elf_files_from_tar(tar):
            for segment in elffile.iter_segments():
                if segment["p_type"] != "PT_GNU_STACK":
                    continue

                mode = segment["p_flags"]
                if mode & 1:
                    exec_stacks.append(entry_name)

        if exec_stacks:
            self.warnings = [("elffile-with-execstack %s", (i,)) for i in exec_stacks]


class ELFGnuRelroRule(TarballRule):
    """
    Check for read-only relocation in ELF files.

    Introduced by FS#26435. Uses pyelftools to check for GNU_RELRO.
    """

    name = "elfgnurelro"
    description = "Check for FULL RELRO in ELF files."

    def has_bind_now(self, elffile):
        DF_BIND_NOW = 0x08

        for section in elffile.iter_sections():
            if not isinstance(section, DynamicSection):
                continue
            for tag in section.iter_tags():
                if tag.entry.d_tag == "DT_BIND_NOW":
                    return True
                if tag.entry.d_tag == "DT_FLAGS" and tag.entry.d_val & DF_BIND_NOW:
                    return True
        return False

    def analyze(self, pkginfo, tar):
        missing_relro = []

        for elffile, entry_name in elf_files_from_tar(tar):
            if ".debug" in entry_name:
                continue

            if any(seg["p_type"] == "PT_GNU_RELRO" for seg in elffile.iter_segments()):
                if self.has_bind_now(elffile):
                    continue

            missing_relro.append(entry_name)

        if missing_relro:
            self.warnings = [("elffile-without-relro %s", (i,)) for i in missing_relro]


class ELFUnstrippedRule(TarballRule):
    """
    Checks for unstripped ELF files. Uses pyelftools to check if
    .symtab exists.

    Introduced by FS#27485.
    """

    name = "elfunstripped"
    description = "Check for unstripped ELF files."

    def analyze(self, pkginfo, tar):
        unstripped_binaries = []

        for elffile, entry_name in elf_files_from_tar(tar):
            if ".debug" in entry_name:
                continue
            for section in elffile.iter_sections():
                if not isinstance(section, SymbolTableSection):
                    continue

                if section["sh_entsize"] == 0:
                    continue

                if section.name == ".symtab":
                    unstripped_binaries.append(entry_name)
        if unstripped_binaries:
            self.warnings = [("elffile-unstripped %s", (i,)) for i in unstripped_binaries]


class NoPIERule(TarballRule):
    """
    Checks for no PIE ELF files.
    """

    name = "elfnopie"
    description = "Check for no PIE ELF files."

    def has_dt_debug(self, elffile):
        for section in elffile.iter_sections():
            if not isinstance(section, DynamicSection):
                continue
            for _ in section.iter_tags("DT_DEBUG"):
                return True
        return False

    def analyze(self, pkginfo, tar):
        nopie_binaries = []

        for elffile, entry_name in elf_files_from_tar(tar):
            if any(x in entry_name for x in [".so", ".debug"]):
                continue
            if elffile.header["e_type"] != "ET_DYN" or not self.has_dt_debug(elffile):
                nopie_binaries.append(entry_name)

        if nopie_binaries:
            self.warnings = [("elffile-nopie %s", (i,)) for i in nopie_binaries]


def _note_props(elffile, note_type, prop_type):
    for section in elffile.iter_sections():
        if not isinstance(section, NoteSection):
            continue
        for note in section.iter_notes():
            if note["n_type"] != note_type:
                continue
            for prop in note["n_desc"]:
                if prop["pr_type"] == prop_type:
                    yield prop


class ELFSHSTKRule(TarballRule):
    """
    Check shadow stack support in ELF files.
    """

    #  This lint functions properly, but given that many/most language
    #  ecosystems don't have support yet this turns up more too many false
    #  positives to be useful. Enable at a later date when tooling supports it
    #  and our warning message points in the right direction to make it work.
    enable = False

    name = "elfnoshstk"
    description = "Check for shadow stack support in ELF files."

    def analyze(self, pkginfo, tar):
        noshstk_binaries = []

        for elffile, entry_name in elf_files_from_tar(tar):
            if ".debug" in entry_name or elffile["e_machine"] != "EM_X86_64":
                continue
            for prop in _note_props(
                elffile,
                note_type="NT_GNU_PROPERTY_TYPE_0",
                prop_type="GNU_PROPERTY_X86_FEATURE_1_AND",
            ):
                if prop["pr_data"] & ENUM_GNU_PROPERTY_X86_FEATURE_1_FLAGS["GNU_PROPERTY_X86_FEATURE_1_SHSTK"]:
                    break

            else:
                noshstk_binaries.append(entry_name)
        if noshstk_binaries:
            self.warnings = [("elffile-noshstk %s", (i,)) for i in noshstk_binaries]
