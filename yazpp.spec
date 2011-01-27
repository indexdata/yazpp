Summary: YAZ++ package (main)
Name: yazpp
Version: 1.2.3
Release: 2indexdata
License: BSD
Group: Applications/Internet
Vendor: Index Data ApS <info@indexdata.dk>
Source: yazpp-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: %{_prefix}
BuildRequires: pkgconfig, libyaz4-devel >= 4.1.0
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.com/yazplusplus

%description
YAZ++ package.

%package -n libyazpp4
Summary: YAZ++ and ZOOM library
Group: Libraries
Requires: libyaz4 >= 4.1.0

%description -n libyazpp4
Libraries for the YAZ++ package.

%package -n libyazpp4-devel
Summary: Z39.50 Library - development package
Group: Development/Libraries
Requires: libyazpp4 = %{version}, libyaz4-devel

%description -n libyazpp4-devel
Development libraries and include files for the YAZ++ package.

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{_mandir} \
	--enable-shared --with-yaz=/usr/bin
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr ${RPM_BUILD_ROOT}
make prefix=${RPM_BUILD_ROOT}/%{_prefix} mandir=${RPM_BUILD_ROOT}/%{_mandir} \
	libdir=${RPM_BUILD_ROOT}/%{_libdir} install
rm ${RPM_BUILD_ROOT}/%{_libdir}/*.la

%clean
rm -fr ${RPM_BUILD_ROOT}

%post -n libyazpp4 -p /sbin/ldconfig 
%postun -n libyazpp4 -p /sbin/ldconfig 

%files -n libyazpp4
%doc README LICENSE NEWS
%defattr(-,root,root)
%{_libdir}/*.so.*

%files -n libyazpp4-devel
%defattr(-,root,root)
%{_bindir}/yazpp-config
%{_includedir}/yazpp
%{_libdir}/*.so
%{_libdir}/*.a
%{_datadir}/aclocal/yazpp.m4
%{_mandir}/man?/yazpp-config.*
%{_datadir}/doc/yazpp

