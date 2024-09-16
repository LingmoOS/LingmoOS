Name:           deepin-gtk-theme
Version:        21.04.23
Release:        1
Summary:        Deepin GTK Theme
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-gtk-theme
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz
BuildArch:      noarch

%description
%{summary}.

%prep
%setup -q

%build

%install
%make_install PREFIX=%{_prefix}

%files
%doc README.md
%license LICENSE
%{_datadir}/themes/deepin/
%{_datadir}/themes/deepin-dark/

%changelog
* Tue Apr 23 2021 uoser <uoser@uniontech.com> - 21.04.23-1
- update to 21.04.23-1
