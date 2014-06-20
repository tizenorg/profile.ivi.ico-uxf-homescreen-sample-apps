Name:       ico-uxf-homescreen-sample-apps
Summary:    HomeScreen sample application
Version:    0.9.8
Release:    0
Group:      System/GUI
License:    Apache License, Version 2.0
URL:        http://www.toyota.com
Source0:    %{name}-%{version}.tar.bz2

#ico-app-soundsample
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: libpulse-devel
BuildRequires: ico-uxf-utilities-devel
BuildRequires: pkgconfig(capi-appfw-application)
Requires: weston >= 1.2
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-homescreen >= 0.9.01
Requires: weston-ivi-shell-clients
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
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(aul)
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

#ico-app-miscellaneous
Requires: weekeyboard

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

# create tizen package metadata related directories
mkdir -p %{buildroot}%{_datadir}/packages/
mkdir -p %{buildroot}%{_datadir}/icons/default/small

# configurations(ico-app-soundsample)
%define sound_PREFIX /usr/apps/org.tizen.ico.app-soundsample

mkdir -p %{buildroot}%{sound_PREFIX}/bin/
mkdir -p %{buildroot}%{sound_PREFIX}/sounds/
mkdir -p %{buildroot}%{sound_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}%{sound_PREFIX}/res/images/
install -m 0644 ico-app-soundsample/soundsample_config.txt %{buildroot}%{sound_PREFIX}/res/
install -m 0644 ico-app-soundsample/sound_bg.png %{buildroot}%{sound_PREFIX}/res/images/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.png %{buildroot}%{sound_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-soundsample/musicbox.wav %{buildroot}%{sound_PREFIX}/sounds/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.xml %{buildroot}%{_datadir}/packages/

# configurations(ico-app-vicsample)
%define vic_PREFIX /usr/apps/org.tizen.ico.app-vicsample

mkdir -p %{buildroot}%{vic_PREFIX}/bin/
mkdir -p %{buildroot}%{vic_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}%{vic_PREFIX}/res/images/
install -m 0644 ico-app-vicsample/vicsample_config.txt %{buildroot}%{vic_PREFIX}/res/
install -m 0644 ico-app-vicsample/vicinfo_bg.png %{buildroot}%{vic_PREFIX}/res/images/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.png %{buildroot}%{vic_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.xml %{buildroot}%{_datadir}/packages/

# configurations(ico-app-miscellaneous)
# install tizen package metadata for weston-terminal
install -m 0644 ico-app-miscellaneous/terminal.xml %{buildroot}%{_datadir}/packages/

# install browser package metadata for MiniBrowser
mkdir -p %{buildroot}%{_bindir}
install -m 0644 ico-app-miscellaneous/browser.xml %{buildroot}%{_datadir}/packages/
install -m 0644 ico-app-miscellaneous/browser.png %{buildroot}%{_datadir}/icons/default/small/
install -m 0755 ico-app-miscellaneous/browser %{buildroot}%{_bindir}

# install tizen package metadata for weekeyboard
install -m 0644 ico-app-miscellaneous/weekeyboard.xml %{buildroot}%{_datadir}/packages/

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
# files(ico-app-soundsample)
%{sound_PREFIX}/bin/ico-app-soundsample
%{sound_PREFIX}/res/soundsample_config.txt
%{sound_PREFIX}/res/images/sound_bg.png
%{sound_PREFIX}/res/icons/default/small/org.tizen.ico.app-soundsample.png
%{sound_PREFIX}/sounds/musicbox.wav
%{_datadir}/packages/org.tizen.ico.app-soundsample.xml

# files(ico-app-vicsample)
%{vic_PREFIX}/bin/ico-app-vicsample
%{vic_PREFIX}/res/vicsample_config.txt
%{vic_PREFIX}/res/images/vicinfo_bg.png
%{vic_PREFIX}/res/icons/default/small/org.tizen.ico.app-vicsample.png
/usr/share/packages/org.tizen.ico.app-vicsample.xml

# files(ico-app-miscellaneous)
%{_bindir}/browser
%{_datadir}/packages/browser.xml
%{_datadir}/packages/terminal.xml
%{_datadir}/packages/weekeyboard.xml
%{_datadir}/icons/default/small/browser.png

%post
/sbin/ldconfig
# This icon exists in main weston package so we don't package it in.
# Create a symbolic link to it instead.
ln -s %{_datadir}/weston/terminal.png %{_datadir}/icons/default/small/
# Update the app database.
%{_bindir}/pkginfo --imd /usr/share/packages/org.tizen.ico.app-soundsample.xml
%{_bindir}/pkginfo --imd /usr/share/packages/org.tizen.ico.app-vicsample.xml
%{_bindir}/pkginfo --imd /usr/share/packages/browser.xml
%{_bindir}/pkginfo --imd /usr/share/packages/terminal.xml

%preun
# Update the app database.
%{_bindir}/pkginfo --rmd /usr/share/packages/org.tizen.ico.app-soundsample.xml
%{_bindir}/pkginfo --rmd /usr/share/packages/org.tizen.ico.app-vicsample.xml
%{_bindir}/pkginfo --rmd /usr/share/packages/browser.xml
%{_bindir}/pkginfo --rmd /usr/share/packages/terminal.xml

%postun
/sbin/ldconfig
rm -f /usr/share/applications/org.tizen.ico.app-soundsample.desktop
rm -f /usr/share/applications/org.tizen.ico.app-vicsample.desktop
rm -f /usr/share/applications/browser.desktop
rm -f /usr/share/applications/terminal.desktop
rm -f %{_datadir}/icons/default/small/terminal.png
