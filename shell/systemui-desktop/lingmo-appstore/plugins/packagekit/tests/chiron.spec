Summary:	Single line synopsis
Name:		chiron
Version:	1.1
Release:	1%{?dist}
URL:		http://127.0.0.1/
License:	GPLv2+

%description
This is the first paragraph in the example package spec file.

This is the second paragraph.

%install
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
touch $RPM_BUILD_ROOT/%{_bindir}/chiron

%files
%{_bindir}/chiron

%changelog
* Tue Apr 26 2016 Richard Hughes <richard@hughsie.com> - 1.1-1
- Initial version
