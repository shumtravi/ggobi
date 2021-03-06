/*-- make_ggobi.c --*/
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

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "gui-impute.h"
#include "gui-jitter.h"
#include "gui-randomize.h"
#include "gui-subset.h"
#include "gui-viewer.h"
#include "stage-subset.h"
#include "stage-display.h"
#include "stage-impute.h"
#include "stage-jitter.h"
#include "stage-randomize.h"
#include "stage-standardize.h"
#include "stage-transform.h"

#include "plugin-old.h"

#ifdef USE_MYSQL
#include "read_mysql.h"
#endif

guint GGobiSignals[MAX_GGOBI_SIGNALS];

static void
pipeline_create_cb(GGobiPipelineFactory *factory, GGobiStage *root, GGobiSession *gg)
{
  GObject *display, *domain_adj, *filter, *impute, *jitter, *randomize, *standardize, *subset, *transform;
  
  subset = g_object_new(GGOBI_TYPE_STAGE_SUBSET, 
    "name", GGOBI_MAIN_STAGE_SUBSET, "parent", root, NULL);

  /* Note: there is no way to control the order of property settings with
     g_object_new, so we have to set the filter col here so that it
     comes after the parent */
  ggobi_stage_filter_set_filter_col(GGOBI_STAGE_FILTER(subset), 
    ggobi_stage_get_col_index_for_name(root, "_sampled"));

  impute = g_object_new(GGOBI_TYPE_STAGE_IMPUTE, 
    "name", GGOBI_MAIN_STAGE_IMPUTE, "parent", subset, NULL);

  jitter = g_object_new(GGOBI_TYPE_STAGE_JITTER, 
    "name", GGOBI_MAIN_STAGE_JITTER, "parent", impute, NULL);
  
  randomize = g_object_new(GGOBI_TYPE_STAGE_RANDOMIZE, 
    "name", GGOBI_MAIN_STAGE_RANDOMIZE, "parent", jitter, NULL);
  
  filter = g_object_new(GGOBI_TYPE_STAGE_FILTER, 
    "name", GGOBI_MAIN_STAGE_FILTER, "parent", randomize, NULL);
  
  // FIXME: 'excluded' is actually 'included' now
  ggobi_stage_filter_set_filter_col(GGOBI_STAGE_FILTER(filter),
    ggobi_stage_get_col_index_for_name(root, "_excluded"));
  
  domain_adj = g_object_new(GGOBI_TYPE_STAGE_TRANSFORM,
    "name", GGOBI_MAIN_STAGE_DOMAIN_ADJ, "parent", filter, NULL);
  transform = g_object_new(GGOBI_TYPE_STAGE_TRANSFORM,
    "name", GGOBI_MAIN_STAGE_TRANSFORM, "parent", domain_adj, NULL);
  
  standardize = g_object_new(GGOBI_TYPE_STAGE_STANDARDIZE, 
    "name", GGOBI_MAIN_STAGE_STANDARDIZE, "parent", transform, NULL);
  
  display = g_object_new(GGOBI_TYPE_STAGE_DISPLAY, 
    "name", GGOBI_MAIN_STAGE_DISPLAY, "parent", standardize, NULL);
  
  
  // FIXME: get rid of these lines ASAP
  // There is absolutely no reason for the pipeline to depend on GGobiSession
  GGOBI_STAGE(display)->gg = gg;
  GGOBI_STAGE(domain_adj)->gg = gg;
  GGOBI_STAGE(filter)->gg = gg;
  GGOBI_STAGE(impute)->gg = gg;
  GGOBI_STAGE(jitter)->gg = gg;
  GGOBI_STAGE(randomize)->gg = gg;
  GGOBI_STAGE(standardize)->gg = gg;
  GGOBI_STAGE(subset)->gg = gg;
  GGOBI_STAGE(transform)->gg = gg;
  
  g_object_unref(display);
  g_object_unref(domain_adj);
  g_object_unref(filter);
  g_object_unref(impute);
  g_object_unref(jitter);
  g_object_unref(randomize);
  g_object_unref(standardize);
  g_object_unref(subset);
  g_object_unref(transform);
    
  GGobiGuiViewer *viewer; 
  viewer = g_object_new(GGOBI_TYPE_GUI_VIEWER, "stage", GGOBI_STAGE(display), NULL);
  gtk_widget_show(GTK_WIDGET(viewer));
  /*
  GGobiGuiSubset *gui_subset;
  gui_subset = g_object_new(GGOBI_TYPE_GUI_SUBSET, "stage", GGOBI_STAGE_SUBSET(subset), NULL);
  gtk_widget_show(GTK_WIDGET(gui_subset));
  
  GGobiGuiRandomize *gui_randomize;
  gui_randomize = g_object_new(GGOBI_TYPE_GUI_RANDOMIZE, "stage", GGOBI_STAGE_RANDOMIZE(randomize), NULL);
  gtk_widget_show(GTK_WIDGET(gui_randomize));


  GGobiGuiJitter *gui_jitter;
  gui_jitter = g_object_new(GGOBI_TYPE_GUI_JITTER, "stage", GGOBI_STAGE_JITTER(jitter), NULL);
  gtk_widget_show(GTK_WIDGET(gui_jitter));

  GGobiGuiImpute *gui_impute;
  gui_impute = g_object_new(GGOBI_TYPE_GUI_IMPUTE, "stage", GGOBI_STAGE_IMPUTE(impute), NULL);
  gtk_widget_show(GTK_WIDGET(gui_impute));
  */
}

GGobiPipelineFactory *
ggobi_create_pipeline_factory(GGobiSession *gg)
{
  GObject *factory = (GObject *)ggobi_pipeline_factory_new();
  g_signal_connect(factory, "build", G_CALLBACK(pipeline_create_cb), gg);
  return(GGOBI_PIPELINE_FACTORY(factory));
}

/*-- initialize variables which don't depend on the size of the data --*/
void
globals_init (GGobiSession * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  gg->close_pending = false;

  gg->glyph_id.type = gg->glyph_0.type = FC;
  gg->glyph_id.size = gg->glyph_0.size = 1;
  gg->color_0 = 0;
  gg->color_id = scheme->n - 1; /* default: initialize to last color */
  /* can be overriden in xml file */

  /*-- for linking by categorical variable --*/
  gg->linkby_cv = false;

  gg->lims_use_visible = true;
  gg->buttondown = 0;  /*-- no button is pressed --*/

  gg->d = NULL;

  gg->statusbar_p = true;
  
  gg->pipeline_factory = ggobi_create_pipeline_factory(gg);
}


GSList *
load_data (const gchar * uri, GGobiSession * gg)
{
  GFile *file = create_file(uri);
  GSList *ds = NULL;
  if (file) { // FIXME: report this error in gg->io_context
    ds = load_data_source(file, gg);
    g_object_unref(G_OBJECT(file));
  }
  return (ds);
}

// returns a list of datasets (some input types (eg. xml) may return 
// multiple data types)
GSList *
load_data_source (GFile *file, GGobiSession * gg)
{
  GGobiDataFactory *factory;
  GSList *datasets = NULL;
  
  factory = create_data_factory(gg, file);
  if (factory == NULL) {
    // FIXME: we should have some unified way of graphically reporting errors
    // from some sort of IO context
    g_critical("No data factory capable of parsing the data");
    return NULL;
  }
  
  datasets = ggobi_data_factory_create(factory, file);
  for (; datasets; datasets = datasets->next) {
    GGobiStage *dataset = GGOBI_STAGE(datasets->data);
    /* hack: if there are no variables in the dataset (ie an edge set)
       at least make sure there are attributes.
       This will go away once we move to just storing attributes as variables */
    if (!dataset->n_cols)
      ggobi_data_add_attributes (GGOBI_DATA (dataset));
      g_signal_emit_by_name (G_OBJECT (gg->pipeline_factory), "build", (gpointer) dataset);
    /* eventually ggobi_stage_attach will happen implicitly when the 
       dataset is added to the main context. Right now we are sort of hacking
       it by attaching the transform stage rather than the dataset. The _attach()
       method knows when to go back to the root. */
    ggobi_stage_attach(ggobi_stage_find(dataset, GGOBI_MAIN_STAGE_DISPLAY), gg, FALSE);
  }
  
  return (datasets);
}

GGobiDataFactory *
create_data_factory (GGobiSession *gg, GFile *file)
{
  GGobiDataFactory *factory = NULL;
  GSList *factories = gg->data_factories;
  for (; factories && !factory; factories = factories->next) {
    if (ggobi_data_factory_supports_file(GGOBI_DATA_FACTORY(factories->data), 
                                         file, NULL))
      factory = GGOBI_DATA_FACTORY(factories->data);
  }
  return (factory);
}

#include <libxml/uri.h>

static gint
scheme_compare_func(gconstpointer list_scheme, gconstpointer scheme)
{
  return (list_scheme || scheme) && (!list_scheme || !scheme ||
    g_ascii_strcasecmp(list_scheme, scheme));
}

gchar *
get_file_description(GFile *file)
{
  GFileInfo *info = g_file_query_info(file,
                                      G_FILE_ATTRIBUTE_STANDARD_DESCRIPTION,
                                      G_FILE_QUERY_INFO_NONE, NULL, NULL);
  gchar *desc = 
    g_file_info_get_attribute_as_string(info,
                                        G_FILE_ATTRIBUTE_STANDARD_DESCRIPTION);
  g_object_unref(G_OBJECT(info));
  return desc;
}

GFile *
create_file(const gchar *uri)
{
  return g_file_new_for_uri(uri);
}

static void
register_default_data_factories(GGobiSession *gg)
{
  GObject *factory = g_object_new(GGOBI_TYPE_DATA_FACTORY_XML, NULL);
  ggobi_session_register_data_factory(gg, GGOBI_DATA_FACTORY(factory));
  g_object_unref(factory);
  factory = g_object_new(GGOBI_TYPE_DATA_FACTORY_CSV, NULL);
  ggobi_session_register_data_factory(gg, GGOBI_DATA_FACTORY(factory));
  g_object_unref(factory); 
}

/*
 * the first display is initialized in datad_attach, so turn on
 * event handlers there as well
*/
void
make_ggobi (GGobiOptions * options, gboolean processEvents, GGobiSession * gg)
{
  gboolean init_data = false;

  /*-- some initializations --*/
  gg->displays = NULL;

  globals_init (gg); /*-- variables that don't depend on the data --*/

  special_colors_init (gg);

  wvis_init (gg);
  svis_init (gg);
  make_ui (gg);

  register_default_data_factories(gg);
  
  /* If the user specified a data file on the command line, then 
     try to load that.
   */
  if (options->data_in || options->data_type) {
    if (load_data (options->data_in, gg)) {
      init_data = true;
    }
  }

  if (options->info != NULL)
    registerPlugins (gg, options->info->plugins);
  
  if (options->timingp) {
    // Initialize the time counter
    set_time(gg);
  }

  start_ggobi (gg, init_data, options->info->createInitialScatterPlot);

  if (options->restoreFile) {
    processRestoreFile (options->restoreFile, gg);
  }

  gg->status_message_func = gg_write_to_statusbar;

  if (options->timingp) {
    run_timing_tests (gg);
  }

  if (processEvents) {
    gtk_main ();
  }
}

// FIXME: when this refactored, we need to emit a signal on GGobi "start"
// to allow stuff like batch execution via plugins
void
start_ggobi (GGobiSession * gg, gboolean init_data, gboolean createPlot)
{
  GGobiStage *d;
  if (init_data) {
    GSList *l;
    gboolean firstd = createPlot;
    for (l = gg->d; l; l = l->next) {
      ggobi_stage_attach(l->data, gg, firstd);
      firstd = false;
    }

    /*-- destroy and rebuild the menu every time data is read in --*/
    display_menu_build (gg);
  }

  /*-- now that we've read some data, set the mode --*/
  if (createPlot && gg->d) {
    d = (GGobiStage *)gg->d->data;
    if (d != NULL && ggobi_stage_has_vars(d)) {
      gg->pmode = (d->n_cols == 1) ? P1PLOT : XYPLOT;
      gg->imode = DEFAULT_IMODE;
    }
  }
  else {
    gg->pmode = NULL_PMODE;
    gg->imode = NULL_IMODE;
  }

  gg->pmode_prev = gg->pmode;
  gg->imode_prev = gg->imode;
  /*-- initialize the mode menus for the new mode --*/
  /*main_miscmenus_update(NULL_PMODE, NULL_IMODE, (displayd *) NULL, gg); */
}
