/* weightedvis_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gint xmargin = 20;
static gint ymargin = 20;

static GtkWidget *
get_clist_from_widget (GtkWidget *w)
{
  /*-- find the current notebook page, then get the current clist --*/
  GtkWidget *notebook = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(w), "notebook");
  gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  GtkWidget *swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
  GtkWidget *clist = GTK_BIN (swin)->child;

  return clist;
}
static gint  /*-- assumes GTK_SELECTION_SINGLE --*/
get_one_selection_from_clist (GtkWidget *clist)
{
  GList *selection = GTK_CLIST (clist)->selection;
  gint selected_var = -1;
  if (selection) selected_var = (gint) selection->data;

  return selected_var;
}
/*-------------------------------------------------------------------------*/

static void
bin_counts_reset (gint jvar, datad *d, ggobid *gg)
{
  gint i, k, m;
  gfloat val;
  gfloat min = d->vartable[jvar].lim_tform.min;
  gfloat max = d->vartable[jvar].lim_tform.max;

  for (k=0; k<gg->wvis.npct; k++)
    gg->wvis.n[k] = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    for (k=0; k<gg->ncolors; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      if (d->tform.vals[i][jvar] <= val) {
        gg->wvis.n[k]++;
        break;
      }
    }
  }
}

void
wvis_init (ggobid  *gg)
{
  GdkColormap *cmap = gdk_colormap_get_system ();
  gboolean writeable = false, best_match = true, success;

  gg->wvis.window = NULL;
  gg->wvis.npct = 0;
  gg->wvis.n = NULL;
  gg->wvis.nearest_color = -1;
  gg->wvis.motion_notify_id = 0;
  gg->wvis.mousepos.x = -1;
  gg->wvis.mousepos.y = -1;
  gg->wvis.pix = NULL;

  gg->wvis.gray1.red = gg->wvis.gray1.blue = gg->wvis.gray1.green =
    (guint16) (.3*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray1, writeable,
    best_match);
  gg->wvis.gray2.red = gg->wvis.gray2.blue = gg->wvis.gray2.green =
    (guint16) (.5*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray2, writeable,
    best_match);
  gg->wvis.gray3.red = gg->wvis.gray3.blue = gg->wvis.gray3.green =
    (guint16) (.7*65535.0);
  success = gdk_colormap_alloc_color (cmap, &gg->wvis.gray3, writeable,
    best_match);
}


static void
close_window_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->wvis.window);
}

/*
 * Use the horizontal position of the mouse to move the nearest
 * boundary
*/
static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, ggobid *gg)
{
  GdkModifierType state;
  icoords pos;
  gboolean rval = false;
  gfloat val;

  GtkWidget *clist = get_clist_from_widget (w);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist);

  icoords *mousepos = &gg->wvis.mousepos;
  gint nearest_color = gg->wvis.nearest_color;

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  if (pos.x != mousepos->x) {
    val = (gfloat) (pos.x - xmargin) /
          (gfloat) (w->allocation.width - 2*xmargin);

    /*-- don't allow it to cross its neighbors' boundaries --*/
    if (val >= gg->wvis.pct[nearest_color-1] &&
        val <= gg->wvis.pct[nearest_color+1])
    {
      gg->wvis.pct[nearest_color] = val;

      if (selected_var != -1 && selected_var < d->ncols)
        bin_counts_reset (selected_var, d, gg);

      gtk_signal_emit_by_name (GTK_OBJECT (w), "expose_event",
        "expose_event",
        (gpointer) gg, (gpointer) &rval);
    }
  }

  mousepos->x = pos.x;  

  return true;
}

/*-- when the button is pressed, listen for motion notify events --*/
static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  GdkModifierType state;
  icoords pos;
  gint k, x, y, nearest = -1, d;
  gint hgt = (w->allocation.height - 2*ymargin) / (gg->ncolors - 1);
  gint dist = w->allocation.width*w->allocation.width +
              w->allocation.height*w->allocation.height;

  gfloat *pct = gg->wvis.pct;
  gint *nearest_color = &gg->wvis.nearest_color;

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  /*-- find nearest slider --*/
  y = ymargin + 10;
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    d = (pos.x-x)*(pos.x-x) + (pos.y-y)*(pos.y-y);
    if (d < 100 && d < dist) {
      nearest = k;
      dist = d;
    }
    y += hgt;
  }

  *nearest_color = nearest;

  if (*nearest_color != -1)
    gg->wvis.motion_notify_id = gtk_signal_connect (GTK_OBJECT (w),
                                "motion_notify_event",
                                (GtkSignalFunc) motion_notify_cb,
                                (gpointer) gg);
  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if (gg->wvis.motion_notify_id) {
    gtk_signal_disconnect (GTK_OBJECT (w), gg->wvis.motion_notify_id);
    gg->wvis.motion_notify_id = 0;
  }

  return true;
}

static gint
da_configure_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  /*-- Create new backing pixmaps of the appropriate size --*/
  if (gg->wvis.pix != NULL)
    gdk_pixmap_unref (gg->wvis.pix);
  gg->wvis.pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  gtk_widget_queue_draw (w);

  return false;
}

static void
da_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint height = w->allocation.height - 2*ymargin;
  gint hgt = height / (gg->ncolors - 1);
  gint k;
  gint x0, x1;
  gint x = xmargin;
  gint y = ymargin;
  GdkPoint *points;
  gfloat diff;

  GtkWidget *clist = get_clist_from_widget (w);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist);

  GtkWidget *da = gg->wvis.da;
  GdkPixmap *pix = gg->wvis.pix;


  if (gg->wvis.npct != gg->ncolors) {
    gg->wvis.npct = gg->ncolors;
    gg->wvis.pct = (gfloat *) g_realloc (gg->wvis.pct,
                                         gg->wvis.npct * sizeof (gfloat));
    gg->wvis.n = (gint *) g_realloc (gg->wvis.n,
                                     gg->wvis.npct * sizeof (gint));
    for (k=0; k<gg->wvis.npct; k++) {
      gg->wvis.pct[k] = (gfloat) (k+1) /  (gfloat) gg->wvis.npct;
      gg->wvis.n[k] = 0;
    }
  }

  /*-- clear the pixmap --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (pix, gg->plot_GC, TRUE,
                      0, 0, w->allocation.width, w->allocation.height);


  /*-- draw the color bars --*/
  x0 = xmargin;
  for (k=0; k<gg->ncolors; k++) {
    x1 = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[k]);
    gdk_draw_rectangle (pix, gg->plot_GC,
                        TRUE, x0, ymargin, x1 - x0, height);
    x0 = x1;
  }

  /*-- draw the horizontal lines --*/
  x0 = xmargin; y = ymargin + 10;
  x1 = xmargin + (w->allocation.width - 2*xmargin) - 1;
  gdk_gc_set_foreground (gg->plot_GC, &gg->wvis.gray2);
  for (k=0; k<gg->ncolors-1; k++) {
    gdk_draw_line (pix, gg->plot_GC, x0, y, x1, y);
    y += hgt;
  }

  /*-- draw rectangles, 20 x 10 for the moment --*/
  y = ymargin + 10;
  gdk_gc_set_foreground (gg->plot_GC, &gg->wvis.gray2);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_draw_rectangle (pix, gg->plot_GC,
                        TRUE, x-10, y-5, 20, 10);
    y += hgt;
  }


  /*-- draw the dark shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->plot_GC, &gg->wvis.gray1);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);

    points [0].x = x - 10;
    points [0].y = y + 5;
    points [1].x = x + 10;
    points [1].y = y + 5;
    points [2].x = x + 10;
    points [2].y = y - 5;

    points [3].x = points[2].x - 1;
    points [3].y = points[2].y + 1;
    points [4].x = points[1].x - 1;
    points [4].y = points[1].y - 1;
    points [5].x = points[0].x + 1;
    points [5].y = points[0].y - 1;

    points [6].x = x - 10;
    points [6].y = y + 5;
    gdk_draw_polygon (pix, gg->plot_GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->plot_GC, x-1, y-4, x-1, y+3);

    y += hgt;
  }

  /*-- draw the light shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->plot_GC, &gg->wvis.gray3);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);

    points [0].x = x - 10;  /*-- lower left --*/
    points [0].y = y + 4;
    points [1].x = x - 10;  /*-- upper left --*/
    points [1].y = y - 5;
    points [2].x = x + 9;  /*-- upper right --*/
    points [2].y = y - 5;

    points [3].x = points[2].x - 1;
    points [3].y = points[2].y + 1;
    points [4].x = points[1].x + 1;
    points [4].y = points[1].y + 1;
    points [5].x = points[0].x + 1;
    points [5].y = points[0].y - 1;

    points [6].x = points[0].x;
    points [6].y = points[0].y;
    gdk_draw_polygon (pix, gg->plot_GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->plot_GC, x, y-4, x, y+3);

    y += hgt;
  }

  /*-- add the variable limits in the top margin --*/
  if (d && selected_var != -1) {
    gfloat min = d->vartable[selected_var].lim_tform.min;
    gfloat max = d->vartable[selected_var].lim_tform.max;
    gfloat val;
    gchar *str;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (da);

    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    y = ymargin;
    for (k=0; k<gg->ncolors-1; k++) {

      val = min + gg->wvis.pct[k] * (max - min);
      str = g_strdup_printf ("%3.3g", val);
      gdk_text_extents (style->font, str, strlen(str),
        &lbearing, &rbearing, &width, &ascent, &descent);
      x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
      gdk_draw_string (pix, style->font, gg->plot_GC,
        x - width/2,
        y - 2,
        str);
      g_free (str);
    }

    /*-- ... and the counts in the bottom margin --*/
    for (k=0; k<gg->ncolors; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      str = g_strdup_printf ("%d", gg->wvis.n[k]);
      gdk_text_extents (style->font, str, strlen(str),
        &lbearing, &rbearing, &width, &ascent, &descent);
      x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
      diff = (k == 0) ? gg->wvis.pct[k] : gg->wvis.pct[k]-gg->wvis.pct[k-1]; 
      x -= diff/2 * (w->allocation.width - 2*xmargin);
      gdk_draw_string (pix, style->font, gg->plot_GC,
        x - width/2,
        (w->allocation.height - ymargin) + ascent + descent + 2,
        str);
      g_free (str);
    }

  }

  gdk_draw_pixmap (w->window, gg->plot_GC, pix,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

/*-- update the contents of the clist --*/
/*
static void wvis_setdata_cb (GtkWidget *w, datad *d)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gboolean rval = false;
  GtkWidget *clist = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(w), "clist");
  gint j;
  gchar *row[1];

  GtkWidget *da = gg->wvis.da;

  gtk_clist_clear (GTK_CLIST (clist));

  gtk_clist_freeze (GTK_CLIST (clist));
  for (j=0; j<d->ncols; j++) {
    row[0] = g_strdup_printf (d->vartable[j].collab_tform);
    gtk_clist_append (GTK_CLIST (clist), row);
  }
  gtk_clist_thaw (GTK_CLIST (clist));

  gtk_object_set_data (GTK_OBJECT (clist), "datad", d);
  gtk_signal_emit_by_name (GTK_OBJECT (da), "expose_event",
    "expose_event",
    (gpointer) gg, (gpointer) &rval);
}
*/

void
selection_made_cb (GtkWidget *clist, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gboolean rval = false;
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");

  bin_counts_reset (row, d, gg);

  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

static void scale_apply_cb (GtkWidget *w, ggobid* gg)
{
  GtkWidget *clist = get_clist_from_widget (w);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist);

  if (d && selected_var != -1) {
    gint i, m, k;
    gfloat min = d->vartable[selected_var].lim_tform.min;
    gfloat max = d->vartable[selected_var].lim_tform.max;
    gfloat val;

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];

      for (k=0; k<gg->ncolors; k++) {
        val = min + gg->wvis.pct[k] * (max - min);
        if (d->tform.vals[i][selected_var] <= val) {
          d->color.els[i] = d->color_now.els[i] = k;
          break;
        }
      }
    }
    clusters_set (d, gg);
    displays_plot (NULL, FULL, gg);
  }
}

void
wvis_window_open (ggobid *gg) {
  GtkWidget *vbox, *hb;
  GtkWidget *notebook, *swin, *clist;
  GtkWidget *btn, *vb;
  gint nd = g_slist_length (gg->d);
  gint j;
  GSList *l;
  datad *d;
  gchar *row[1];

  if (gg->wvis.window == NULL) {

    gg->wvis.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis.window),
      "brushing by weights");
    gtk_signal_connect (GTK_OBJECT (gg->wvis.window),
      "delete_event", GTK_SIGNAL_FUNC (close_window_cb), gg);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
    gtk_container_add (GTK_CONTAINER (gg->wvis.window), vbox);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vbox, GTK_SELECTION_SINGLE,
      (GtkSignalFunc) selection_made_cb, gg);

    /*-- colors, symbols --*/
    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, true, true, 0);
    gg->wvis.da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->wvis.da), 400, 200);
    gtk_object_set_data (GTK_OBJECT (gg->wvis.da), "notebook", notebook);
    gtk_box_pack_start (GTK_BOX (vb), gg->wvis.da, true, true, 0);

    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "configure_event",
                        (GtkSignalFunc) da_configure_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "expose_event",
                        (GtkSignalFunc) da_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "button_press_event",
                        (GtkSignalFunc) button_press_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "button_release_event",
                        (GtkSignalFunc) button_release_cb,
                        (gpointer) gg);

    gtk_widget_set_events (gg->wvis.da, GDK_EXPOSURE_MASK
               | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
               | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    btn = gtk_button_new_with_label ("Apply");
    gtk_object_set_data (GTK_OBJECT (btn), "notebook", notebook);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Apply the color scale", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_apply_cb), gg);

    /*-- add a close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(), false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_with_label ("Close");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 2);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (close_window_cb), gg);
  }

  gtk_widget_show_all (gg->wvis.window);
  gdk_window_raise (gg->wvis.window->window);
}
