Name:           lingmo-gtk-theme
Version:        21.04.23
Release:        1
Summary:        Lingmo GTK Theme
License:        GPLv3
URL:            https://github.com/lingmoos/lingmo-gtk-theme
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
%{_datadir}/themes/lingmo/
%{_datadir}/themes/lingmo-dark/

%changelog
* Tue Apr 23 2021 uoser <uoser@uniontech.com> - 21.04.23-1
- update to 21.04.23-1
