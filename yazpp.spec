%define idmetaversion %(. ./IDMETA; echo $VERSION)
Summary: YAZ++ package (main)
Name: yazpp
Version: %{idmetaversion}
Release: 1indexdata
License: BSD
Group: Applications/Internet
Vendor: Index Data ApS <info@indexdata.dk>
Source: yazpp-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: %{_prefix}
BuildRequires: pkgconfig, libyaz5-devel >= 5.29.0
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.com/yazplusplus

%description
YAZ++ package.

%package -n libyazpp7
Summary: YAZ++ and ZOOM library
Group: Libraries
Requires: libyaz5 >= 5.1.0

%description -n libyazpp7
Libraries for the YAZ++ package.

%package -n libyazpp7-devel
Summary: Z39.50 Library - development package
Group: Development/Libraries
Requires: libyazpp7 = %{version}, libyaz5-devel
Conflicts: libyazpp4-devel
Conflicts: libyazpp5-devel
Conflicts: libyazpp6-devel

%description -n libyazpp7-devel
Development libraries and include files for the YAZ++ package.

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{_mandir} \
	--enable-shared --with-yaz=pkg
%if %{?make_build:1}%{!?make_build:0}
%make_build
%else
make -j4 CFLAGS="$RPM_OPT_FLAGS"
%endif

%install
rm -fr ${RPM_BUILD_ROOT}
make install DESTDIR=${RPM_BUILD_ROOT}
rm ${RPM_BUILD_ROOT}/%{_libdir}/*.la

%clean
rm -fr ${RPM_BUILD_ROOT}

%post -n libyazpp7 -p /sbin/ldconfig
%postun -n libyazpp7 -p /sbin/ldconfig

%files -n libyazpp7
%doc README LICENSE NEWS
%defattr(-,root,root)
%{_libdir}/*.so.*

%files -n libyazpp7-devel
%defattr(-,root,root)
%{_bindir}/yazpp-config
%{_includedir}/yazpp
%{_libdir}/*.so
%{_libdir}/*.a
%{_datadir}/aclocal/yazpp.m4
%{_mandir}/man?/yazpp-config.*
%{_datadir}/doc/yazpp

