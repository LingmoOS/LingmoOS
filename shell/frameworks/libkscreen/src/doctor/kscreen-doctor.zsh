#compdef kscreen-doctor

# SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

local curcontext="$curcontext" state expl ret=1

_kscreen-doctor-check-jq() {
  local tag=$1 descr=$2

  if (( $+commands[jq] )); then
    return 0
  else
    local -a empty  # Is there a better way to print description?
    _describe -t "$tag" "$descr (completions unavailable, please install jq)" empty
    return 1
  fi
}

_kscreen-doctor-outputs() {
  local -a outputs  # array of triples: id, name, enabled status (true/false)

  _kscreen-doctor-check-jq outputs output || return 1

  data=(${(f)"$(
    kscreen-doctor --json |
    jq --raw-output '
        .outputs
        | map(select(.connected))
        | sort_by((.enabled | not), .name)
        | map(.id, .name, .enabled)
        | .[]
      '
    )"})

  local id name enabled desc
  for id name enabled in $data ; do
    if [[ "$enabled" == true ]]; then
      enabled="[Enabled] "  # a bit of right padding, like git does for recent commits list
    else
      enabled="[Disabled]"
    fi
    desc="$enabled Output ID $id, connected as $name"
    # Duplicate completions for id and name. But given identical description,
    # they will occupy the same row.
    outputs+=( "$id:$desc" "$name:$desc" )
  done

  _describe -t outputs "output" outputs "$@" -V unsorted
}

_kscreen-mode-fmt() {
  local dst="$1" mode="$2"
  local size refresh_rate

  size=${mode%@*}
  refresh_rate=${mode#*@}
  # WWWWxHHHH @ RRR, (l:n:) is padding with spaces on the left
  printf -v $dst " ${(l:9:)size} @ ${(l:3:)refresh_rate} "
}

_kscreen-doctor-mode() {
  local ret=1 output="$1"

  _kscreen-doctor-check-jq modes mode || return $ret

  # 'top_comp' is a list of current and preferred mode IDs (literal completions).
  # 'top_descr' is a list of pretty-printed version of top_comp and their descriptions.
  # 'rest_comp' is a list of mode IDs, and 'rest_fmt' is pretty-printed version of rest (without descriptions).
  local -a stdout top_comp top_descr rest_comp rest_fmt
  stdout=(${(f)"$(
    kscreen-doctor --json |
    jq --raw-output --arg output "$output" '
        .outputs[]
        # note about "\(.id)": stringifying known-to-be integer sounds safer
        # than parsing untrusted input as an int.
        | select("\(.id)" == $output or .name == $output)
        | [
            .preferredModes as $pref
          | .currentModeId as $curr
          | .modes
          | map({
              id: .id,
              w: .size.width,
              h: .size.height,
              r: (.refreshRate | round),
            })
          # Some names may be duplicated after rounding refresh rates.
          # Group them by what going to be part of a name,
          # while keeping a list of IDs to be able to determine which ones are "current" and/or "preferred".
          | group_by(.w, .h, .r)
          | map({  # flatten back
              p: any(
                # this is how you do a nested loop
                .id as $id | $pref[] as $p | $id == $p
              ),
              c: any(.id == $curr),
              # Just take the first mode`s data. They are identical, and there will always be at least one.
              w: .[0].w,
              h: .[0].h,
              r: .[0].r
            })
          | sort_by(.w, .h, .r)
          | reverse
          | map({ name: "\(.w)x\(.h)@\(.r)", p: .p, c: .c })
          # show current mode on top, then preferred, then the rest.
          # Second line is a flag that indicates whether current mode is also the preferred one.
          | map(select(.c) | "\(.name);\(.p)")[],
            "--",
            map(select((.c | not) and .p) | .name)[],
            "++",
            map(select((.c | not) and (.p | not)) | .name)[]
        ][]
      '
    )"})
  # sample result:
  #   4096x2160@60;false
  #   --
  #   1920x1080@60
  #   ++
  #   4096x2160@50
  #   4096x2160@30
  #   3840x2160@60
  #   3840x2160@50
  #   3840x2160@30
  #   1920x1080@50
  #   1920x1080@30
  #   1920x1080@25
  #   ...
  # The 'false' on a first line indicates that current is not also a
  # preferred one. If it were, it would not appear in the list of preferred
  # modes below.

  local current label formatted line parser=current  # then "preferred" and "rest"
  for line in $stdout ; do
    case $line in
      --)
        parser=preferred
        continue
        ;;
      ++)
        parser=rest
        continue
    esac
    case $parser in
      current)
        current=${line%;*}
        _kscreen-mode-fmt formatted "$current"
        label="Current"
        # current is also preferred
        if [[ "${line#*;}" != "false" ]]; then
          label="$label & Preferred"
        fi
        top_comp+=( "$current" )
        top_descr+=( "${formatted}:${label} mode" )
        ;;
      preferred)
        _kscreen-mode-fmt formatted "$line"
        top_comp+=( "$line" )
        top_descr+=( "${formatted}:Preferred mode" )
        ;;
      rest)
        _kscreen-mode-fmt formatted "$line"
        rest_comp+=( $line )
        rest_fmt+=( $formatted )
    esac
  done

  ret=1
  # Resetting expl to avoid it 'leaking' from one line to the next.
  expl=()
  _describe -V -t notable-modes 'notable modes' top_descr top_comp && ret=0
  expl=()
  _wanted all-modes expl 'other available modes' \
    compadd -o nosort -d rest_fmt -a rest_comp \
    && ret=0
  return $ret
}

_kscreen-doctor-priorities() {
  local connected_count
  if (( $+commands[jq] )); then
    connected_count="$(
      kscreen-doctor --json |
      jq --raw-output '
        .outputs
        | map(select(.connected))
        | length
      ')"
  else
    # best effort fallback
    connected_count="$(kscreen-doctor --outputs | wc -l)"
  fi
  _alternative "priority::( {0..${connected_count}} )"
}

_arguments -C \
    '(-h --help)'{-h,--help}'[Displays help on commandline options]' \
    '--help-all[Displays help including Qt specific options]' \
    '(-i --info)'{-i,--info}'[Show runtime information: backends, logging, etc]' \
    '(-j --json)'{-j,--json}'[Show configuration in JSON format]' \
    '(-o --outputs)'{-o,--outputs}'[Show outputs]' \
    '(DPMS)'{-d=,--dpms=}'[(Wayland only) Display power management]:status:(on off)' \
    '--dpms-excluded=[Do not apply the dpms change to the output with said model names]:output:_kscreen-doctor-outputs' \
    '(-l --log)'{-l=,--log=}'[Write a comment to the log file]:comment' \
    '*: :->settings' && ret=0

case $state in
  settings)
    if compset -P 'output.' ; then

      if compset -P 1 '*.' ; then
        local output; output="${${IPREFIX#*.}%.}"

        if compset -P 1 'mode.' ; then
          _kscreen-doctor-mode "$output" && ret=0
        elif compset -P 1 'priority.' ; then
          _kscreen-doctor-priorities && ret=0
        elif compset -P 1 'position.' ; then
          _arguments '1::x,y:' && ret=0
        elif compset -P 1 'rgbrange.' ; then
          _alternative 'rgbrange::(automatic full limited)' && ret=0
        elif compset -P 1 'rotation.' || compset -P 1 'orientation.' ; then
          _alternative 'rotation::(none normal left right inverted flipped flipped90 flipped180 flipped270)' && ret=0
        elif compset -P 1 'overscan.' ; then
          local -a overscan_descr overscan_comp
          overscan_descr=(
            ' 0%:Disable overscan (Default)'
            ' 3%:Action safe area (Vertical, round down)'
            ' 4%:Action safe area (Vertical, round up)'
            '10%:Action safe area (Horizontal, 14:9 displayed on 16:9)'
            '15%:Action safe area (Horizontal,  4:3 displayed on 16:9)'
            '17%:Title safe area  (Horizontal,  4:3 displayed on 16:9)'
          )
          overscan_comp=('0' '3' '4' '10' '15' '17')
          _describe -t overscan "(Wayland only) output overscan, 0%%..100%%" \
            overscan_descr overscan_comp -o nosort && ret=0
        elif compset -P 1 'scale.' ; then
          local -a scale_descr scale_comp
          scale_descr=('100%' '125%' '150%' '175%' '200%' '225%' '250%' '275%' '300%' )
          scale_comp=( '1'    '1,25' '1,5'  '1,75' '2'    '2,25' '2,5'  '2,75' '3'    )
          _describe -t scale "(Wayland only) per-output scaling" scale_descr scale_comp -o nosort && ret=0
        elif compset -P 1 'vrrpolicy.' ; then
          _alternative 'vrrpolicy::(never always automatic)' && ret=0
        elif compset -P 1 'hdr.' ; then
          _alternative 'hdr::(enable disable)' && ret=0
        elif compset -P 1 'sdr-brightness.' ; then
          _numbers -t 'sdr-brightness' -u nits -l 100 -m 10000 'SDR brightness' && ret=0
        elif compset -P 1 'wcg.' ; then
          _alternative 'wcg::(enable disable)' && ret=0
        else
          # two groups: first without suffix, and second with '.' at the end
          _describe -t subcommands 'subcommand' '(
            enable:"Toggle output"
            disable:"Toggle output"
            primary:"Make this output primary (same as priority.1)"
          )' -- '(
            mode:"Resolution and refresh rate"
            orientation:"Display orientation"
            overscan:"(Wayland only) Overscan area size"
            position:"x and y coordinates"
            priority:"Set output priority"
            rgbrange:"RGB range"
            rotation:"Display orientation"
            scale:"(Wayland only) Per-output scaling"
            vrrpolicy:"(Wayland only) Variable refresh rate"
            hdr:"(Wayland only) High Dynamic Range"
            sdr-brightness:"(Wayland only) Brightness of Standard Dynamic Range content"
            wcg:"(Wayland only) Wide Color Gamut"
          )' -S '.' && ret=0
        fi
      else
        _kscreen-doctor-outputs -S '.' && ret=0
      fi
    else
      _sep_parts '(output)' . && ret=0
    fi
  ;;
esac

return $ret
