Name:           lingmo-sound-theme
Version:        15.10.3.1
Release:        1
Summary:        Lingmo sound theme
License:        GPLv3
URL:            https://github.com/lingmoos/lingmo-sound-theme
Source0:        %{name}-%{version}.orig.tar.xz	
BuildArch:      noarch

%description
Sound files for the Lingmog Desktop Environment.

%prep
%setup -q

%build

%install
%make_install

%files
%doc README.md
%license LICENSE
%dir %{_datadir}/sounds/lingmo/
%dir %{_datadir}/sounds/lingmo/stereo/
%{_datadir}/sounds/lingmo/index.theme
%{_datadir}/sounds/lingmo/stereo/*.wav

%changelog
* Tue Apr 21 2021 uoser <uoser@uniontech.com> - 15.10.3.1-1
- update to 15.10.3.1-1
