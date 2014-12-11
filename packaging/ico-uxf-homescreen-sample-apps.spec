# configurations(ico-app-soundsample)
%define sound_PREFIX %TZ_SYS_RW_APP/org.tizen.ico.app-soundsample

# configurations(ico-app-vicsample)
%define vic_PREFIX %TZ_SYS_RW_APP/org.tizen.ico.app-vicsample

Name:       ico-uxf-homescreen-sample-apps
Summary:    HomeScreen sample application
Version:    0.9.8
Release:    0
Group:      Automotive/ICO Homescreen
License:    Apache-2.0
URL:        http://www.toyota.com
Source0:    %{name}-%{version}.tar.bz2
Source1001: %{name}.manifest

BuildRequires:  pkgconfig(libtzplatform-config)

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
Requires: automotive-message-broker >= 0.10.804
Requires: capi-appfw-application
Requires: edbus
Requires: ico-uxf-utilities >= 0.9.04
Requires: ico-vic-amb-plugin >= 0.9.4

#ico-app-miscellaneous
Requires: genivi-shell
Requires: weekeyboard

%description
HomeScreen sample application files

%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .

%build
%reconfigure
%__make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

# create tizen package metadata related directories
mkdir -p %{buildroot}%{_datadir}/packages/
mkdir -p %{buildroot}%{_datadir}/icons/default/small

mkdir -p %{buildroot}%{sound_PREFIX}/bin/
mkdir -p %{buildroot}%{sound_PREFIX}/sounds/
mkdir -p %{buildroot}%{sound_PREFIX}/res/images/
install -m 0644 ico-app-soundsample/soundsample_config.txt %{buildroot}%{sound_PREFIX}/res/
install -m 0644 ico-app-soundsample/sound_bg.png %{buildroot}%{sound_PREFIX}/res/images/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.png %{buildroot}%{_datadir}/icons/default/small/
install -m 0644 ico-app-soundsample/musicbox.wav %{buildroot}%{sound_PREFIX}/sounds/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.xml %{buildroot}%{_datadir}/packages/

mkdir -p %{buildroot}%{vic_PREFIX}/bin/
mkdir -p %{buildroot}%{vic_PREFIX}/res/images/
install -m 0644 ico-app-vicsample/vicsample_config.txt %{buildroot}%{vic_PREFIX}/res/
install -m 0644 ico-app-vicsample/vicinfo_bg.png %{buildroot}%{vic_PREFIX}/res/images/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.png %{buildroot}%{_datadir}/icons/default/small/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.xml %{buildroot}%{_datadir}/packages/

# configurations(ico-app-miscellaneous)
# install tizen package metadata for weston-terminal
install -m 0644 ico-app-miscellaneous/terminal.xml %{buildroot}%{_datadir}/packages/

# install tizen package metadata for weekeyboard
install -m 0644 ico-app-miscellaneous/weekeyboard.xml %{buildroot}%{_datadir}/packages/


%post
/sbin/ldconfig
# This icons exists in main weston package so we don't package it in.
# Create a symbolic link to it instead.
ln -sf %{_datadir}/weston/terminal.png %{_datadir}/icons/default/small/
# Update the app database.
pkg_initdb
ail_initdb

%postun
if [ "$1" = "0" ]; then
/sbin/ldconfig
rm -f %{_datadir}/applications/org.tizen.ico.app-soundsample.desktop
rm -f %{_datadir}/applications/org.tizen.ico.app-vicsample.desktop
rm -f %{_datadir}/applications/terminal.desktop
rm -f %{_datadir}/icons/default/small/terminal.png
# Update the app database.
pkg_initdb
ail_initdb
fi

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
# files(ico-app-soundsample)
%{sound_PREFIX}/bin/ico-app-soundsample
%{sound_PREFIX}/res/soundsample_config.txt
%{sound_PREFIX}/res/images/sound_bg.png
%{sound_PREFIX}/sounds/musicbox.wav
%{_datadir}/icons/default/small/org.tizen.ico.app-soundsample.png
%{_datadir}/packages/org.tizen.ico.app-soundsample.xml

# files(ico-app-vicsample)
%{vic_PREFIX}/bin/ico-app-vicsample
%{vic_PREFIX}/res/vicsample_config.txt
%{vic_PREFIX}/res/images/vicinfo_bg.png
%{_datadir}/icons/default/small/org.tizen.ico.app-vicsample.png
%{_datadir}/packages/org.tizen.ico.app-vicsample.xml

# files(ico-app-miscellaneous)
%{_datadir}/packages/terminal.xml
%{_datadir}/packages/weekeyboard.xml
