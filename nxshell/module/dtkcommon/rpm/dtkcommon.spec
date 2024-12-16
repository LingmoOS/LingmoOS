Name:    dtkcommon
Version: 5.5.17
Release: 1
Summary: dtk common files
Source0: %{name}-%{version}.orig.tar.xz

License: GPLv3

BuildRequires:  qt5-qtbase-devel

Obsoletes:      dtkcore <= 5.4.10
Obsoletes:      dtkcore-devel <= 5.4.10

%global debug_package %{nil}

%description
dtk common files

%package devel
Summary:        Development package for %{sname}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
dtk common files - devel

%prep
%setup -q

%build
export QTDIR="%{_qt5_prefix}"
export PATH="%{_qt5_bindir}:$PATH"
%qmake_qt5
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}


%files
%doc README.md
%license LICENSE
%{_datarootdir}/glib-2.0/schemas/*

%files devel
%doc README.md
%license LICENSE
%{_prefix}/lib/cmake/Dtk/*
%{_libdir}/*/mkspecs/features/*
%{_libdir}/*/mkspecs/modules/*

%changelog
