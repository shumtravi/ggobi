/* tour2d_ui.c */

#include <strings.h>
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

static void tour2dadv_window_open (ggobid *gg);

void speed_set (gint speed) {
  g_printerr ("speed=%d\n", speed);
}
static void speed_set_cb (GtkAdjustment *adj, gpointer cbd) {
  speed_set ((gint)adj->value);
}

/*-- not a callback, but an initialization routine for the scrollbar --*/
static void scale_set_default_values (GtkScale *scale )
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_draw_value (scale, false);
}

void tour2d_pause (cpaneld *cpanel, gboolean state) {
  extern void tour_func (gboolean);
  cpanel->is_tour_paused = state;

  tour_func (!cpanel->is_tour_paused);
}

static void tour2d_pause_cb (GtkToggleButton *button, ggobid *gg)
{
  tour2d_pause (&gg->current_display->cpanel, button->active);
}

static void reinit_cb (GtkWidget *w) {
  g_printerr ("reinit\n");
}
static void pcaxes_cb (GtkToggleButton *button)
{
  g_printerr ("pcaxes: %d\n", button->active);
}


static void tour2dpp_cb (GtkWidget *w, ggobid *gg) 
{
  g_printerr ("open projection pursuit panel\n");
  tour2dpp_window_open (gg);
}

static void tour2dadv_cb (GtkWidget *w, ggobid *gg) {
  g_printerr ("open advanced tour features panel\n");
  tour2dadv_window_open (gg);
}

static gchar *manip_lbl[] = {"Oblique", "Vert", "Horiz", "Radial",
                             "Angular"};
static void manip_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", manip_lbl[indx]);
}

void
cpanel_tour2d_make (ggobid *gg) {
  GtkWidget *box, *tgl, *btn, *sbar, *lbl, *vb;
  GtkObject *adj;
  GtkWidget *manip_opt;
  
  gg->control_panel[TOUR2D] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[TOUR2D]), 5);

/*
 * speed scrollbar
*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */
  adj = gtk_adjustment_new (1.0, 0.0, 100.0, 1.0, 1.0, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (speed_set_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
    "Adjust speed of tour motion", NULL);
  scale_set_default_values (GTK_SCALE (sbar));

  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]), sbar,
    false, false, 1);

/*
 * Box to hold 'pause' toggle and 'reinit' button
*/
  box = gtk_hbox_new (true, 2);

  btn = gtk_check_button_new_with_label ("Pause");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Stop tour motion temporarily", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (tour2d_pause_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Reset projection", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                     GTK_SIGNAL_FUNC (reinit_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]), box, false, false, 1);


/*
 * manipulation option menu and label inside vbox
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]), vb, false, false, 0);

  lbl = gtk_label_new ("Manual manipulation:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  manip_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), manip_opt,
    "Set the manual manipulation method", NULL);
  gtk_box_pack_end (GTK_BOX (vb), manip_opt, false, false, 0);
  populate_option_menu (manip_opt, manip_lbl,
                        sizeof (manip_lbl) / sizeof (gchar *),
                        manip_cb, gg);

/*
 * PC Axes toggle
*/
  tgl = gtk_check_button_new_with_label ("PC axes");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
    "Show principal component axes or plain variable axes", NULL);
  gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                      GTK_SIGNAL_FUNC (pcaxes_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]),
                      tgl, false, false, 1);

/*
 * projection pursuit button
*/
  btn = gtk_button_new_with_label ("Projection pursuit ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Open panel for grand tour projection pursuit", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (tour2dpp_cb), gg);

/*
 * advanced features button
*/
  btn = gtk_button_new_with_label ("Advanced features ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Open panel for additional grand tour features", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TOUR2D]),
                      btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (tour2dadv_cb), gg);

  gtk_widget_show_all (gg->control_panel[TOUR2D]);
}


/*----------------------------------------------------------------------*/
/*               Advanced features panel and callbacks                  */
/*----------------------------------------------------------------------*/

/*

The following are considered advanced features for now:
  local tour
  step/go tour
  interpolation methods (geodesic, HH, Givens)
  path length
  history

  section tour
*/

static GtkWidget *window = NULL;

static gchar *pathlen_lbl[] = {"1/10", "1/5", "1/4", "1/3", "1/2", "1",
                               "2", "10", "Infinite"};
static void pathlen_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", pathlen_lbl[indx]);
}

static gchar *interp_lbl[] = {"Geodesic", "Householder", "Givens"};
static void interp_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", interp_lbl[indx]);
}

static void localscan_cb (GtkToggleButton *button)
{
  g_printerr ("local scan: %d\n", button->active);
}

static void step_cb (GtkToggleButton *tgl, GtkWidget *btn)
{
  g_printerr ("step: %d\n", tgl->active);
  gtk_widget_set_sensitive (btn, tgl->active);
}
static void go_cb (GtkButton *button, ggobid *gg)
{
  displayd *dsp = gg->current_display; 

  g_printerr ("go\n");
  g_printerr ("in go_cb %f \n",dsp->tau[0]);

  tour_do_step (dsp, gg);
}

static void storebases_cb (GtkToggleButton *button)
{
  g_printerr ("store bases: %d\n", button->active);
}

/* 
 * Section callbacks
*/
static void section_cb (GtkToggleButton *button)
{
  g_printerr ("local scan: %d\n", button->active);
}
static void epsilon_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("epsilon %f\n", adj->value);
}

static void hide_cb (GtkWidget *w ) {
  gtk_widget_hide (w);
}

static void tour2dadv_window_open (ggobid *gg) {
  GtkWidget *vbox, *box, *btn, *opt, *tgl, *entry;
  GtkWidget *pathlen_opt, *vb, *hb, *lbl, *sbar, *notebook;
  GtkObject *adj;

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "advanced tour");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* Create a new notebook, place the position of the tabs */
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    gtk_container_add (GTK_CONTAINER (window), notebook);

/*-- vbox to be placed in the notebook page --*/
    vbox = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);

    /*-- local scan toggle --*/
    tgl = gtk_check_button_new_with_label ("Local scan");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Perform the tour within a small local region", NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (localscan_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox),
                        tgl, false, false, 1);

    /*-- Box to hold 'step' toggle and 'go' button --*/
    box = gtk_hbox_new (true, 2);

    tgl = gtk_check_button_new_with_label ("Step");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Prepare to run the grand tour one step at a time", NULL);
    gtk_box_pack_start (GTK_BOX (box), tgl, true, true, 1);

    btn = gtk_button_new_with_label ("Go");
    gtk_widget_set_sensitive (btn, false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Take one step of the grand tour", NULL);
    gtk_box_pack_start (GTK_BOX (box), btn, true, true, 1);

    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                       GTK_SIGNAL_FUNC (go_cb), (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (step_cb), GTK_WIDGET (btn));

    gtk_box_pack_start (GTK_BOX (vbox), box, false, false, 1);

    lbl = gtk_label_new ("General");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, lbl);

    /*-- path length option menu inside frame --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    lbl = gtk_label_new ("Path length:");
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    pathlen_opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), pathlen_opt,
      "Set the path length", NULL);
    gtk_box_pack_end (GTK_BOX (hb), pathlen_opt, false, false, 0);
    populate_option_menu (pathlen_opt, pathlen_lbl,
                          sizeof (pathlen_lbl) / sizeof (gchar *),
                          pathlen_cb, gg);

    /*-- interpolation option menu inside hbox --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    lbl = gtk_label_new ("Interpolation: ");
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Set the interpolation method", NULL);
    gtk_box_pack_end (GTK_BOX (hb), opt, false, false, 0);
    populate_option_menu (opt, interp_lbl,
                          sizeof (interp_lbl) / sizeof (gchar *),
                          interp_cb, gg);

/*-- tour history functions: vbox to be placed in the notebook page --*/
    vb = gtk_vbox_new (true, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    lbl = gtk_label_new ("History");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vb, lbl);

    /*-- Store bases toggle --*/
    tgl = gtk_check_button_new_with_label ("Store bases");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Store basis vectors", NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (storebases_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vb), tgl, false, false, 0);

    /*-- Number of bases stored; a label and a text entry --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 0);

    lbl = gtk_label_new ("Number of bases stored:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    entry = gtk_entry_new ();
    gtk_widget_set_usize (entry,
                          gdk_string_width (entry->style->font, "XXXX"),
                          -1);
    gtk_box_pack_end (GTK_BOX (hb), entry, false, false, 0);

    /*-- Number of bases stored; a label and a text entry --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 0);

    lbl = gtk_label_new ("Current base pair: ");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

    entry = gtk_entry_new ();
    gtk_widget_set_usize (entry,
                          gdk_string_width (entry->style->font, "XXXX"),
                          -1);
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 0);
    entry = gtk_entry_new ();
    gtk_widget_set_usize (entry,
                          gdk_string_width (entry->style->font, "XXXX"),
                          -1);
    gtk_box_pack_end (GTK_BOX (hb), entry, false, false, 0);

    /*-- Return to basis x --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 0);

    tgl = gtk_check_button_new_with_label ("Return to basis");
    gtk_box_pack_start (GTK_BOX (hb), tgl, false, false, 0);

    entry = gtk_entry_new ();
    gtk_widget_set_usize (entry,
                          gdk_string_width (entry->style->font, "XXXX"),
                          -1);
    gtk_box_pack_end (GTK_BOX (hb), entry, false, false, 0);

    /*-- Display basis as bitmap --*/
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 0);

    tgl = gtk_check_button_new_with_label ("Display basis");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Display basis as bitmap", NULL);
    gtk_box_pack_start (GTK_BOX (hb), tgl, false, false, 0);

    entry = gtk_entry_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "Enter bitmap number", NULL);
    gtk_widget_set_usize (entry,
                          gdk_string_width (entry->style->font, "XXXX"),
                          -1);
    gtk_box_pack_end (GTK_BOX (hb), entry, false, false, 0);

/*-- section tour widgets: vbox to be placed in the notebook page --*/
    box = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box), 4);
    lbl = gtk_label_new ("Section");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box, lbl);

    tgl = gtk_check_button_new_with_label ("Section");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Downlight points that are not within epsilon of the center plane",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (section_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (box), tgl, false, false, 1);

    /*-- vbox for label and rangewidget --*/
    vb = gtk_vbox_new (true, 0);
    gtk_box_pack_start (GTK_BOX (box), vb, false, false, 1);

    lbl = gtk_label_new ("Epsilon:");
    gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

    adj = gtk_adjustment_new (1.0, 0.0, 1.0, 0.01, .01, 0.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (epsilon_cb), NULL);

    sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), sbar,
      "Set the width of the cross-section",
      NULL);
    gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (sbar), 2);
    gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
    gtk_box_pack_start (GTK_BOX (vb), sbar, false, false, 0);

    /*-- Close button --*/
    btn = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (btn), "clicked",
                   GTK_SIGNAL_FUNC (hide_cb), (gpointer) window);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, true, 2);
  }

  gtk_widget_show_all (window);

}

/*----------------------------------------------------------------------*/
/*                              I/O events                              */
/*----------------------------------------------------------------------*/

static void tour2d_io_cb (GtkWidget *w, gpointer *cbd) {
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
}

/*----------------------------------------------------------------------*/
/*               Handling mouse events in the plot window               */
/*----------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(gt_motion_notify_cb)\n");

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  gg->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

  gg->mousepos.x = event->x;
  gg->mousepos.y = event->y;

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
                                      (gpointer) sp);

  return true;
}
static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);

  gg->mousepos.x = event->x;
  gg->mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

void
tour2d_event_handlers_toggle (splotd *sp, gboolean state) {
  if (state == on) {
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
  } else {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

/*----------------------------------------------------------------------*/
/*                   Resetting the main menubar                         */
/*----------------------------------------------------------------------*/



void
tour2d_menus_make (ggobid *gg) {
  GtkWidget *item;

/*
 * I/O menu
*/
  gg->app.tour2d_io_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Save coefficients");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "write_coeffs");
  gtk_menu_append (GTK_MENU (gg->app.tour2d_io_menu), item);

  item = gtk_menu_item_new_with_label ("Save history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "write_history");
  gtk_menu_append (GTK_MENU (gg->app.tour2d_io_menu), item);

  item = gtk_menu_item_new_with_label ("Read history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "read_history");
  gtk_menu_append (GTK_MENU (gg->app.tour2d_io_menu), item);

  gtk_widget_show_all (gg->app.tour2d_io_menu);
}

/*----------------------------------------------------------------------*/
/*                   End of main menubar section                        */
/*----------------------------------------------------------------------*/
