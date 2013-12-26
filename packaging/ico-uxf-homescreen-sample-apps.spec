Name:       ico-uxf-homescreen-sample-apps
Summary:    HomeScreen sample application 
Version:    0.9.6
Release:    2.1
Group:      System/GUI
License:    Apache License, Version 2.0
URL:        http://www.toyota.com
Source0:    %{name}-%{version}.tar.bz2

#ico-app-soundsample
BuildRequires: pkgconfig(wayland-client) >= 1.0
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(ail)
BuildRequires: libpulse-devel
BuildRequires: ico-uxf-weston-plugin-devel
BuildRequires: ico-uxf-homescreen-system-controller-devel >= 0.9.01
BuildRequires: ico-uxf-utilities-devel
BuildRequires: pkgconfig(capi-appfw-application)
Requires: weston >= 1.2
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-homescreen >= 0.9.01
Requires: ecore
Requires: elementary
Requires: evas
Requires: glib2
Requires: libpulse
Requires: capi-appfw-application
Requires: ico-uxf-utilities

#ico-app-vicsample
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(aul)
BuildRequires: ico-uxf-weston-plugin-devel
BuildRequires: ico-uxf-homescreen-system-controller-devel >= 0.9.01
BuildRequires: ico-uxf-utilities-devel >= 0.9.04
BuildRequires: pkgconfig(capi-appfw-application)
Requires: weston >= 1.2
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-homescreen >= 0.9.01
Requires: ecore
Requires: elementary
Requires: evas
Requires: dbus
Requires: dbus-glib
Requires: automotive-message-broker >= 0.10.804
Requires: capi-appfw-application
Requires: edbus
Requires: ico-uxf-utilities >= 0.9.04
Requires: ico-vic-amb-plugin >= 0.9.4

%description
HomeScreen sample application 

%prep
%setup -q -n %{name}-%{version}

%build
autoreconf --install

%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

# configurations(ico-app-soundsample)
%define sound_PREFIX /usr/apps/org.tizen.ico.app-soundsample

mkdir -p %{buildroot}%{sound_PREFIX}/bin/
mkdir -p %{buildroot}%{sound_PREFIX}/sounds/
mkdir -p %{buildroot}%{sound_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}%{sound_PREFIX}/res/images/
mkdir -p %{buildroot}/usr/share/packages/
install -m 0644 ico-app-soundsample/soundsample_config.txt %{buildroot}%{sound_PREFIX}/res/
install -m 0644 ico-app-soundsample/sound_bg.png %{buildroot}%{sound_PREFIX}/res/images/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.png %{buildroot}%{sound_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-soundsample/musicbox.wav %{buildroot}%{sound_PREFIX}/sounds/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.xml %{buildroot}/usr/share/packages/

# configurations(ico-app-vicsample)
%define vic_PREFIX /usr/apps/org.tizen.ico.app-vicsample

mkdir -p %{buildroot}%{vic_PREFIX}/bin/
mkdir -p %{buildroot}%{vic_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}%{vic_PREFIX}/res/images/
mkdir -p %{buildroot}/usr/share/packages/
install -m 0644 ico-app-vicsample/vicsample_config.txt %{buildroot}%{vic_PREFIX}/res/
install -m 0644 ico-app-vicsample/vicinfo_bg.png %{buildroot}%{vic_PREFIX}/res/images/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.png %{buildroot}%{vic_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.xml %{buildroot}/usr/share/packages/

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
# files(ico-app-soundsample)
%{sound_PREFIX}/bin/ico-app-soundsample
%{sound_PREFIX}/res/soundsample_config.txt
%{sound_PREFIX}/res/images/sound_bg.png
%{sound_PREFIX}/res/icons/default/small/org.tizen.ico.app-soundsample.png
%{sound_PREFIX}/sounds/musicbox.wav
/usr/share/packages/org.tizen.ico.app-soundsample.xml

# files(ico-app-vicsample)
%{vic_PREFIX}/bin/ico-app-vicsample
%{vic_PREFIX}/res/vicsample_config.txt
%{vic_PREFIX}/res/images/vicinfo_bg.png
%{vic_PREFIX}/res/icons/default/small/org.tizen.ico.app-vicsample.png
/usr/share/packages/org.tizen.ico.app-vicsample.xml

%post
/sbin/ldconfig
# init db
/usr/bin/pkg_initdb
/usr/bin/ail_initdb

%postun
/sbin/ldconfig
rm -f /usr/share/applications/org.tizen.ico.app-soundsample.desktop
rm -f /usr/share/applications/org.tizen.ico.app-vicsample.desktop
# init db
/usr/bin/pkg_initdb
/usr/bin/ail_initdb
