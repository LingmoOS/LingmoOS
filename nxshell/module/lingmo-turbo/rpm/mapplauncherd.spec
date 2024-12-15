Name:       mapplauncherd
Summary:    Application launcher for fast startup
Version:    4.1.24
Release:    1
Group:      System/Daemons
License:    LGPLv2+
URL:        https://git.merproject.org/mer-core/mapplauncherd
Source0:    %{name}-%{version}.tar.bz2
Source1:    booster-cgroup-mount.service
Requires:   systemd-user-session-targets
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(pre):  shadow-utils
BuildRequires:  pkgconfig(libshadowutils)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  cmake
BuildRequires:  python
Provides:   meegotouch-applauncherd > 3.0.3
Obsoletes:   meegotouch-applauncherd <= 3.0.3

%description
Application invoker and launcher daemon that speed up
application startup time and share memory. Provides also
functionality to launch applications as single instances.


%package devel
Summary:    Development files for launchable applications
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}
Provides:   meegotouch-applauncherd-devel > 3.0.3
Obsoletes:  meegotouch-applauncherd-devel <= 3.0.3

%description devel
Development files for creating applications that can be launched
using mapplauncherd.

%package cgroup
Summary:    Service files for booster cgroup mount
Group:      System/Daemons
Requires:   %{name} = %{version}-%{release}

%description cgroup
Scripts and services files for application launcher to mount
booster cgroup at startup.

%prep
%setup -q -n %{name}-%{version}

%build
export BUILD_TESTS=1
export MEEGO=1
unset LD_AS_NEEDED

%configure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

# Don't use %exclude, remove at install phase
rm -f %{buildroot}/usr/share/fala_images/fala_qml_helloworld

mkdir -p %{buildroot}/usr/lib/systemd/user/user-session.target.wants
ln -s ../booster-generic.service %{buildroot}/usr/lib/systemd/user/user-session.target.wants/

mkdir -p %{buildroot}%{_datadir}/mapplauncherd/privileges.d

install -D -m 0755 %{SOURCE1} %{buildroot}/lib/systemd/system/booster-cgroup-mount.service
mkdir -p %{buildroot}/lib/systemd/system/multi-user.target.wants
ln -s ../booster-cgroup-mount.service %{buildroot}/lib/systemd/system/multi-user.target.wants/

install -D -m 0755 scripts/booster-cgroup-mount %{buildroot}/usr/lib/startup/booster-cgroup-mount

%pre
groupadd -rf privileged

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_datadir}/mapplauncherd
%dir %{_datadir}/mapplauncherd/privileges.d
%{_bindir}/invoker
%{_bindir}/single-instance
%{_libdir}/libapplauncherd.so*
%attr(2755, root, privileged) %{_libexecdir}/mapplauncherd/booster-generic
%{_libdir}/systemd/user/booster-generic.service
%{_libdir}/systemd/user/user-session.target.wants/booster-generic.service

%files devel
%defattr(-,root,root,-)
%{_includedir}/applauncherd/*

%files cgroup
/lib/systemd/system/booster-cgroup-mount.service
/lib/systemd/system/multi-user.target.wants/booster-cgroup-mount.service
%dir %{_libdir}/startup
%{_libdir}/startup/booster-cgroup-mount
