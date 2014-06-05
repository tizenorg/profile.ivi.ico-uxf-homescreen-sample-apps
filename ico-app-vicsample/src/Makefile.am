##ico-app-vicsample
app_vicsample_bindir = /usr/apps/org.tizen.ico.app-vicsample/bin
app_vicsample_bin_PROGRAMS = ico-app-vicsample
 
ico_app_vicsample_SOURCES = vicsample.c 
ico_app_vicsample_LDADD = -L/usr/lib -lico-util -lcapi-appfw-application $(AUL_LIBS) $(BUNDLE_LIBS) $(EFL_LIBS) -lwebsockets -lrt -lm -ldbus-1 -lpthread
ico_app_vicsample_CFLAGS = -Wall -I. -I../../common/include $(AUL_CFLAGS) $(BUNDLE_CFLAGS) $(EFL_CFLAGS) -I/usr/include/appfw -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/ico-util

