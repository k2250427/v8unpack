Name:           v8unpack
Version:        3.0
Release:        1%{?dist}
Summary:        Enterprise 8 unpack tool 

License:        No License
URL:            https://github.com/dmpas/v8unpack 
Source0:        v8unpack-3.0.tar.gz

BuildRequires:  zlib-devel, boost-devel

%description
Enterprise 8 unpack tool

%prep
%setup -q


%build
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%make_install


%files
%defattr(-,root,root)
/usr/bin/v8unpack
%doc



%changelog
* Tue Jul 21 2015 Shenja Sosna
- add rpm build 

* Sun Jun 28 2015 Sergey Batanov <sergey.batanov@dmpas.ru> 
- v8unpack (3.0.36-1) trusty; urgency=low
- Corrected installation binary path 

* Sun Jun 28 2015 Sergey Batanov <sergey.batanov@dmpas.ru> 
- Updated version number

* Sun Jun 28 2015 Sergey Batanov <sergey.batanov@dmpas.ru> 
- v8unpack (3.0.33-4) trusty; urgency=low
- Corrected build dependencies: zlib added 
- Sergey Batanov <sergey.batanov@dmpas.ru>  Sun, 28 Jun 2015 14:30:52 +0300

