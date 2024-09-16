# dci-icon-theme
````
Usage: dci-icon-theme [options] ~/dci-png-icons
dci-icon-theme tool is a command tool that generate dci icons from common icons.
For example, the tool is used in the following ways:
     dci-icon-theme /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95
     dci-icon-theme -m *.png /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95
     dci-icon-theme --fix-dark-theme <input dci files directory> -o <output directory path>
     dci-icon-theme --find <icon name>
     dci-icon-theme --find <icon name> -t bloom
     dci-icon-theme <input file directory> -o <output directory path> -s <csv file> -O <qualities>


Options:
  -m, --match <wildcard palette>       Give wildcard rules on search icon
                                       files, Each eligible icon will be
                                       packaged to a dci file, If the icon have
                                       the dark mode, it needs to store the dark
                                       icon file at "dark/" directory relative
                                       to current icon file, and the file name
                                       should be consistent.
  -o, --output <directory>             Save the *.dci files to the given
                                       directory.
  -s, --symlink <csv file>             Give a csv file to create symlinks for
                                       the output icon file.
                                       The content of symlink.csv like:
                                         sublime-text, com.sublimetext.2
                                         deb, "
                                         application-vnd.debian.binary-package
                                         application-x-deb
                                         gnome-mime-application-x-deb
                                         "

  --fix-dark-theme                     Create symlinks from light theme for
                                       dark theme files.
  --find                               Find dci icon file path
  -t, --theme <theme name>             Give a theme name to find dci icon file
                                       path
  -O, --scale-quality <scale quality>  Quility of dci scaled icon image
                                       The value may like <scale size>=<quality
                                       value>  e.g. 2=98:3=95
                                       The quality factor must be in the range 0
                                       to 100 or -1.
                                       Specify 0 to obtain small compressed
                                       files, 100 for large uncompressed files
                                       and -1 to use the image handler default
                                       settings.
                                       The higher the quality, the larger the
                                       dci icon file size
  -h, --help                           Displays help on commandline options.
  --help-all                           Displays help including Qt specific
                                       options.
  -v, --version                        Displays version information.

Arguments:
  source                               Search the given directory and it's
                                       subdirectories, get the files conform to
                                       rules of --match.
````
