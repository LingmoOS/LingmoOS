Name:           deepin-gettext-tools
Version:        1.0.9
Release:        1
Summary:        Deepin Gettext Tools
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-gettext-tools
Source0:        %{name}-%{version}.orig.tar.gz

BuildArch:      noarch
BuildRequires:  perl-XML-LibXML
BuildRequires:  python3-devel
BuildRequires:  perl(Config::Tiny)
Requires:       gettext
Requires:       qt5-linguist
Requires:       perl(Config::Tiny)
Requires:       perl(Exporter::Tiny)
Requires:       perl(XML::LibXML)
Requires:       perl(XML::LibXML::PrettyPrint)

%description
The tools of gettext function wrapper.

desktop-ts-convert - handling desktop file translations.
policy-ts-convert - convert PolicyKit Policy file to the ts file.
update-pot - scan msgid and generate pot file according to the ini file.
generate-mo - scan po files and generate mo files according to the ini file.

%prep
%setup -q

# fix shebang
find -iname "*.py" | xargs sed -i '1s|.*|#!%{__python3}|'
sed -i '1s|.*|#!%{__perl}|' src/desktop_ts_convert.pl

sed -i 's|sudo cp|cp|' src/generate_mo.py
sed -i 's|lconvert|lconvert-qt5|; s|deepin-lupdate|lupdate-qt5|' src/update_pot.py

%build

%install
install -d %{buildroot}%{_bindir}
install -m755 src/desktop_ts_convert.pl %{buildroot}%{_bindir}/deepin-desktop-ts-convert
install -m755 src/policy_ts_convert.py %{buildroot}%{_bindir}/deepin-policy-ts-convert
install -m755 src/generate_mo.py %{buildroot}%{_bindir}/deepin-generate-mo
install -m755 src/update_pot.py %{buildroot}%{_bindir}/deepin-update-pot

%check
/bin/perl src/desktop_ts_convert.pl --help
/bin/python3 src/generate_mo.py --help
/bin/python3 src/update_pot.py --help

%files
%doc README.md
%license LICENSE
%{_bindir}/deepin-desktop-ts-convert
%{_bindir}/deepin-policy-ts-convert
%{_bindir}/deepin-update-pot
%{_bindir}/deepin-generate-mo

%changelog
* Thu Mar 23 2021 uoser <uoser@uniontech.com> - 1.0.9-1
- Update to 1.0.9
