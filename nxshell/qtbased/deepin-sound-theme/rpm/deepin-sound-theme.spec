Name:           deepin-sound-theme
Version:        15.10.3.1
Release:        1
Summary:        Deepin sound theme
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-sound-theme
Source0:        %{name}-%{version}.orig.tar.xz	
BuildArch:      noarch

%description
Sound files for the Deeping Desktop Environment.

%prep
%setup -q

%build

%install
%make_install

%files
%doc README.md
%license LICENSE
%dir %{_datadir}/sounds/deepin/
%dir %{_datadir}/sounds/deepin/stereo/
%{_datadir}/sounds/deepin/index.theme
%{_datadir}/sounds/deepin/stereo/*.wav

%changelog
* Tue Apr 21 2021 uoser <uoser@uniontech.com> - 15.10.3.1-1
- update to 15.10.3.1-1
