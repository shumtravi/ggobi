/*-- cluster_ui.c  --*/
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

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "pipeline.h"
#include "utils_ui.h"

static void exclusion_notebook_adddata_cb (GGobiSession *, GGobiStage *,
                                           void *notebook);

static void
destroyit (gboolean kill, GGobiSession * gg)
{
  gint n, nrows;
  GSList *l;
  GGobiStage *d;
  GtkWidget *child;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    if (d->cluster_table) {
      nrows = GTK_TABLE (d->cluster_table)->nrows;
      for (n = 0; n < nrows - 1; n++)
        cluster_free (n, d, gg);
    }
  }

  if (kill) {
    gtk_widget_destroy (gg->cluster_ui.window);
    gg->cluster_ui.window = NULL;
  }
  else {
    /*-- kill all the children of the window --*/
    GList *gl, *children =
      gtk_container_get_children (GTK_CONTAINER(GTK_DIALOG(gg->cluster_ui.window)->vbox));
    for (gl = children; gl; gl = gl->next) {
      child = (GtkWidget *) gl->data;
      gtk_widget_destroy (child);
    }
  }
}

/*-- called when closed from the close button --*/
static void
close_btn_cb (GtkWidget * w, gint response, GGobiSession * gg)
{
  destroyit (true, gg);
}

/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget * w, GdkEvent * event, GGobiSession * gg)
{
  destroyit (true, gg);
}

static gint
cluster_symbol_show (GtkWidget * w, GdkEventExpose * event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  GGobiSession *gg = GGobiFromWidget (w, true);
  icoords pos;
  glyphd g;
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  colorschemed *scheme = gg->activeColorScheme;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (w->window, gg->plot_GC,
                      true, 0, 0, w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[d->clusv[k].color]);
  g.type = d->clusv[k].glyphtype;
  g.size = d->clusv[k].glyphsize;

  pos.x = w->allocation.width / 2;
  pos.y = w->allocation.height / 2;
  draw_glyph (w->window, &g, &pos, 0, gg);

  return FALSE;
}

void
cluster_table_labels_update (GGobiStage * d, GGobiSession * gg)
{
  gint k;
  gchar *str;

  if (gg->cluster_ui.window == NULL)
    return;

  for (k = 0; k < d->nclusters; k++) {
    str = g_strdup_printf ("%ld", d->clusv[k].nhidden);
    gtk_label_set_text (GTK_LABEL (d->clusvui[k].nh_lbl), str);
    g_free (str);

    str = g_strdup_printf ("%ld", d->clusv[k].nshown);
    gtk_label_set_text (GTK_LABEL (d->clusvui[k].ns_lbl), str);
    g_free (str);

    str = g_strdup_printf ("%ld", d->clusv[k].n);
    gtk_label_set_text (GTK_LABEL (d->clusvui[k].n_lbl), str);
    g_free (str);
  }
}


static gint
hide_cluster_cb (GtkToggleButton * btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  gint i;
  GGobiSession *gg = GGobiFromWidget (GTK_WIDGET (btn), true);
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  gboolean changed = false;

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  /*-- operating on the current sample, whether hidden or shown --*/
  for (i = 0; i < d->n_rows; i++) {
    //if (GGOBI_STAGE_GET_ATTR_SAMPLED(d)) {
      if (GGOBI_STAGE_GET_ATTR_CLUSTER(d, i) == k) {
        if (GGOBI_STAGE_SET_ATTR_HIDDEN(d, i, btn->active, ATTR_SET_PERSISTENT)) {
          changed = brush_all_matching_id (d, i, true, br_shadow, ATTR_SET_PERSISTENT) || changed;          
        }
      }
    //}
  }

  clusters_set(d);
  cluster_table_labels_update (d, gg);

  //if (changed) {
    ggobi_stage_update_col(d, ggobi_stage_get_col_index_for_name(d, "_hidden"));
    ggobi_stage_update_col(d, ggobi_stage_get_col_index_for_name(d, "_hidden_prev"));
    ggobi_stage_update_col(d, ggobi_stage_get_col_index_for_name(d, "_hidden_now"));
    ggobi_stage_flush_changes(d);
  //}
    
  //displays_plot (NULL, FULL, gg);

  return false;
}

/*-- include or exclude hidden cases --*/
gint
include_hiddens (gboolean include, GGobiStage * d, GGobiSession * gg)
{
  // gint i;
  // displayd *dsp = gg->current_display;
  // cpaneld *cpanel = &dsp->cpanel;
  // gboolean prev, changed = false;
  // GGobiStage *f = ggobi_stage_find(ggobi_stage_get_root(d), GGOBI_MAIN_STAGE_FILTER);
  // 
  // GGOBI_STAGE_BRUSH_ATTR_INIT(f, hidden);  
  // for (i = 0; i < d->n_rows; i++) {
  //   prev = ggobi_stage_filter_is_included(GGOBI_STAGE_FILTER(f), i);
  //   ggobi_stage_filter_set_included(GGOBI_STAGE_FILTER(f), i, 
  //     (include || !GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)));
  //   if ((prev != ggobi_stage_filter_is_included(GGOBI_STAGE_FILTER(f), i)) 
  //       && !gg->linkby_cv) 
  //     /*-- this doesn't link the value of excluded --*/
  //     changed = changed || brush_all_matching_id (d, i, true, 
  //       br_include, ATTR_SET_PERSISTENT);
  // }
  // // ggobi_stage_filter_update(GGOBI_STAGE_FILTER(f));
  // 
  // /*-- make the other datad's update their rows_in_plot, too --*/
  // /* FIXME: Are we sure that this even works? Do we really want to do this? - mfl */
  // if (changed) {
  //   GGobiStage *dd;
  //   GSList *l;
  //   for (l = gg->d; l; l = l->next) {
  //     dd = (GGobiStage *) l->data;
  //     if (dd == ggobi_stage_get_root(d))
  //       continue;
  //     clusters_set(d);
  //     cluster_table_labels_update (dd, gg);
  //     /*limits_set (dd, gg->lims_use_visible);*/
  //     vartable_limits_set (dd);
  //     vartable_stats_set (dd);
  //     tform_to_world(dd);
  //   }
  // }
  // 
  // clusters_set(f);
  // cluster_table_labels_update (f, gg);
  // /*limits_set (f, gg->lims_use_visible);*/
  // vartable_limits_set (f);
  // vartable_stats_set (f);
  // tform_to_world(f);
  // 
  // if (cpanel->pmode == TOUR1D)
  //   dsp->t1d.get_new_target = true;
  // else if (cpanel->pmode == TOUR2D3)
  //   dsp->t2d3.get_new_target = true;
  // else if (cpanel->pmode == TOUR2D)
  //   dsp->t2d.get_new_target = true;
  // else if (cpanel->pmode == COTOUR) {
  //   dsp->tcorr1.get_new_target = true;
  //   dsp->tcorr2.get_new_target = true;
  // }
  // 
  // displays_tailpipe (FULL, gg);
  // displays_plot (NULL, FULL, gg);
  // 
  return false;
}

static void
exclude_hiddens_cb (GtkWidget * w, GGobiSession * gg)
{
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  include_hiddens (false, d, gg);
}
static void
include_hiddens_cb (GtkWidget * w, GGobiSession * gg)
{
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  include_hiddens (true, d, gg);
}

static gint
cluster_symbol_cb (GtkWidget * w, GdkEventExpose * event, gpointer cbd)
{
  /*-- reset the glyph and color of this glyph to the current values --*/
  gint n = GPOINTER_TO_INT (cbd);
  GGobiSession *gg = GGobiFromWidget (w, true);
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  GGobiStage *f = ggobi_stage_find(ggobi_stage_get_root(d), GGOBI_MAIN_STAGE_FILTER);
  gint k, i;
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean rval = false;
  gint nclusters = symbol_table_populate (d);
  gboolean proceed = true;
  gint targets = cpanel->br.point_targets;
  gint nd = g_slist_length (gg->d);

/*
 * Almost surely the user is not trying to collapse groups, so
 * check whether there's another cluster with this color/glyph
 * combination.  If there is, don't go ahead.  (Though we may
 * want to add a dialog for this later.)
*/
  for (k = 0; k < nclusters; k++) {
    if (k != n) {
      switch (targets) {
      case br_candg:
        if (d->clusv[k].glyphtype == gg->glyph_id.type &&
            d->clusv[k].glyphsize == gg->glyph_id.size &&
            d->clusv[k].color == gg->color_id) {
          proceed = false;
          break;
        }
        break;
      case br_color:
        /*
         * This would produce an identical cluster if the glyph
         * types and sizes of the clusters are already equal, and
         * the new glyph sizes would complete the match.
         */
        if (d->clusv[k].glyphtype == d->clusv[n].glyphtype &&
            d->clusv[k].glyphsize == d->clusv[n].glyphsize &&
            d->clusv[k].color == gg->color_id) {
          proceed = false;
          break;
        }
        break;
      case br_glyph:
        if (d->clusv[k].color == d->clusv[n].color &&
            d->clusv[k].glyphtype == gg->glyph_id.type &&
            d->clusv[k].glyphsize == gg->glyph_id.size) {
          proceed = false;
          break;
        }
        break;
      }
    }
  }

/*
 * This is a bit paternalistic, no?  But it's probably ok for now.
*/
  if (!proceed) {
    quick_message
      ("You're about to reset the color and/or glyph for this cluster\nin such a way as to merge it with another cluster.  I bet\nthat's not what you intend, so I won't let you do it.\n",
       false);
    return true;
  }

  GGOBI_STAGE_ATTR_INIT_ALL(f);  
  for (i = 0; i < f->n_rows; i++) {
    if (GGOBI_STAGE_GET_ATTR_CLUSTER(f, i) == n) {
      GGOBI_STAGE_BRUSH_POINT(f, i, true, targets, ATTR_SET_PERSISTENT);
      
      /*-- link so that displays of linked datad's will be brushed as well --*/
      if (nd > 1 && !gg->linkby_cv)
        brush_all_matching_id (f, i, true, targets, ATTR_SET_PERSISTENT);
    }
  }

  g_signal_emit_by_name (G_OBJECT (w), "expose_event",
                         (gpointer) gg, (gpointer) & rval);

  clusters_set(d);

  displays_plot (NULL, FULL, gg);

  return false;
}

void
cluster_add (gint k, GGobiStage * d, GGobiSession * gg)
{
  gchar *str;
  gint dawidth = 2 * NGLYPHSIZES + 1 + 10;

  d->clusvui[k].da = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (d->clusvui[k].da, false);
  gtk_widget_set_size_request (GTK_WIDGET (d->clusvui[k].da),
                               dawidth, dawidth);

  gtk_widget_set_events (d->clusvui[k].da,
                         GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK
                         | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

  g_signal_connect (G_OBJECT (d->clusvui[k].da), "expose_event",
                    G_CALLBACK (cluster_symbol_show), GINT_TO_POINTER (k));
  g_signal_connect (G_OBJECT (d->clusvui[k].da), "button_press_event",
                    G_CALLBACK (cluster_symbol_cb), GINT_TO_POINTER (k));
  ggobi_widget_set (d->clusvui[k].da, gg, true);
  gtk_table_attach (GTK_TABLE (d->cluster_table), d->clusvui[k].da,
                    0, 1, k + 1, k + 2,
                    (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 2);


  // Set clusv[k].hidden_p in case the user has made changes.
  d->clusv[k].hidden_p = (d->clusv[k].nhidden == d->clusv[k].n);

  d->clusvui[k].h_btn = gtk_toggle_button_new_with_label ("Shadow");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->clusvui[k].h_btn),
                                d->clusv[k].hidden_p);
  g_signal_connect (G_OBJECT (d->clusvui[k].h_btn), "toggled",
                    G_CALLBACK (hide_cluster_cb), GINT_TO_POINTER (k));
  ggobi_widget_set (d->clusvui[k].h_btn, gg, true);
  gtk_table_attach (GTK_TABLE (d->cluster_table),
                    d->clusvui[k].h_btn,
                    1, 2, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);

/*
  d->clusvui[k].e_btn = gtk_toggle_button_new_with_label("E");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(d->clusvui[k].e_btn),
    d->clusv[k].excluded_p);
  g_signal_connect(G_OBJECT(d->clusvui[k].e_btn), "toggled",
    G_CALLBACK(exclude_cluster_cb), GINT_TO_POINTER(k));
  ggobi_widget_set(d->clusvui[k].e_btn, gg, true);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].e_btn,
    2, 3, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
*/

  str = g_strdup_printf ("%ld", d->clusv[k].nhidden);
  d->clusvui[k].nh_lbl = gtk_label_new (str);
  gtk_table_attach (GTK_TABLE (d->cluster_table),
                    d->clusvui[k].nh_lbl,
                    2, 3, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free (str);

  str = g_strdup_printf ("%ld", d->clusv[k].nshown);
  d->clusvui[k].ns_lbl = gtk_label_new (str);
  gtk_table_attach (GTK_TABLE (d->cluster_table),
                    d->clusvui[k].ns_lbl,
                    3, 4, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free (str);

  str = g_strdup_printf ("%ld", d->clusv[k].n);
  d->clusvui[k].n_lbl = gtk_label_new (str);
  gtk_table_attach (GTK_TABLE (d->cluster_table),
                    d->clusvui[k].n_lbl,
                    4, 5, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free (str);
}

void
cluster_free (gint k, GGobiStage * d, GGobiSession * gg)
{
  if (d->clusvui[k].da) {
    gtk_widget_destroy (d->clusvui[k].da);
    gtk_widget_destroy (d->clusvui[k].h_btn);
    gtk_widget_destroy (d->clusvui[k].nh_lbl);
    gtk_widget_destroy (d->clusvui[k].ns_lbl);
    gtk_widget_destroy (d->clusvui[k].n_lbl);
  }
}


static void
update_cb (GtkWidget * w, GGobiSession * gg)
{
  GGobiStage *d = datad_get_from_notebook(gg->cluster_ui.notebook);
  splotd *sp = gg->current_splot;

  //ggobi_stage_set_rows_in_plot(d);
  if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
    void (*f) (GGobiStage *, splotd *, GGobiSession *);
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
    f = klass->splot_assign_points_to_bins;
    if (f) {
      f (d, sp, gg);            // need to exclude area plots
    }
  }
  //assign_points_to_bins(d, sp, gg);
  clusters_set(d);

  cluster_table_labels_update (d, gg);
  displays_plot (NULL, FULL, gg);

  cluster_window_open (gg);
}

static gboolean
nclusters_changed (GGobiSession * gg)
{
  GGobiStage *d;
  gint k, nrows = 0;
  GtkWidget *page;
  gboolean changed = false;
  gint nd = g_slist_length (gg->d);

  for (k = 0; k < nd; k++) {
    nrows = 0;
    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (gg->cluster_ui.notebook),
                                      k);
    if (page) {
      d = (GGobiStage *) g_object_get_data (G_OBJECT (page), "datad");
      nrows = GTK_TABLE (d->cluster_table)->nrows;

      if (nrows != d->nclusters + 1) {/*-- add one for the titles --*/
        changed = true;
      }
    }
    else {    /*-- if page is NULL, a new datad has been added --*/
      changed = true;
    }
    if (changed)
      break;
  }
  return changed;
}


void
cluster_table_update (GGobiStage * d, GGobiSession * gg)
{
  if (gg->cluster_ui.window != NULL) {
    if (nclusters_changed (gg)) {  /*-- for any of the datad's --*/
      cluster_window_open (gg);
    }
    else {
      cluster_table_labels_update (d, gg);  /*-- d, or all d's? --*//* do all */
    }
  }
}

static void
exclusion_notebook_adddata_cb (GGobiSession * gg, GGobiStage * d, void *notebook)
{
  cluster_window_open (gg);
  return;                       /* Should this return a boolean? */
}

CHECK_EVENT_SIGNATURE (exclusion_notebook_adddata_cb, datad_added_f)


     void cluster_window_open (GGobiSession * gg)
{
  GtkWidget *scrolled_window = NULL;
  GtkWidget *tebox, *btn, *hbox, *lbl;
  GtkWidget *ebox, *dialog;
  gint k;
  GSList *l;
  GGobiStage *d;
  gboolean new = false;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0)
    /**/ return;

  /*-- if it isn't NULL, then destroy it and start afresh --*/
  if (gg->cluster_ui.window != NULL) {
    destroyit (false, gg);  /*-- don't kill the whole thing --*/
  }

  if (gg->cluster_ui.window == NULL ||
      !GTK_WIDGET_REALIZED (gg->cluster_ui.window)) {
    gg->cluster_ui.window = gtk_dialog_new_with_buttons ("Color & Glyph Groups",
      GTK_WINDOW(gg->main_window), GTK_DIALOG_DESTROY_WITH_PARENT, 
      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
    g_signal_connect (G_OBJECT (gg->cluster_ui.window), "delete_event",
                      G_CALLBACK (close_wmgr_cb), (gpointer) gg);
    new = true;
  }
  
  dialog = gg->cluster_ui.window;

  tebox = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), tebox, true, true, 2);

  /* Create a notebook, set the position of the tabs */
  gg->cluster_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->cluster_ui.notebook),
                            GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->cluster_ui.notebook),
                              g_slist_length (gg->d) > 1);
  gtk_container_add (GTK_CONTAINER (tebox), gg->cluster_ui.notebook);

  for (l = gg->d; l; l = l->next) {
    d = ggobi_stage_find((GGobiStage *) l->data, GGOBI_MAIN_STAGE_SUBSET);

    /*-- skip datasets without variables --*/
    if (!ggobi_stage_has_vars(d))
      continue;

    /* Create a scrolled window to hold the table */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    g_object_set_data (G_OBJECT (scrolled_window), "datad", d); /*setdata */
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->cluster_ui.notebook),
      scrolled_window, gtk_label_new (((GGobiStage *)l->data)->name));
    gtk_widget_show (scrolled_window);

    d->cluster_table = gtk_table_new (d->nclusters + 1, 5, true);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW
                                           (scrolled_window),
                                           d->cluster_table);

    /*-- add the row of titles --*/

    ebox = gtk_event_box_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
                          "Click to change the color/glyph of all members of the selected cluster to the current brushing color/glyph",
                          NULL);
    lbl = gtk_label_new ("Symbol");
    gtk_container_add (GTK_CONTAINER (ebox), lbl);
    gtk_table_attach (GTK_TABLE (d->cluster_table), ebox, 0, 1, 0, 1,
      /*-- left, right, top, bottom --*/
                      GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
                          "Shadow brush all cases with the corresponding symbol.",
                          NULL);
    lbl = gtk_label_new ("Shadow");
    gtk_container_add (GTK_CONTAINER (ebox), lbl);
    gtk_table_attach (GTK_TABLE (d->cluster_table), ebox,
                      1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

/*
    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Exclude all hidden cases with the corresponding symbol",
      NULL);
    lbl = gtk_label_new("Exclude");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
*/

/*
    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Show all cases with the corresponding symbol",
      NULL);
    lbl = gtk_label_new("Show");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Complement: Show/hide all cases with the corresponding symbol that are hidden/shown",
      NULL);
    lbl = gtk_label_new("Comp");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      3, 4, 0, 1, GTK_FILL, GTK_FILL, 5, 2);
*/

    ebox = gtk_event_box_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
                          "The number of cases in shadow out of N with the corresponding symbol.",
                          NULL);
    lbl = gtk_label_new ("Shadowed");
    gtk_container_add (GTK_CONTAINER (ebox), lbl);
    gtk_table_attach (GTK_TABLE (d->cluster_table), ebox,
                      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
                          "The number of visible cases (cases not in shadow) out of N with the corresponding symbol.",
                          NULL);
    lbl = gtk_label_new ("Shown");
    gtk_container_add (GTK_CONTAINER (ebox), lbl);
    gtk_table_attach (GTK_TABLE (d->cluster_table), ebox,
                      3, 4, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), ebox,
                          "The number of cases with the corresponding symbol.  If sampling, the number of cases in the current subsample",
                          NULL);
    lbl = gtk_label_new ("N");
    gtk_container_add (GTK_CONTAINER (ebox), lbl);
    gtk_table_attach (GTK_TABLE (d->cluster_table), ebox,
                      4, 5, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    d->clusvui = (clusteruid *)
      g_realloc (d->clusvui, d->nclusters * sizeof (clusteruid));
    /*-- add the cluster rows, one by one --*/
    for (k = 0; k < d->nclusters; k++)
      cluster_add (k, d, gg);
  }

  /*-- listen for datad_added events on main_window --*/
  /*-- Be careful to add this signal handler only once! --*/
  if (new) {
    g_signal_connect (G_OBJECT (gg),
                      "datad_added",
                      G_CALLBACK (exclusion_notebook_adddata_cb), NULL);
  }

  /*-- give the window an initial height --*/
  gtk_widget_set_size_request (GTK_WIDGET (scrolled_window), -1, 150);

  /*-- horizontal box to hold a few buttons --*/
  hbox = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), hbox, false, false, 0);

  /*-- Exclude button --*/
  btn = gtk_button_new_with_mnemonic ("E_xclude shadows");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Exclude all points in shadow, so that they're not drawn and they're ignored when scaling the view.",
                        NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (exclude_hiddens_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, true, 0);

  /*-- Include button --*/
  btn = gtk_button_new_with_mnemonic ("_Include shadows");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Include all previously hidden and excluded points.",
                        NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (include_hiddens_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, true, 0);

  /*-- Update button --*/
  btn = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                        "Reset plots after brushing so that shadow and excluded status is consistent with this table; reset this table if necessary.",
                        NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
                    G_CALLBACK (update_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, true, 0);

  /*-- Close button --*/
  g_signal_connect (dialog, "response",
                    G_CALLBACK (close_btn_cb), (gpointer) gg);
  
  gtk_widget_show_all (gg->cluster_ui.window);

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    /*-- this doesn't track cluster counts, just cluster identities --*/
    g_signal_emit (G_OBJECT (gg), GGobiSignals[CLUSTERS_CHANGED_SIGNAL], 0,
                   d);
  }

  gdk_window_raise (gg->cluster_ui.window->window);
}
