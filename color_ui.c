#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#define PSIZE 20



/*
 * Redraw one of the foreground color swatches
*/
static void
redraw_fg (GtkWidget *w, gint k, ggobid *gg) {

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &gg->default_color_table[k]);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);

  /*
   * Draw a background border around the box containing the selected color
  */
  if (k == gg->color_id) {
    gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
    gdk_draw_rectangle (w->window, gg->plot_GC,
      false, 0, 0, w->allocation.width-1, w->allocation.height-1);
    gdk_draw_rectangle (w->window, gg->plot_GC,
      false, 1, 1, w->allocation.width-2, w->allocation.height-2);
  }
}

static void
find_selection_circle_pos (icoords *pos, ggobid *gg) {
  gint i;
  glyphv g;
  gint spacing = gg->color_ui.spacing;
  gint margin = gg->color_ui.margin;

  if (gg->glyph_id.type == POINT_GLYPH) {
    pos->y = margin + 3/2;
    pos->x = spacing/2;

  } else {

    pos->y = 0;
    for (i=0; i<NGLYPHSIZES; i++) {
      g.size = i;
      pos->y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
      pos->x = spacing + spacing/2;

      if (gg->glyph_id.type == PLUS_GLYPH && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == X_GLYPH && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == OPEN_RECTANGLE && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == FILLED_RECTANGLE && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == OPEN_CIRCLE && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == FILLED_CIRCLE && gg->glyph_id.size == g.size)
        break;
    }
  }
}


static void
redraw_symbol_display (GtkWidget *w, ggobid *gg) {
  gint i;
  glyphv g;
  icoords pos;
  gint margin, spacing;

  gg->color_ui.spacing = w->allocation.width/NGLYPHTYPES;

  margin = gg->color_ui.margin;
  spacing = gg->color_ui.spacing;

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
  gdk_gc_set_foreground (gg->plot_GC, &gg->default_color_table[gg->color_id]);

  /*
   * The factor of three is dictated by the sizing of circles
   *  ... this should no longer be true; it should be 2*width + 1
  */
  pos.y = margin + 3/2;
  pos.x = spacing/2;
  gdk_draw_point (w->window, gg->plot_GC, pos.x, pos.y);

  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = i;
    pos.y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos.x = spacing + spacing/2;

    g.type = PLUS_GLYPH;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = X_GLYPH;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = OPEN_RECTANGLE;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = FILLED_RECTANGLE;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = OPEN_CIRCLE;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = FILLED_CIRCLE;
    draw_glyph (w->window, &g, &pos, 0, gg);
  }
  
  if (!gg->mono_p) {
    icoords p;
    /*-- NGLYPHSIZES is the size of the largest glyph --*/
    gint radius = (3*NGLYPHSIZES)/2 + gg->color_ui.margin/2;
    find_selection_circle_pos (&p, gg);

    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    gdk_gc_set_line_attributes (gg->plot_GC,
      2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_draw_arc (w->window, gg->plot_GC, false, p.x-radius, p.y-radius,
      2*radius, 2*radius, 0, (gshort) 23040);
    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}

static void
redraw_bg (GtkWidget *w, ggobid *gg) {

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}

static void
redraw_accent (GtkWidget *w, ggobid *gg) {

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}

void
color_changed_cb (GtkWidget *colorsel, ggobid *gg)
{
  gdouble color[3];
  GdkColor gdk_color;
  GdkColormap *cmap = gdk_colormap_get_system ();
  splotd *sp = gg->current_splot;
  GtkWidget *wheel = GTK_COLOR_SELECTION (colorsel)->wheel_area;

  /* Get current color */
  gtk_color_selection_get_color (GTK_COLOR_SELECTION (colorsel), color);

  gdk_color.red = (guint16)(color[0]*65535.0);
  gdk_color.green = (guint16)(color[1]*65535.0);
  gdk_color.blue = (guint16)(color[2]*65535.0);

  /* Allocate color */
  gdk_colormap_alloc_color (cmap, &gdk_color, false, true);

  if (gg->color_ui.current_da == gg->color_ui.bg_da) {
    gg->bg_color = gdk_color;
    redraw_bg (gg->color_ui.bg_da, gg);
  } else if (gg->color_ui.current_da == gg->color_ui.accent_da) {
    gg->accent_color = gdk_color;
    redraw_accent (gg->color_ui.accent_da, gg);
  } else {
    gg->default_color_table[gg->color_id] = gdk_color;
    redraw_fg (gg->color_ui.fg_da[gg->color_id], gg->color_id, gg);
  }

  redraw_symbol_display (gg->color_ui.symbol_display, gg);

  if (sp->da != NULL) {
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (sp->da), "expose_event",
      (gpointer) sp, (gpointer) &rval);
  }

  /*
   * If wheel doesn't have the grab, it means that the button
   * has been released:  update all plots.
  */
  if (!GTK_WIDGET_HAS_GRAB (wheel)) {
    displays_plot ((splotd *) NULL, FULL, gg);
  }
}

static gint
color_expose_fg (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  int k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));
  redraw_fg (w, k, gg);
  return FALSE;
}

static void
dlg_close_cb (GtkWidget *ok_button, ggobid* gg) {
  gtk_widget_hide (gg->color_ui.colorseldlg);
}

static gint
open_colorsel_dialog (GtkWidget *w, ggobid *gg) {
  gint handled = FALSE;
  GtkWidget *colorsel, *ok_button, *cancel_button, *help_button;
  gint i;
  gdouble color[3];

  /* Check if we've received a button pressed event */

  if (gg->color_ui.colorseldlg == NULL) {
    handled = true;

    /* Create color selection dialog */
    gg->color_ui.colorseldlg = gtk_color_selection_dialog_new ("Select color");

    /* Get the ColorSelection widget */
    colorsel = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->colorsel;

    /*
     * Connect to the "color_changed" signal, set the client-data
     * to the colorsel widget
    */
    gtk_signal_connect (GTK_OBJECT (colorsel), "color_changed",
      (GtkSignalFunc) color_changed_cb, gg);

    /*
     * Connect up the buttons
    */
    ok_button = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->ok_button;
    cancel_button = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->cancel_button;
    help_button = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->help_button;
    gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, gg);
    gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, gg);

  } else {

    colorsel = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->colorsel;

    if (w == gg->color_ui.bg_da) {
      color[0] = (gdouble) gg->bg_color.red / 65535.0;
      color[1] = (gdouble) gg->bg_color.green / 65535.0;
      color[2] = (gdouble) gg->bg_color.blue / 65535.0;

      gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);

    } else if (w == gg->color_ui.accent_da) {
      color[0] = (gdouble) gg->accent_color.red / 65535.0;
      color[1] = (gdouble) gg->accent_color.green / 65535.0;
      color[2] = (gdouble) gg->accent_color.blue / 65535.0;

      gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
    }
    else {
      for (i=0; i<NCOLORS; i++) {
        if (w == gg->color_ui.fg_da[i]) {
          color[0] = (gdouble) gg->default_color_table[i].red / 65535.0;
          color[1] = (gdouble) gg->default_color_table[i].green / 65535.0;
          color[2] = (gdouble) gg->default_color_table[i].blue / 65535.0;
          gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
        }
      }
    }
  }

  /* Show the dialog */
  gtk_widget_show (gg->color_ui.colorseldlg);

  return handled;
}

static void
set_one_color ( GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS)
    open_colorsel_dialog (w, gg);
}
static void
set_color_fg ( GtkWidget *w, GdkEventButton *event , ggobid *gg)
{
  gint i;
  gint prev = gg->color_id;
  gint k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  for (i=0; i<d->nrows; i++)
    d->color_prev[i] = d->color_ids[i];
  gg->color_id = k;

  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS) {
    open_colorsel_dialog (w, gg);
  } else {
    gint rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
      "expose_event", (gpointer) sp, (gpointer) &rval);
  }

  redraw_fg (gg->color_ui.fg_da[prev], prev, gg);
  redraw_fg (w, k, gg);
}
static gint
set_color_id (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{

  /*
   * So that the same routines can be used to handle both the foreground
   * and background color swatches, keep track of which drawing area
   * was most recently pressed.
  */
  gg->color_ui.current_da = w;

  if (w == gg->color_ui.bg_da || w == gg->color_ui.accent_da)
    set_one_color (w, event, gg);
  else
    set_color_fg (w, event, gg);

  splot_redraw (gg->current_splot, QUICK, gg);  /*-- redraw brush --*/
  return FALSE;
}
  
static gint
color_expose_bg (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_bg (w, gg);
  return FALSE;
}
static gint
color_expose_accent (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_accent (w, gg);
  return FALSE;
}

static void
choose_glyph_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
/*-- Reset glyph_id to the nearest glyph.  --*/
  glyphv g;
  gint i, dsq, nearest_dsq, type, size, rval = false;
  icoords pos, ev;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint spacing = gg->color_ui.spacing;
  gint margin = gg->color_ui.margin;

  for (i=0; i<d->nrows; i++) { 
    d->glyph_prev[i].type = d->glyph_ids[i].type;
    d->glyph_prev[i].size = d->glyph_ids[i].size;
  }

  ev.x = (gint) event->x;
  ev.y = (gint) event->y;

  pos.y = margin + 3/2;
  pos.x = spacing/2;
  g.type = POINT_GLYPH;
  g.size = 1;
  nearest_dsq = dsq = sqdist (pos.x, pos.y, ev.x, ev.y);
  type = g.type; size = g.size;

  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = i;
    pos.y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos.x = spacing + spacing/2;

    g.type = PLUS_GLYPH;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = X_GLYPH;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OPEN_RECTANGLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FILLED_RECTANGLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OPEN_CIRCLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FILLED_CIRCLE;
    dsq = sqdist (pos.x, pos.y, ev.x, ev.y);
    if (dsq < nearest_dsq) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }
  }

  gg->glyph_id.type = type;
  gg->glyph_id.size = size;
  gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
    "expose_event",
    (gpointer) sp, (gpointer) &rval);
}

static gint
color_expose_show (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_symbol_display (w, gg);

  return FALSE;
}

static void
hide_symbol_window (ggobid* gg) {

  gtk_widget_hide (gg->color_ui.symbol_window);

  if (gg->color_ui.colorseldlg != NULL &&
      GTK_IS_WIDGET (gg->color_ui.colorseldlg) &&
      GTK_WIDGET_VISIBLE (gg->color_ui.colorseldlg))
  {
     gtk_widget_hide (gg->color_ui.colorseldlg);
  }
}
/*-- catch a click on the close button --*/
static void
hide_symbol_window_cb (GtkWidget *w, ggobid* gg) {
  hide_symbol_window (gg);
}
/*-- catch the delete (close) event from the window manager --*/
static void
delete_symbol_window_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  hide_symbol_window (gg);
}

void
make_symbol_window (ggobid *gg) {

  GtkWidget *vbox, *fg_frame, *bg_frame, *accent_frame, *btn;
  GtkWidget *fg_table, *bg_table, *accent_table, *ebox;
  gint i, j, k;
  gint width, height;

  /*
   * This seems to handle the case where a the window was
   * closed using the window manager -- even though I'm capturing
   * a delete_event.
  */
  if (!GTK_IS_WIDGET (gg->color_ui.symbol_window))
    gg->color_ui.symbol_window = NULL;

  if (gg->color_ui.symbol_window == NULL) {
    gg->color_ui.symbol_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->color_ui.symbol_window),
      "color/glyph chooser");

    /*
     * I thought this would be enough to prevent the window from
     * being destroyed, but it doesn't seem to be.
    */
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_window),
                        "delete_event",
                        GTK_SIGNAL_FUNC (delete_symbol_window_cb),
                        (gpointer) gg);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->color_ui.symbol_window), vbox);

    /*
     * the current brush color and bg; the circle will be drawn
     * in the current fg (accent) color
    */
    gg->color_ui.symbol_display = gtk_drawing_area_new (); 

    /*
     * margin pixels between glyphs and at the edges
    */
    /*-- 2*(NGLYPHSIZES+1) is the size of the largest glyph --*/
    width = NGLYPHTYPES*2*(NGLYPHSIZES+1) + gg->color_ui.margin*(NGLYPHTYPES+1);

    height = gg->color_ui.margin;
    for (i=0; i<NGLYPHSIZES; i++)
      height += (gg->color_ui.margin + 2*(i+2));
    height += gg->color_ui.margin;

    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.symbol_display), width, height);
    gtk_box_pack_start (GTK_BOX (vbox), gg->color_ui.symbol_display, false, false, 0);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.symbol_display, "Click to select glyph", NULL);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_display),
      "expose_event",
      GTK_SIGNAL_FUNC (color_expose_show), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_display),
      "button_press_event",
      GTK_SIGNAL_FUNC (choose_glyph_cb), gg);

    gtk_widget_set_events (gg->color_ui.symbol_display, GDK_EXPOSURE_MASK
          | GDK_ENTER_NOTIFY_MASK
          | GDK_LEAVE_NOTIFY_MASK
          | GDK_BUTTON_PRESS_MASK);

    fg_frame = gtk_frame_new ("Foreground colors");
    gtk_box_pack_start (GTK_BOX (vbox), fg_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (fg_frame), ebox);

    fg_table = gtk_table_new (2, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), fg_table);

    k = 0;
    for (i=0; i<5; i++) {
      for (j=0; j<2; j++) {
        gg->color_ui.fg_da[k] = gtk_drawing_area_new ();
        gtk_object_set_data (GTK_OBJECT (gg->color_ui.fg_da[k]),
                             "index",
                             GINT_TO_POINTER (k));
        gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.fg_da[k]),
          PSIZE, PSIZE);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->color_ui.fg_da[k],
          "Click to select brushing color, double click to reset",
          NULL);

        gtk_widget_set_events (gg->color_ui.fg_da[k],
                               GDK_EXPOSURE_MASK
                               | GDK_ENTER_NOTIFY_MASK
                               | GDK_LEAVE_NOTIFY_MASK
                               | GDK_BUTTON_PRESS_MASK);

        gtk_signal_connect (GTK_OBJECT (gg->color_ui.fg_da[k]),
         "button_press_event",
          GTK_SIGNAL_FUNC (set_color_id), gg);
        gtk_signal_connect (GTK_OBJECT (gg->color_ui.fg_da[k]),
         "expose_event",
          GTK_SIGNAL_FUNC (color_expose_fg), gg);
        gtk_table_attach (GTK_TABLE (fg_table),
          gg->color_ui.fg_da[k], i, i+1, j, j+1,
          GTK_FILL, GTK_FILL, 10, 10);

        k++;
      }
    }

/*
 * Background color
*/

    bg_frame = gtk_frame_new ("Background color");
    gtk_box_pack_start (GTK_BOX (vbox), bg_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (bg_frame), ebox);

    bg_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), bg_table);

    gg->color_ui.bg_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.bg_da), PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.bg_da, "Double click to reset", NULL);
    gtk_widget_set_events (gg->color_ui.bg_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.bg_da), "expose_event",
      GTK_SIGNAL_FUNC (color_expose_bg), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.bg_da), "button_press_event",
      GTK_SIGNAL_FUNC (set_color_id), gg);

    gtk_table_attach (GTK_TABLE (bg_table), gg->color_ui.bg_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

/*
 * Accent color
*/
    accent_frame = gtk_frame_new ("Accent color");
    gtk_box_pack_start (GTK_BOX (vbox), accent_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (accent_frame), ebox);

    accent_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), accent_table);

    gg->color_ui.accent_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.accent_da),
      PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.accent_da, "Double click to reset color for labels and axes",
      NULL);
    gtk_widget_set_events (gg->color_ui.accent_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.accent_da),
      "expose_event", GTK_SIGNAL_FUNC (color_expose_accent), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.accent_da),
      "button_press_event", GTK_SIGNAL_FUNC (set_color_id), gg);

    gtk_table_attach (GTK_TABLE (accent_table),
      gg->color_ui.accent_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

/*
 * Close button
*/

    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn),
                        "clicked",
                        GTK_SIGNAL_FUNC (hide_symbol_window_cb),
                        (gpointer) gg);
  }

  gtk_widget_show_all (gg->color_ui.symbol_window);
}
