/* brush_link.c */
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

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                Linking to other datad's by id                        */
/*----------------------------------------------------------------------*/

gboolean
symbol_link_by_id (gboolean persistentp, gint k, GGobiData * sd, ggobid * gg)
{
  GGobiData *d;
  GSList *l;
  gint i, id = -1;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/

  id = ggobi_data_get_row_for_id(sd, sd->rowIds[k]);
  if (id < 0)
    return false;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/

    i = ggobi_data_get_row_for_id(d, sd->rowIds[id]);

    if (i < 0)              /*-- then no cases in d have this id --*/
      continue;

    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (d->sampled.els[i] && !d->excluded.els[i]) {
      if (persistentp || cpanel->br.mode == BR_PERSISTENT) {

        /*
         * make it link for everything, no matter
         * what kind of brushing is turned on, because
         * otherwise, connections between points and edges
         * gets messed up.
         */

        if (!d->hidden_now.els[i]) {
          d->color.els[i] = d->color_now.els[i] = sd->color.els[k];
          d->glyph.els[i].size = d->glyph_now.els[i].size =
            sd->glyph.els[k].size;
          d->glyph.els[i].type = d->glyph_now.els[i].type =
            sd->glyph.els[k].type;
        }
        d->hidden.els[i] = d->hidden_now.els[i] = sd->hidden.els[k];

        /*-- should we handle this here?  --*/
        //d->excluded.els[i] = sd->excluded.els[k];

      }
      else if (cpanel->br.mode == BR_TRANSIENT) {

        if (!d->hidden_now.els[i]) {
          d->color_now.els[i] = sd->color_now.els[k];
          d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
          d->glyph_now.els[i].type = sd->glyph_now.els[k].type;
        }
        d->hidden_now.els[i] = sd->hidden_now.els[k];
      }
    }
  }
  return changed;
}

gboolean
exclude_link_by_id (guint k, GGobiData * sd, ggobid * gg)
{
/*-- sd = source_d --*/
  GGobiData *d;
  GSList *l;
  gint i, id = -1;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/
  id = ggobi_data_get_row_for_id(sd, sd->rowIds[k]);
  if (id < 0)
    return false;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/

    i = ggobi_data_get_row_for_id(d, sd->rowIds[id]);

    if (i < 0)              /*-- then no cases in d have this id --*/
      continue;

    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (d->sampled.els[i])
      d->excluded.els[i] = sd->excluded.els[k];
  }
  return changed;
}

/*----------------------------------------------------------------------*/
/*   Linking within and between datad's using a categorical variable    */
/*----------------------------------------------------------------------*/

void
brush_link_by_var (gint jlinkby, vector_b * levelv,
                   cpaneld * cpanel, GGobiData * d, ggobid * gg)
{
  gint m, i, level_value;

  /*
   * for this datad, loop once over all rows in plot 
   */
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];

    level_value = ggobi_data_get_integer_value(d, i, jlinkby);

    if (levelv->els[level_value]) {  /*-- if it's to acquire the new symbol --*/
      if (cpanel->br.mode == BR_PERSISTENT) {
        switch (cpanel->br.point_targets) {
        case br_candg:   /*-- color and glyph, type and size --*/
          d->color.els[i] = d->color_now.els[i] = gg->color_id;
          d->glyph.els[i].size = d->glyph_now.els[i].size = gg->glyph_id.size;
          d->glyph.els[i].type = d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
        case br_color:   /*-- color only --*/
          d->color.els[i] = d->color_now.els[i] = gg->color_id;
          break;
        case br_glyph:   /*-- glyph type and size --*/
          d->glyph.els[i].size = d->glyph_now.els[i].size = gg->glyph_id.size;
          d->glyph.els[i].type = d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
        case br_shadow:   /*-- shadowed, hidden --*/
          d->hidden.els[i] = d->hidden_now.els[i] = true;
          break;
          /*
             case br_unshadow:
             d->hidden.els[i] = d->hidden_now.els[i] = false;
             break;
           */
        default:
          break;
        }

      }
      else if (cpanel->br.mode == BR_TRANSIENT) {
        switch (cpanel->br.point_targets) {
        case br_candg:
          d->color_now.els[i] = gg->color_id;
          d->glyph_now.els[i].size = gg->glyph_id.size;
          d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
        case br_color:
          d->color_now.els[i] = gg->color_id;
          break;
        case br_glyph:   /*-- glyph type and size --*/
          d->glyph_now.els[i].size = gg->glyph_id.size;
          d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
        case br_shadow:   /*-- hidden --*/
          d->hidden_now.els[i] = true;
          break;
          /*
             case br_unshadow:
             d->hidden_now.els[i] = false;
             break;
           */
        default:
          break;
        }
      }

    }
    else {    /*-- if it's to revert to the previous symbol --*/
      /*-- should only matter if transient, right? --*/
      switch (cpanel->br.point_targets) {
      case br_candg:
        d->color_now.els[i] = d->color.els[i];
        d->glyph_now.els[i].size = d->glyph.els[i].size;
        d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
      case br_color:
        d->color_now.els[i] = d->color.els[i];
        break;
      case br_glyph:   /*-- glyph type and size --*/
        d->glyph_now.els[i].size = d->glyph.els[i].size;
        d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
      case br_shadow:   /*-- hidden --*/
        d->hidden_now.els[i] = d->hidden.els[i];
        break;
        /* disabled
           case br_unshadow:
           d->hidden_now.els[i] = d->hidden.els[i];
           break;
         */
      default:
        break;
      }
    }
  }
}

/*
 * We're working too hard here, looping whether there's any
 * change or not.  Maybe there's an easy way to set the value
 * of changed by keeping track of pts_under_brush_prev?
*/
gboolean
build_symbol_vectors_by_var (cpaneld * cpanel, GGobiData * d, ggobid * gg)
{
  gint i, m, j, level_value, level_value_max;
  vector_b levelv;
  GSList *l;

  if (!d->linkvar)
    return false;

  j = ggobi_data_get_col_index_for_name(d, d->linkvar);
  level_value_max = (gint) ggobi_data_get_col_max(d, j);

  vectorb_init_null (&levelv);
  vectorb_alloc (&levelv, level_value_max + 1);
  vectorb_zero (&levelv);

  /*-- find the levels which are among the points under the brush --*/
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    if (d->pts_under_brush.els[i]) {
      level_value = ggobi_data_get_integer_value(d, i, j);
      levelv.els[level_value] = true;
    }
  }

  /*-- first do this d --*/
  brush_link_by_var (j, &levelv, cpanel, d, gg);

  /*-- now for the rest of them --*/
  for (l = gg->d; l; l = l->next) {
    GGobiData *dd = l->data;
    if (dd == d) continue;

    j = ggobi_data_get_col_index_for_name(dd, d->linkvar);
    if (j != -1) {
      brush_link_by_var (j, &levelv, cpanel, dd, gg);
    }
  }

  vectorb_free (&levelv);

  return (true);
}


/*********************************************************************/
/*          Create a variable notebook for brush linking rule        */
/*********************************************************************/

enum
{ LINKBYLIST_NAME, LINKBYLIST_NCOLS };

void linkby_notebook_subwindow_add (GGobiData * d, GtkWidget * notebook,
                                    ggobid *);

void
varlist_append (GtkListStore * list, GGobiData *d, gchar* name)
{
  GtkTreeIter iter;
  guint j = ggobi_data_get_col_index_for_name(d, name);
  
  if(ggobi_data_get_col_type(d, j) != categorical)
    return;

  gtk_list_store_append (list, &iter);
  gtk_list_store_set (list, &iter, LINKBYLIST_NAME, name, -1);
}

void
varlist_populate (GtkListStore * list, GGobiData * d)
{
  gint j;
  GtkTreeIter first;

  gtk_list_store_append (list, &first);
  gtk_list_store_set (list, &first, LINKBYLIST_NAME, "<i>Case ID</i>", -1);

  for (j = 0; j < d->ncols; j++) {
    varlist_append(list, d, ggobi_data_get_col_name(d, j));
  }
}

/* In this routine, we could show two pages if appropriate, one
 * for points and one for edges. */
/* called from cpanel_brush_set */
void
linkby_current_page_set (displayd * display, GtkWidget * notebook,
                         ggobid * gg)
{
  GtkWidget *swin, *treeview;
  GGobiData *d = display->d, *paged;
  gint page_num;
  cpaneld *cpanel = &display->cpanel;
  GList *children;

  if (notebook == NULL) {
    return;
  }

  /*
   * For each page of the notebook, get its child, the scrolled
   * window.  Get the datad that the scrolled window knows about,
   * and compare it with display->d
   */

  page_num = 0;
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
  while (swin) {
    paged = (GGobiData *) g_object_get_data (G_OBJECT (swin), "datad");

    gtk_widget_set_sensitive (swin, (paged == d));
    if (paged == d) {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);

      children = gtk_container_get_children (GTK_CONTAINER (swin));
      treeview = children->data;
      /* Set the selected row of the selected page */
      select_tree_view_row (treeview, cpanel->br.linkby_row);
      gg->linkby_cv = (cpanel->br.linkby_row > 0);
      break;
    }
    page_num += 1;
    swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
  }
}

void
linking_method_set_cb (GtkTreeSelection * treesel, ggobid * gg)
{
  GGobiData *d =
    g_object_get_data (G_OBJECT (gtk_tree_selection_get_tree_view (treesel)),
                       "datad");
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path;
  gint row = -1;

  if (gtk_tree_selection_get_selected (treesel, &model, &iter)) {
    path = gtk_tree_model_get_path (model, &iter);
    row = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);
  }

  /* Before  touching the  cpanel, make  sure the  display  and cpanel
   * actually correspond to the current data.  This looks more and
   * more like a kludge -- I do need to revisit this.  -- dfs
   */
  if (gg->current_display->d == d) {
    displayd *display = gg->current_display;
    cpaneld *cpanel = &display->cpanel;
    cpanel->br.linkby_row = row;
  }

  if (row <= 0) {
    gg->linkby_cv = false;
    d->linkvar = NULL;
    return;                     /* link by case id; done */
  }
  else {
    gchar* name;
    gtk_tree_model_get (model, &iter, LINKBYLIST_NAME, &name, -1);
    gg->linkby_cv = true;
    d->linkvar = name;
  }
}

GtkListStore *
list_from_data (ggobid * gg, GGobiData * data, GtkNotebook * notebook)
{
  GtkWidget *swin;
  GtkListStore *list = NULL;

  gint kd = g_slist_index (gg->d, data);
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), kd);

  if (swin)
    list =
      GTK_LIST_STORE (gtk_tree_view_get_model
                      (GTK_TREE_VIEW (GTK_BIN (swin)->child)));

  return (list);
}

void
linkby_notebook_varchanged_cb (ggobid * gg, GGobiData * data,
                               GtkNotebook * notebook)
{
  GtkListStore *list = list_from_data (gg, data, notebook);
  gtk_list_store_clear (list);
  varlist_populate (list, data);
}

void
linkby_notebook_varadded_cb (ggobid * gg, gint which,
                             GGobiData * d, GtkNotebook * notebook)
{
  GtkListStore *model = list_from_data (gg, d, notebook);
  
  if (model)
    varlist_append (model, d, ggobi_data_get_col_name(d, which));
}

void
linkby_notebook_list_changed_cb (ggobid * gg, GGobiData * d, void *notebook)
{
  linkby_notebook_varchanged_cb (gg, d, notebook);
}

CHECK_EVENT_SIGNATURE (linkby_notebook_adddata_cb, datad_added_f)
  CHECK_EVENT_SIGNATURE (linkby_notebook_varchange_cb, variable_added_f)
  CHECK_EVENT_SIGNATURE (linkby_notebook_list_changed_cb,
                       variable_list_changed_f)

static void
linkby_notebook_adddata_cb (ggobid * gg, GGobiData * d, void *notebook,
                                   GtkSignalFunc func)
{
  linkby_notebook_subwindow_add (d, notebook, gg);
}

void
linkby_notebook_subwindow_add (GGobiData * d, GtkWidget * notebook,
                               ggobid * gg)
{
  GtkWidget *swin, *treeview;
  GtkListStore *list;

  GtkSelectionMode mode = GTK_SELECTION_SINGLE;

  if (!ggobi_data_has_cols(d))
    return;

  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                       GTK_SHADOW_NONE);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /* If this is not the first child of the notebook, initialize it
   * as insensitive.
   */

  if (g_list_length (gtk_container_get_children (GTK_CONTAINER (notebook))) !=
      0) {
    gtk_widget_set_sensitive (swin, false);
  }
  g_object_set_data (G_OBJECT (swin), "datad", d);  /*setdata */
/*
 * name or nickname?  Which one we'd prefer to use depends on the
 * size of the space we're working in -- maybe this will become an
 * argument.
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin,
                            (d->nickname != NULL) ?
                            gtk_label_new (d->nickname) : gtk_label_new (d->
                                                                         name));

  /* add the treeview (list) */
  list = gtk_list_store_new (LINKBYLIST_NCOLS, G_TYPE_STRING, G_TYPE_POINTER);
  varlist_populate (list, d);

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list));
  populate_tree_view (treeview, NULL, 1, false, mode,
                      G_CALLBACK (linking_method_set_cb), gg);

  g_object_set_data (G_OBJECT (treeview), "datad", d);
  //g_object_set_data (G_OBJECT (clist), "notebook", notebook);

  gtk_container_add (GTK_CONTAINER (swin), treeview);
  gtk_widget_show_all (swin);

  select_tree_view_row (treeview, 0);
}

GtkWidget *
create_linkby_notebook (GtkWidget * box, ggobid * gg)
{
  GtkWidget *notebook;
  GSList *l;
  GGobiData *d;

  GtkSelectionMode mode = GTK_SELECTION_SINGLE;
  vartyped vtype = categorical;
  datatyped dtype = all_datatypes;

  /* Create a notebook, set the position of the tabs */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  //gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), nd > 1);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), false);
  gtk_box_pack_start (GTK_BOX (box), notebook, true, true, 2);
  g_object_set_data (G_OBJECT (notebook), "SELECTION", (gpointer) mode);
  g_object_set_data (G_OBJECT (notebook), "vartype", (gpointer) vtype);
  g_object_set_data (G_OBJECT (notebook), "datatype", (gpointer) dtype);

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (ggobi_data_has_cols(d)) {
      linkby_notebook_subwindow_add (d, notebook, gg);
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*--   ... list_changed would be enough but it's only called on delete --*/
  g_signal_connect (G_OBJECT (gg),
                    "variable_added",
                    G_CALLBACK (linkby_notebook_varadded_cb),
                    GTK_OBJECT (notebook));
  g_signal_connect (G_OBJECT (gg),
                    "variable_list_changed",
                    G_CALLBACK (linkby_notebook_varchanged_cb),
                    GTK_OBJECT (notebook));

  /*-- listen for datad_added events on main_window --*/
  g_signal_connect (G_OBJECT (gg),
                    "datad_added",
                    G_CALLBACK (linkby_notebook_adddata_cb),
                    GTK_OBJECT (notebook));

  return notebook;
}
