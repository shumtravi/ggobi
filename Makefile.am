SUBDIRS = src plugins po doc tests

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = bootstrap

datafilesdir = $(datadir)/doc/ggobi/examples
dist_datafiles_DATA = \
	data/places.xml \
  data/sleep.csv \
  data/tmin.xml \
  data/Shipman.csv \
  data/laser.csv \
  data/prim7.xml \
  data/sleep.xml \
  data/tmin1.csv \
  data/Shipman.xml \
  data/laser.xml \
  data/ratcns.csv \
  data/snetwork.xml \
  data/tmin1.xml \
  data/adhoc.xml \
  data/morsecodes.xml \
  data/ratcns.xml \
  data/stdColorMap.xml \
  data/tmin2.xml \
  data/algal-bloom.xml \
  data/olive.csv \
  data/river.xml \
  data/tao-nomiss.xml \
  data/tmin3.xml \
  data/buckyball.xml \
  data/olive.xml \
  data/roos.csv \
  data/tao.csv \
  data/tmin4.xml \
  data/cube6.xml \
  data/perm4.xml \
  data/roos.xml \
  data/tao.xml \
  data/eies.xml \
  data/perm5.xml \
  data/sarsHK.xml \
  data/tips.csv \
  data/flea.csv \
  data/pigs.xml \
  data/sat.csv \
  data/tips.xml \
  data/flea.xml \
  data/places.csv \
  data/sat.xml \
  data/tmin.csv

#docfilesdir = $(datadir)/doc/ggobi
#dist_docfiles_DATA = doc/manual/manual.pdf

desktopdir = $(datadir)/applications
dist_desktop_DATA = xdg/ggobi.desktop

iconsdir = $(datadir)/pixmaps
dist_icons_DATA = xdg/ggobi.png

sharedir = $(pkgdatadir)
dist_share_DATA = share/colorschemes.xml

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ggobi-system.pc

ggobiconfdir = $(sysconfdir)/xdg/ggobi
ggobiconf_DATA = ggobirc

install-data-hook:
	cd $(DESTDIR)$(pkgconfigdir) && test -f ggobi-system.pc && \
  mv ggobi-system.pc ggobi.pc

uninstall-local:
	rm -f $(DESTDIR)$(pkgconfigdir)/ggobi.pc

DISTCHECK_CONFIGURE_FLAGS = --with-all-plugins --enable-gtk-doc

# We add symlinks for developer convenience
all-local: ggobi ggobirc

if OS_WIN32
ggobi:
else
ggobi:
	ln -s src/ggobi ggobi
endif

# Use this rule to build your ggobirc (for loading plugins)
ggobirc:
	echo "<ggobirc>" > $@
	echo "<plugins>" >> $@

	for i in $(PLUGIN_DIRS) ; do \
	  echo "" >> $@ ; \
	  cat plugins/$$i/plugin.xml >> $@ ; \
	done
	echo "</plugins>" >> $@
	echo "</ggobirc>" >> $@

win32-installer: ggobirc
	makensis //DVERSION="@VERSION@" win32/ggobi.nsi


doc/manual/manual.pdf:
	$(MAKE) $(AM_MAKEFLAGS) -C doc/manual manual.pdf


