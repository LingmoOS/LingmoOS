#compdef plasmoidviewer

# SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# keep this function in sync with plasmashell from plasma-workspace
function _plasma_list_packages() {
  local expl global_dir descr="$1" purpose="$2" type="${3:-Plasma/Applet}"
  local -a packages containments
  # Then strip " -- " prefix and " *" suffix (i.e. everything after and including first whitespace).
  # (@) array Expansion Flags are needed because we need to operate on each element separately.
  packages=( ${(f)"$(
    _call_program plasma-list-packages \
      kpackagetool6 --type "$type" --list --global
    )"} )
  # First line is an output header, it contains path where plugins are probed.
  global_dir="${${${packages[1]}##* in }%/}"
  packages[1]=()

  if [[ "$purpose" == "containment" ]]; then
    # No need to get jq involved. We simply look for a magic string, which
    # hopefully won't be named literally in any other irrelevant field.
    containments=( ${${${(f)"$(
      # Glob Qualifiers (.) and (@r) match plain files and readable symbolic links;
      # (N) sets the NULL_GLOB option for the current pattern, so it doesn't print
      # an error for non-existent and empty directories.
      grep --files-with-matches -R "Plasma/Containment" $global_dir/*/metadata.json(N.,@r)
    )"}#$global_dir/}%/metadata.json} )  # strip back prefix and suffix
    packages=( $containments )  # directory name is the package id
  else
    # Don't filter containments out in an 'else' branch: they might be perfectly
    # executable applets on their own, e.g. org.kde.plasma.folder view.
  fi

  _wanted "$descr" expl "$purpose" \
    compadd -a packages
}

_arguments \
  '(-c --containment)'{-c=,--containment=}'[override containment package id]::: _plasma_list_packages containments containment' \
  '(-a --applet)'{-a=,--applet=}'[applet package id]::: _plasma_list_packages applets applet' \
  '(-f --formfactor)'{-f=,--formfactor=}'[form factor]:form-factor:(horizontal vertical mediacenter planar application)' \
  '(-l --location)'{-l=,--location=}'[location constraint]::location:(floating desktop fullscreen topedge bottomedge leftedge rightedge)' \
  '(-x --xPosition)'{-x=,--xPosition=}'[set window x position on screen]:::_numbers' \
  '(-y --yPosition)'{-y=,--yPosition=}'[set window y position on screen]:::_numbers' \
  '(-s --size)'{-s=,--size=}'[set window size]:size widthXheight:' \
  '(-p --pixmapcache)'{-p=,--pixmapcache=}'[size (in kB) of the pixmap cache]:size in kB::_numbers' \
  '(-t --theme)'{-t=,--theme=}'[override theme which the shell will use]::: _plasma_list_packages themes theme Plasma/Theme' \
  '(-)'{-h,--help}'[print usage information and exit]' \
  '(-)'{-v,--version}'[displays version information]'
