Name:           iotivity-example
Version:        1.1.0
Release:        0
License:        Apache-2.0
Summary:        Very minimalist example of IoTivity resource
Url:            http://git.s-osg.org/iotivity-example/plain/README.md
Group:          System/Libraries
#X-Vc-Url:      http://git.s-osg.org/iotivity-example
Group:          Contrib
Source:         %{name}-%{version}.tar.gz

BuildRequires:  make
BuildRequires:  fdupes
BuildRequires:  pkgconfig(iotivity)
BuildRequires:  boost-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  systemd
Requires:  iotivity


%description
Mimimal client/server application that share an IoTivity resource

%prep
%setup -q

%build

%__make %{?_smp_mflags} \
    PLATFORM=TIZEN \
    config_pkgconfig=1 \
    #eol

%install
%__make install \
    name=%{name} \
    DESTDIR=%{buildroot} \
    PLATFORM=TIZEN \
    config_pkgconfig=1 \
    #eol

make %{name}.service

install -d %{buildroot}%{_unitdir}

install extra/iotivity-example.service \
  %{buildroot}%{_unitdir}/%{name}.service

#install -d %{buildroot}%{_unitdir}/network.target.wants


#ln -s ../%{name}.service \
#  %{buildroot}%{_unitdir}/network.target.wants/%{name}.service
%install_service network.target.wants %{name}.service


%fdupes %{buildroot}

%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root)
/opt/%{name}/*
%{_unitdir}/%{name}.service
%{_unitdir}/network.target.wants/%{name}.service
