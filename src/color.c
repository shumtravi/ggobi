/*-- color.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <stdlib.h>

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "colorscheme.h"

void
colorscheme_init (colorschemed * scheme)
{
  gint i;
  gboolean writeable = false, best_match = true, *success;

  if (!scheme || scheme->n <= 0) {
    g_printerr ("unable to init colorscheme: ncolors=%d\n", scheme->n);
    return;
  }

  success = (gboolean *) g_malloc (scheme->n * sizeof (gboolean));

  scheme->rgb = (GdkColor *) g_realloc (scheme->rgb,
                                        scheme->n * sizeof (GdkColor));

  /* this may have already been done; is there harm in doing it again? */
  for (i = 0; i < scheme->n; i++) {
    scheme->rgb[i].red = (guint16) (scheme->data[i][0] * 65535.0);
    scheme->rgb[i].green = (guint16) (scheme->data[i][1] * 65535.0);
    scheme->rgb[i].blue = (guint16) (scheme->data[i][2] * 65535.0);
  }

  gdk_colormap_alloc_colors (gdk_colormap_get_system (),
                             scheme->rgb, scheme->n, writeable, best_match,
                             success);

  /*
   * Success[i] should always be true, since I'm allowing best_match,
   * but this can't hurt.
   */
  for (i = 0; i < scheme->n; i++) {
    if (!success[i]) {
      scheme->rgb[i].red = (guint16) 65535;
      scheme->rgb[i].green = (guint16) 65535;
      scheme->rgb[i].blue = (guint16) 65535;
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                    &scheme->rgb[i], writeable,
                                    best_match) == false) {
        g_printerr ("Unable to allocate colors, not even white!\n");
        exit (0);
      }
    }
  }

/*
 * background color
*/
  scheme->rgb_bg.red = (guint16) (scheme->bg[0] * 65535.0);
  scheme->rgb_bg.green = (guint16) (scheme->bg[1] * 65535.0);
  scheme->rgb_bg.blue = (guint16) (scheme->bg[2] * 65535.0);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                 &scheme->rgb_bg, writeable, best_match))
    g_printerr ("failure allocating background color\n");

/*
 * color for showing hidden points and edges to preserve context
 * in a few situations:  when doing "un-hide" brushing and when showing
 * neighbors in the GraphAction plugin
*/
  {
    gdouble red, green, blue;
    if (scheme->bg[0] + scheme->bg[1] + scheme->bg[2] > 1.5) {
      red = MAX (0.0, scheme->bg[0] - .3);
      green = MAX (0.0, scheme->bg[1] - .3);
      blue = MAX (0.0, scheme->bg[2] - .3);
    }
    else {
      red = MIN (1.0, scheme->bg[0] + .3);
      green = MIN (1.0, scheme->bg[1] + .3);
      blue = MIN (1.0, scheme->bg[2] + .3);
    }
    scheme->rgb_hidden.red = (guint16) (red * 65535.0);
    scheme->rgb_hidden.green = (guint16) (green * 65535.0);
    scheme->rgb_hidden.blue = (guint16) (blue * 65535.0);
    if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                   &scheme->rgb_hidden, writeable,
                                   best_match))
      g_printerr ("failure allocating hidden color\n");
  }

/*
 * accent color:  that is, the color that's used for
 * axes and labels, and not for brushing.
*/
  scheme->rgb_accent.red = (guint16) (scheme->accent[0] * 65535.0);
  scheme->rgb_accent.green = (guint16) (scheme->accent[1] * 65535.0);
  scheme->rgb_accent.blue = (guint16) (scheme->accent[2] * 65535.0);
  if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                 &scheme->rgb_accent, writeable, best_match))
    g_printerr ("failure allocating background color\n");

  g_free (success);
}

/*-- If gg->colorSchemes == NULL, then provide one and make it active --*/
colorschemed *
default_scheme_init ()
{
  gint i, k;


  static gdouble data[][3] = {
    {1.0000, 1.0000, 0.2000},
    {0.8941, 0.1020, 0.1098},
    {0.2157, 0.4941, 0.7216},
    {0.3020, 0.6863, 0.2902},
    {1.0000, 0.4980, 0.0000},
    {0.6510, 0.3373, 0.1569},
    {0.9686, 0.5059, 0.7490},
    {0.4980, 0.4980, 0.4980},
    {0.5961, 0.3059, 0.6392}
  };
  static gchar *colorNames[] = {
    "Yellow", "Orange", "Blue", "Green", "Orange", "Brown",
    "Pink", "Gray", "Purple"
  };
  static gdouble bg[] = { 0.000, 0.000, 0.000 };
  static gdouble accent[] = { 1.000, 1.000, 1.000 };
  colorschemed *scheme = (colorschemed *) g_malloc (sizeof (colorschemed));

  scheme->name = g_strdup ("Set1 9");
  scheme->description =
    g_strdup
    ("From Cindy Brewer, one of the schemes in the ColorBrewer software");
  scheme->type = qualitative;
  scheme->system = rgb;
  scheme->n = sizeof (data) / sizeof (data[0]);
  scheme->rgb = NULL;
  scheme->criticalvalue = 0;  /*-- unused --*/

  /*-- fill in the color names --*/
  scheme->colorNames = g_array_new (false, false, sizeof (gchar *));
  for (i = 0; i < scheme->n; i++)
    g_array_append_val (scheme->colorNames, colorNames[i]);

  /*-- allocate and populate data, bg, and accent --*/
  scheme->data = (gdouble **) g_malloc (scheme->n * sizeof (gdouble *));
  for (k = 0; k < scheme->n; k++) {
    scheme->data[k] = (gdouble *) g_malloc (3 * sizeof (gdouble));
    for (i = 0; i < 3; i++)
      scheme->data[k][i] = data[k][i];
  }

  scheme->bg = (gdouble *) g_malloc (3 * sizeof (gdouble));
  for (i = 0; i < 3; i++)
    scheme->bg[i] = bg[i];

  scheme->accent = (gdouble *) g_malloc (3 * sizeof (gdouble));
  for (i = 0; i < 3; i++)
    scheme->accent[i] = accent[i];

  colorscheme_init (scheme);

  return scheme;
}


/* API */
/*
static guint m[MAXNCOLORS][3];
guint **
getColorTable (GGobiSession * gg)
{
  gint k;
  colorschemed *scheme = gg->activeColorScheme;

  for (k = 0; k < MAXNCOLORS; k++) {
    m[k][0] = scheme->rgb[k].red;
    m[k][1] = scheme->rgb[k].green;
    m[k][2] = scheme->rgb[k].blue;
  }

  return (guint **) m;
}*/


/*-- initialize the tour manip colors and the shades of gray --*/
void
special_colors_init (GGobiSession * gg)
{
  GdkColormap *cmap = gdk_colormap_get_system ();
  gboolean writeable = false, best_match = true;

/*
 * colors that show up in the variable circle panel
*/
  gg->vcirc_manip_color.red = (guint16) 65535;
  gg->vcirc_manip_color.green = (guint16) 0;
  gg->vcirc_manip_color.blue = (guint16) 65535;
  if (!gdk_colormap_alloc_color (cmap, &gg->vcirc_manip_color, writeable,
                                 best_match))
    g_printerr ("trouble allocating vcirc_manip_color\n");

  gg->vcirc_freeze_color.red = (guint16) 0;
  gg->vcirc_freeze_color.green = (guint16) 64435;
  gg->vcirc_freeze_color.blue = (guint16) 0;
  if (!gdk_colormap_alloc_color (cmap, &gg->vcirc_freeze_color, writeable,
                                 best_match))
    g_printerr ("trouble allocating vcirc_freeze_color\n");

  gg->darkgray.red = gg->darkgray.blue = gg->darkgray.green =
    (guint16) (.3 * 65535.0);
  if (!gdk_colormap_alloc_color (cmap, &gg->darkgray, writeable, best_match))
    g_printerr ("trouble allocating dark gray\n");
  gg->mediumgray.red = gg->mediumgray.blue = gg->mediumgray.green =
    (guint16) (.5 * 65535.0);
  if (!gdk_colormap_alloc_color
      (cmap, &gg->mediumgray, writeable, best_match))
    g_printerr ("trouble allocating medium gray\n");
  gg->lightgray.red = gg->lightgray.blue = gg->lightgray.green =
    (guint16) (.7 * 65535.0);
  if (!gdk_colormap_alloc_color (cmap, &gg->lightgray, writeable, best_match))
    g_printerr ("trouble allocating light gray\n");
}

void
init_plot_GC (GdkWindow * w, GGobiSession * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  gg->plot_GC = gdk_gc_new (w);
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_gc_set_background (gg->plot_GC, &scheme->rgb_bg);
  /* line_width, GdkLineStyle, GdkCapStyle, GdkJoinStyle */
  gdk_gc_set_line_attributes (gg->plot_GC,
                              0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
}

void
init_var_GCs (GtkWidget * w, GGobiSession * gg)
{
  GdkWindow *window = w->window;
  GtkStyle *style = gtk_widget_get_style (w);
  GdkColor white, black, bg, *bblack;

  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);

/*
  if(!sessionOptions->info->bgColor) {
    gdk_color_black (gdk_colormap_get_system (), &black);
    bblack = &black;
  } else
    bblack = sessionOptions->info->bgColor;
*/
  gdk_color_black (gdk_colormap_get_system (), &black);
  bblack = &black;

/*
 * the unselected variable GCs: thin lines
*/
  gg->unselvarbg_GC = gdk_gc_new (window);
  bg = style->bg[GTK_STATE_NORMAL];
  gdk_gc_set_foreground (gg->unselvarbg_GC, &bg);

  gg->unselvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->unselvarfg_GC,
                              0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->unselvarfg_GC, bblack);


/*
 * the selected variable GC: thick lines
*/
  gg->selvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->selvarfg_GC,
                              2, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->selvarfg_GC, &black);

  gg->selvarbg_GC = gdk_gc_new (window);
  gdk_gc_set_foreground (gg->selvarbg_GC, &white);

/*
 * the manip variable GCs: thin purple lines
*/
  gg->manipvarfg_GC = gdk_gc_new (window);
  gdk_gc_set_line_attributes (gg->manipvarfg_GC,
                              0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
  gdk_gc_set_foreground (gg->manipvarfg_GC, &gg->vcirc_manip_color);
}

gushort  /*-- returns the maximum color id --*/
datad_colors_used_get (gint * ncolors_used, gushort * colors_used,
                       GGobiStage * d, GGobiSession * gg)
{
  gboolean new_color;
  gint i, k, n;
  gushort colorid, maxcolorid = 0;
  GGobiStage *p = ggobi_stage_find(d, GGOBI_MAIN_STAGE_DISPLAY);

  g_return_val_if_fail(d != NULL && GGOBI_IS_STAGE(d), 0);
  
  n = 0;  /*-- *ncolors_used --*/
  GGOBI_STAGE_ATTR_INIT_ALL(p);  

  /*
   * Loop once through d->color[], collecting the colors currently
   * in use into the colors_used[] vector.
   */
  for (i = 0; i < p->n_rows; i++) {
    if (GGOBI_STAGE_GET_ATTR_HIDDEN(p, i)) {  /*-- if it's hidden, we don't care --*/
      new_color = false;
    } else {
      new_color = true;
      for (k = 0; k < n; k++) {
        if (colors_used[k] == GGOBI_STAGE_GET_ATTR_COLOR(p, i)) {
          new_color = false;
          break;
        }
      }
    }
    if (new_color) {
      colorid = GGOBI_STAGE_GET_ATTR_COLOR(p, i);
      colors_used[n] = colorid;
      maxcolorid = MAX (colorid, maxcolorid);
      (n)++;
    }
  }


  /*
   * Make sure that the current brushing color is
   * last in the list, so that it is drawn on top of
   * the pile of points.
   */
  for (k = 0; k < (n - 1); k++) {
    if (colors_used[k] == gg->color_id) {
      colors_used[k] = colors_used[n - 1];
      colors_used[n - 1] = gg->color_id;
      break;
    }
  }

  /* even if every row is filtered, this will still give a color */
  if (n == 0) {
    n = 1;
    colors_used[0] = GGOBI_STAGE_GET_ATTR_COLOR(ggobi_stage_get_root(d), 0);
  }

  *ncolors_used = n;
  return (maxcolorid);
}
