Name:       ico-uxf-HomeScreen-sample-app
Summary:    HomeScreen sample application 
Version:    0.0.11
Release:    1
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
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(ail)
BuildRequires: libwebsockets-devel
BuildRequires: pulseaudio-libs-devel
BuildRequires: ico-uxf-weston-plugin-devel
BuildRequires: ico-uxf-HomeScreen-devel >= 0.3.06
Requires: weston >= 1.0
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-HomeScreen >= 0.3.06
Requires: ecore
Requires: elementary
Requires: evas
Requires: glib2
Requires: pulseaudio-libs

#ico-app-vicsample
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(aul)
BuildRequires: libwebsockets-devel
BuildRequires: ico-uxf-weston-plugin-devel
BuildRequires: ico-uxf-HomeScreen-devel >= 0.3.06
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-HomeScreen >= 0.3.06
Requires: ecore
Requires: elementary
Requires: evas
Requires: dbus
Requires: dbus-glib
Requires: automotive-message-broker

#ico-app-samplenavi
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(ecore-evas)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ewebkit2)
BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(wayland-cursor)
BuildRequires: pkgconfig(wayland-egl)
BuildRequires: pkgconfig(wayland-server)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(glesv2)
BuildRequires: pkgconfig(egl)
BuildRequires: pkgconfig(cairo)
BuildRequires: pkgconfig(pango)
BuildRequires: pkgconfig(pangocairo)
BuildRequires: pkgconfig(gdk-pixbuf-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(gthread-2.0)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(aul)
BuildRequires: libwebsockets-devel
BuildRequires: ico-uxf-weston-plugin-devel
BuildRequires: ico-uxf-HomeScreen-devel >= 0.3.06
BuildRequires: edje-tools
#BuildRequires: pkgconfig(opencv)
Requires: ico-uxf-weston-plugin
Requires: ico-uxf-HomeScreen >= 0.3.06
Requires: ecore
Requires: evas
Requires: ecore-evas
Requires: eina
Requires: elementary
Requires: webkit2-efl
Requires: wayland
Requires: mesa-libwayland-egl
Requires: ecore-wayland
Requires: mesa-libGLESv2
Requires: mesa-libEGL
Requires: cairo
Requires: pango
Requires: gdk-pixbuf
Requires: glib2
Requires: json-glib
Requires: aul
Requires: dbus
Requires: dbus-glib
Requires: automotive-message-broker
#Requires: libopencv2_4

#DemoMeterApp.wgt DemoAudioApp.wgt
Requires: pkgmgr

%description
HomeScreen sample application 

%prep
%setup -q -n %{name}-%{version}

%build
autoreconf --install

%configure
make %{?_smp_mflags}
cd ico-app-samplenavi/data/;sh make.sh;cd ../../

%install
rm -rf %{buildroot}
%make_install

# configurations(ico-app-soundsample)
%define sound_PREFIX /opt/apps/org.tizen.ico.app-soundsample

mkdir -p %{buildroot}%{sound_PREFIX}/bin/
mkdir -p %{buildroot}%{sound_PREFIX}/sounds/
mkdir -p %{buildroot}%{sound_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}/opt/share/applications/
install -m 0644 ico-app-soundsample/soundsample_config.txt %{buildroot}%{sound_PREFIX}/res/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.png %{buildroot}%{sound_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-soundsample/musicbox.wav %{buildroot}%{sound_PREFIX}/sounds/
install -m 0644 ico-app-soundsample/org.tizen.ico.app-soundsample.desktop %{buildroot}/opt/share/applications/

# configurations(ico-app-vicsample)
%define vic_PREFIX /opt/apps/org.tizen.ico.app-vicsample

mkdir -p %{buildroot}%{vic_PREFIX}/bin/
mkdir -p %{buildroot}%{vic_PREFIX}/res/icons/default/small/
mkdir -p %{buildroot}/opt/share/applications/
install -m 0644 ico-app-vicsample/vicsample_config.txt %{buildroot}%{vic_PREFIX}/res/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.png %{buildroot}%{vic_PREFIX}/res/icons/default/small/
install -m 0644 ico-app-vicsample/org.tizen.ico.app-vicsample.desktop %{buildroot}/opt/share/applications/

# configurations(ico-app-samplenavi)
%define navi_PREFIX /opt/apps/org.tizen.ico.app-samplenavi

mkdir -p %{buildroot}%{navi_PREFIX}/bin/
mkdir -p %{buildroot}%{navi_PREFIX}/data/
mkdir -p %{buildroot}/opt/share/applications/
install -m 0644 ico-app-samplenavi/data/*.edj %{buildroot}%{navi_PREFIX}/data/
install -m 0644 ico-app-samplenavi/org.tizen.ico.app-samplenavi.desktop %{buildroot}/opt/share/applications/
cp -r ico-app-samplenavi/res %{buildroot}%{navi_PREFIX}/.

# configurations(DemoMeterApp.wgt DemoAudioApp.wgt)
mkdir -p %{buildroot}/tmp/
install -m 0644 wgt/*.wgt %{buildroot}/tmp/

%files
%defattr(-,root,root,-)
# files(ico-app-soundsample)
%{sound_PREFIX}/bin/ico-app-soundsample
%{sound_PREFIX}/res/soundsample_config.txt
%{sound_PREFIX}/res/icons/default/small/org.tizen.ico.app-soundsample.png
%{sound_PREFIX}/sounds/musicbox.wav
/opt/share/applications/org.tizen.ico.app-soundsample.desktop

# files(ico-app-vicsample)
%{vic_PREFIX}/bin/ico-app-vicsample
%{vic_PREFIX}/res/vicsample_config.txt
%{vic_PREFIX}/res/icons/default/small/org.tizen.ico.app-vicsample.png
/opt/share/applications/org.tizen.ico.app-vicsample.desktop

# files(ico-app-samplenavi)
%{navi_PREFIX}/bin/ico-app-samplenavi
%{navi_PREFIX}/data/*.edj
%{navi_PREFIX}/res
/opt/share/applications/org.tizen.ico.app-samplenavi.desktop

# files(DemoMeterApp.wgt DemoAudioApp.wgt)
/tmp/*.wgt

%post
# install cmd(DemoMeterApp.wgt DemoAudioApp.wgt)
pkgcmd -i -t wgt -p /tmp/DemoMeterApp.wgt -q
pkgcmd -i -t wgt -p /tmp/DemoAudioApp.wgt -q

# init db
rm -rf /opt/dbspace/.app_info*
/usr/bin/ail_initdb

rm -f /tmp/DemoMeterApp.wgt
rm -f /tmp/DemoAudioApp.wgt

%postun
# uninstall cmd(DemoMeterApp.wgt DemoAudioApp.wgt)
pkgcmd -u -t wgt -n AKsMREAjt9 -q
pkgcmd -u -t wgt -n d41dRLC2Qs -q

# init db
rm -rf /opt/dbspace/.app_info*
/usr/bin/ail_initdb
