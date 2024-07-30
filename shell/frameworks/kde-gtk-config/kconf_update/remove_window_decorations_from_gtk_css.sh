#!/usr/bin/env sh

remove_window_decorations_import() {
  if [ -z "$XDG_CONFIG_HOME" ]; then
    GTK_CSS_PATH="$HOME/.config/gtk-3.0/gtk.css"
  else
    GTK_CSS_PATH="$XDG_CONFIG_HOME/gtk-3.0/gtk.css"
  fi

  sed -i "/@import 'window_decorations.css';/d" "$GTK_CSS_PATH"
}

remove_window_decorations_import
