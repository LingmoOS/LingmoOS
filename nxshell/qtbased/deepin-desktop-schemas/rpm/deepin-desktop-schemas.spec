%define specrelease 1%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

Name:           deepin-desktop-schemas
Version:        5.9.7
Release:        %{specrelease}
Summary:        GSettings deepin desktop-wide schemas
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-desktop-schemas
Source0:        %{name}-%{version}.tar.xz

Provides:       deepin-default-gsettings
Obsoletes:      deepin-default-gsettings

BuildArch:     noarch

BuildRequires:  golang
BuildRequires:  glib2
BuildRequires:  gocode
BuildRequires:  deepin-desktop-base
BuildRequires:  go-lib-devel


Requires:  deepin-desktop-base

%description
deepin-desktop-schemas contains a collection of GSettings schemas for
 settings shared by various components of a desktop.

%prep
%autosetup

%build
export GOPATH=/usr/share/gocode

%if %{_arch} == "aarch64"
%define  buildarch arm
%else
%define  buildarch x86
%endif

%make_build ARCH=%{buildarch}
%install
%make_install PREFIX=%{_prefix} ARCH=%{buildarch}

%check
make test

%post
data_dir=/usr/share/deepin-desktop-schemas
gschemas_dir=/usr/share/glib-2.0/schemas
app_store_dir=/usr/share/deepin-app-store
app_store_ini_file=$app_store_dir/settings.ini
app_store_ini_file_pro=$app_store_dir/settings-pro.ini
app_store_ini_file_community=$app_store_dir/settings-community.ini
app_store_ini_file_personal=$app_store_dir/settings-personal.ini

SYSTYPE=$(grep Type= /etc/deepin-version|cut -d= -f 2)
product_override_file=$gschemas_dir/91_deepin_product.gschema.override
if [ -e $app_store_ini_file -a -L $app_store_ini_file ]; then
    # delete it if it is symbol link
    rm $app_store_ini_file
fi
case "$SYSTYPE" in
    Professional)
        ln -sf $data_dir/pro-override $product_override_file
        if [ ! -f $app_store_ini_file ]; then
            ln -sf $app_store_ini_file_pro $app_store_ini_file
        fi
    ;;
    Server)
        ln -sf $data_dir/server-override $product_override_file
        if [ ! -f $app_store_ini_file ]; then
            ln -sf $app_store_ini_file_pro $app_store_ini_file
        fi
    ;;
    Desktop)
        ln -sf $data_dir/desktop-override $product_override_file
        if [ ! -f $app_store_ini_file ]; then
            ln -sf $app_store_ini_file_community $app_store_ini_file
        fi
    ;;
   Personal)
        ln -sf $data_dir/personal-override $product_override_file
        if [ ! -f $app_store_ini_file ]; then
            ln -sf $app_store_ini_file_personal $app_store_ini_file
        fi
    ;;
esac

%files
%doc README.md
%license LICENSE
%{_datadir}/glib-2.0/schemas/*
%{_datadir}/deepin-app-store/*
%{_datadir}/deepin-desktop-schemas/*
%{_datadir}/deepin-appstore/*

%changelog
* Thu Mar 11 2021 uniontech <uoser@uniontech.com> - 5.9.7-1
- Update to 5.9.7