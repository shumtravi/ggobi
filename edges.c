/* edges.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* --------------------------------------------------------------- */
/*                   Dynamic allocation section                    */
/* --------------------------------------------------------------- */

/*
 * The brushing arrays are handled in brush_init.c, and the
 * edges and arrowheads arrays are handled in splot.c
*/

void edges_alloc(gint nsegs, datad * d)
{
  d->edge.n = nsegs;
  d->edge.sym_endpoints = (SymbolicEndpoints*)
     g_realloc(d->edge.sym_endpoints, nsegs * sizeof(SymbolicEndpoints));

  vectorb_alloc(&d->edge.xed_by_brush, nsegs);
}

void edges_free(datad * d, ggobid * gg)
{
  gpointer ptr;

  vectorb_free(&d->edge.xed_by_brush);
  ptr = (gpointer) d->edge.sym_endpoints;
  g_free(ptr);
  d->edge.n = 0;
}

/* --------------------------------------------------------------- */
/*               Add and delete edges                              */
/* --------------------------------------------------------------- */

/**
  Allocate space for another edge observation and set the
  locations of the rows.
 */
gboolean edge_add (gint a, gint b, gchar *lbl, gchar *id, datad * d, datad * e,
  ggobid *gg)
{
  gchar *s1, *s2;
  gint n = e->edge.n;
  GList *l, *sl;
  splotd *sp;
  displayd *dsp;

/*-- while the code is evolving ... --*/
g_printerr ("lbl %s id %s\n", lbl, id);

  g_assert (e->edge.n == e->nrows);

  /*-- eventually check whether a->b already exists before adding --*/

  /*-- Here's what the datad needs --*/
/*
 * some of this can be encapsulated as datad_record_add, as long
 * as problems with the sequence of operations don't arise.
*/
  e->nrows += 1;

  /*-- add a row label --*/
  if (!lbl) s1 = g_strdup_printf ("%d", n+1);
  rowlabel_add ((lbl)?lbl:s1, e);  /*-- don't free s1 --*/

  /*-- if necessary, add an id --*/
  if (e->idTable) {
    if (!id) s2 = g_strdup_printf ("%d", n+1);
    datad_record_id_add (s2, e);  /*-- don't free s2 --*/
  }

  pipeline_arrays_check_dimensions (e);
  rows_in_plot_set (e, gg);

  /*-- allocate and initialize brushing arrays --*/
  br_glyph_ids_add (e, gg);
  br_color_ids_add (e, gg);
  br_hidden_alloc (e);
  vectorb_realloc (&e->pts_under_brush, e->nrows);
  clusters_set (e, gg);

  if (e->nmissing)
    arrays_add_rows (&e->missing, e->nrows);

  edges_alloc(e->nrows, e);
  e->edge.sym_endpoints[n].a = g_strdup (d->rowIds[a]);
  e->edge.sym_endpoints[n].b = g_strdup (d->rowIds[b]);
  e->edge.sym_endpoints[n].jpartner = -1; /* XXX */
  unresolveAllEdgePoints(e);
  resolveEdgePoints (e, d);

/*
DTL: So need to call unresolveEdgePoints(e, d) to remove it from the 
     list of previously resolved entries.
     Can do better by just re-allocing the endpoints in the
     DatadEndpoints struct and putting the new entry into that,
     except we have to check it resolves correctly, etc. So
     unresolveEdgePoints() will just cause entire collection to be
     recomputed.
*/

/*
 * This will be handled with signals, where each splotd listens
 * for (maybe) point_added or edge_added events.
*/

  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->e == e) {
      for (sl = dsp->splots; sl; sl = sl->next) {
        sp = (splotd *) sl->data;
        if (sp != NULL) {
          splot_edges_realloc (n, sp, e);
          /*-- this is only necessary if there are variables, I think --*/
          if (e->ncols && GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
            GtkGGobiExtendedSPlotClass *klass;
            klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
            if(klass->alloc_whiskers)
              sp->whiskers = klass->alloc_whiskers(sp->whiskers, sp,
                e->nrows, e);
          }
        }
      }
    }
  }
  /*-- I need to reallocate whiskers for some displays --*/

  displays_tailpipe (FULL, gg);

  /*-- I don't yet know what I need to reallocate for the tour --*/

  return true;
}


/* --------------------------------------------------------------- */
/*               Add an edgeset                                    */
/* --------------------------------------------------------------- */


/**
  This sets the data set as the source of the edge information
  for all the plots within the display.
 */
datad *setDisplayEdge(displayd * dpy, datad * e)
{
  GList *l;
  datad *old = NULL;

  if(resolveEdgePoints(e, dpy->d)) {
    dpy->e = e;
     /*XXX  This needs to be more general, working on all displays.
            Need to emit an event and have the displays respond to it by
            checking whether the edgeset_add returns true. */
    scatterplot_display_edge_menu_update(dpy, e->gg->app.sp_accel_group,
      display_options_cb, e->gg);
  }

  for (l = dpy->splots; l; l = l->next) {
    splotd *sp;
    sp = (splotd *) l->data;
    splot_edges_realloc (-1, sp, e);
  }
  return (old);
}

/**
  This looks for the first dataset that can be used
  as a source of edge information with the point dataset
  associated with the given display.

  @see setDisplayEdge.
 */
gboolean edgeset_add(displayd * display)
{
  datad *d = display->d;
  datad *e;
  gint k;
  gboolean added = false;
  ggobid *gg = GGobiFromDisplay(display);

  if (gg->d != NULL) {
    gint nd = g_slist_length(gg->d);

    if (d->idTable) {
      for (k = 0; k < nd; k++) {
        e = (datad *) g_slist_nth_data(gg->d, k);
        if (/* e != d && */ e->edge.n > 0) {
          setDisplayEdge(display, e);
          added = true;
          break;
        }
      }
    }
  }

  return added;
}

/**
 Invoked when the user selected an item in the Edges menu 
 on a scatterplot to control whether edges are displayed or not
 on the plot.
 */
void edgeset_add_cb(GtkWidget * w, datad * e)
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = (displayd *) gtk_object_get_data(GTK_OBJECT(w),
                                                       "display");
  setDisplayEdge(display, e);

  display_plot(display, FULL, gg);   /*- moving edge drawing */

  /*
   * If no edge option is true, then turn on undirected edges.
  */
  if (!display->options.edges_undirected_show_p &&
      !display->options.edges_directed_show_p &&
      !display->options.edges_arrowheads_show_p)
  {
    GtkWidget *ww = widget_find_by_name(display->edge_menu,
      "DISPLAYMENU:edges_u");
    if (ww) {
      gtk_check_menu_item_set_active((GtkCheckMenuItem *) ww, true);
    }
  }
}

#if 0
void
GGobi_cleanUpEdgeRelationships(struct _EdgeData *edge, int startPosition)
{
  int k, i, start, end;
  for (i = startPosition; i < edge->n; i++) {
    end = edge->endpoints[i].b;
    start = edge->endpoints[i].a;
    for (k = 0; k < i; k++) {
      if (edge->endpoints[k].a == end && edge->endpoints[k].b == start) {
        edge->endpoints[i].jpartner = k;
        edge->endpoints[k].jpartner = i;
      }
    }
  }
}
#endif


/* --------------------------------------------------------------- */
/*                          Utilities                              */
/* --------------------------------------------------------------- */

gboolean
edge_endpoints_get (gint k, gint *a, gint *b, datad *d, endpointsd *endpoints, datad *e)
{
  gboolean ok;

  *a = endpoints[k].a;
  *b = endpoints[k].b;

  ok = (*a >= 0 && *a < d->nrows && *b >= 0 && *b < d->nrows);

  return ok;
}

/*****************************************************************************************/

/* A constant used to identify that we have resolved the edgepoints and there were none. */
static endpointsd DegenerateEndpoints;


/*
  Do the computations to lookup the different source and destination
  records in the target dataset (d) to get the record numbers
  given the symbolic names in the edgeset specification (sym).
*/
static endpointsd *
computeResolvedEdgePoints(datad *e, datad *d)
{
  endpointsd *ans;
  GHashTable *tbl = d->idTable;
  int ctr = 0, i;
  guint *tmp;
  gboolean resolved_p = false;

  ans = g_malloc( sizeof(endpointsd) * e->edge.n);
 
  for(i = 0; i < e->edge.n; i++, ctr++) {
    tmp = (guint *) g_hash_table_lookup(tbl, e->edge.sym_endpoints[i].a);
    if(!tmp) {
      ans[ctr].a = -1;
      continue;
    }

    ans[ctr].a = *tmp;

    tmp = (guint *) g_hash_table_lookup(tbl, e->edge.sym_endpoints[i].b);
    if(!tmp) {
      ans[ctr].a = ans[ctr].b = -1;
      continue;
    } else {
      ans[ctr].b = *tmp;
      ans[ctr].jpartner = e->edge.sym_endpoints[i].jpartner;
      if (!resolved_p && ans[ctr].a != -1)
        resolved_p = true;
    }
  }

  if(ctr == 0 || resolved_p == false) {
    g_free(ans);
    ans = &DegenerateEndpoints;
  }

  return(ans);
}



static endpointsd *
do_resolveEdgePoints(datad *e, datad *d, gboolean compute)
{
  endpointsd *ans = NULL;
  DatadEndpoints *ptr;
  GList *tmp;


  if(e->edge.n < 1)
    return(NULL);

  /* Get the entry in the table for this dataset (d). Use the name for now. */
  for(tmp = e->edge.endpointList; tmp ; tmp = tmp->next) {
     ptr = (DatadEndpoints *) tmp->data;
     if(ptr->data == d) {
       ans = ptr->endpoints;
       break;
     }
  }

  /* If it is already computed but empty, then return NULL. */
  if(ans == &DegenerateEndpoints)
     return(NULL);

  /* So no entry in the table yet. So compute the endpoints and add that to the table. */
  if(ans == NULL && compute) {
     /* resolve the endpoints */
/*
g_printerr ("resolving %s on %s (%d)\n", e->name, d->name, compute);
g_printerr ("   creating a new match\n");
*/
    ans = computeResolvedEdgePoints(e, d);
    ptr = (DatadEndpoints *) g_malloc(sizeof(DatadEndpoints));
    ptr->data = d;
    ptr->endpoints = ans; /* (ans == &DegenerateEndpoints) ? NULL : ans; */
/*
g_printerr ("   %d %d\n", ptr->endpoints[0].a, ptr->endpoints[0].b);
*/
    e->edge.endpointList = g_list_append(e->edge.endpointList, ptr);
  }

  if(ans == &DegenerateEndpoints)
     return(NULL);

  return(ans);
}


/*
 Find the resolved edgepoints array associated with
 this dataset (d) by resolving the symbolic edge definitions
 in e.
*/
endpointsd *
resolveEdgePoints(datad *e, datad *d)
{
  return(do_resolveEdgePoints(e, d, true));
}


gboolean
hasEdgePoints(datad *e, datad *d)
{
  return(do_resolveEdgePoints(e, d, false) ? true : false);
}

static void
cleanEdgePoint(gpointer data, gpointer userData)
{
   DatadEndpoints *el = (DatadEndpoints *)data;
   if(el && el->endpoints) {
     g_free(el->endpoints);
   }
}

void
unresolveAllEdgePoints(datad *e)
{
  if(e->edge.endpointList) {
    g_list_foreach(e->edge.endpointList, cleanEdgePoint, NULL);
    g_list_free(e->edge.endpointList);
    e->edge.endpointList = NULL;
  }
}


gboolean
unresolveEdgePoints(datad *e, datad *d)
{
  DatadEndpoints *ptr;
  GList *tmp;

  if(e->edge.n < 1)
    return(false);

  for(tmp = e->edge.endpointList; tmp ; tmp = tmp->next) {
     ptr = (DatadEndpoints *) tmp->data;
     if(ptr->data == d) {
       if(ptr->endpoints)
          g_free(ptr->endpoints);

          /* equivalent to 
               g_list_remove(e->edge.endpointList, tmp) 
             except we don't do the extra looping. Probably
             minute since # of datasets is small!
           */
       if(tmp == e->edge.endpointList) {
          e->edge.endpointList = tmp->next;
       } else {
          tmp->prev = tmp->next;
       }
       return(true);
     }
  }

  return(false);
}
