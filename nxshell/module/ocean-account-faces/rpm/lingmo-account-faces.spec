Name:           ocean-account-faces
Version:        1.0.12
Release:        1
Summary:        Account faces for Linux Lingmo
License:        GPL-3.0-or-later
URL:            https://github.com/lingmoos/ocean-account-faces
Source0:        %{name}-%{version}.orig.tar.xz
BuildArch:      noarch

Requires:  accountsservice
Conflicts: lingmo-system-settings-module-account

%description
Account faces for Linux Lingmo

%prep
%setup -q -n %{name}-%{version}

%build

%install
%make_install

%files
%{_sharedstatedir}/AccountsService/icons/*

%changelog
* Tue Apr 21 2021 uoser <uoser@uniontech.com> - 1.0.12-1
- update to 1.0.12-1
