#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

#include "GGStructSizes.c"

void       close_ggvis_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_ggvis_window(ggobid *gg, PluginInstance *inst);
void       show_ggvis_window (GtkWidget *widget, PluginInstance *inst);


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "ggvis (MDS) ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_ggvis_window), inst);
  return(true);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_IO",            NULL,     NULL,             0, "<Branch>" },
  { "/IO/Save distance matrix ...",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/_Reset",         NULL,     NULL,             0, "<Branch>" },
  { "/Reset/Center and scale",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/_View",         NULL,     NULL,             0, "<Branch>" },
  { "/View/Shepard Plot",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/_Help",         NULL,     NULL,             0, "<LastBranch>" },
  { "/Help/MDS Background",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/MDS Controls",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/Formula for Kruskal-Shepard distance scaling",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/Formula for Torgerson-Gower dot-product scaling (classic)",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  };


void
ggvis_init (ggvisd *ggv) {
/*
  arrayd_init_null (&ggv->dist_orig);
  arrayd_init_null (&ggv->dist);
  arrayd_init_null (&ggv->pos_orig);
  arrayd_init_null (&ggv->pos);
*/
}

static const gchar *const metric_lbl[] = {"Metric MDS", "Nonmetric MDS"};
static void metric_cb (GtkWidget *w, gpointer cbd)
{
}
static const gchar *const kruskal_lbl[] = {"Kruskal/Shepard", "Classic"};
static void kruskal_cb (GtkWidget *w, gpointer cbd)
{
}
static const gchar *const groups_lbl[] = {
  "Scale all",
  "Within groups",
  "Between groups",
  "Anchors scale",
  "Anchors fixed"};
static void groups_cb (GtkWidget *w, gpointer cbd)
{
}
static const gchar *const constrained_lbl[] = {
  "No variables frozen",
  "First variable frozen",
  "First two variables frozen"};
static void constrained_cb (GtkWidget *w, gpointer cbd)
{
}

void
show_ggvis_window (GtkWidget *widget, PluginInstance *inst)
{
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    GtkWidget *window;
    ggvisd *gl = (ggvisd *) g_malloc (sizeof (ggvisd));

    ggvis_init (gl);

    window = create_ggvis_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "ggvisd", gl);
    inst->data = window;  /*-- or this could be the ggvis structure --*/

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

ggvisd *
ggvisFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  ggvisd *gl = (ggvisd *) gtk_object_get_data (GTK_OBJECT(window), "ggvisd");
  return gl;
}

void ggvis_scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_digits (scale, 2);
  gtk_scale_set_value_pos (scale, GTK_POS_BOTTOM);
  gtk_scale_set_draw_value (scale, TRUE);
}

GtkWidget *
create_ggvis_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *run_vbox;
  GtkWidget *notebook, *opt;
  GtkWidget *label, *frame, *btn, *vbox, *hbox, *hb, *vb, *hscale, *table;
  GtkObject *adj;
  gint top;
  GtkAccelGroup *ggv_accel_group;
  GtkWidget *menubar;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggvis");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
    GTK_SIGNAL_FUNC (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  /* main menu bar */
  ggv_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items,
    sizeof (menu_items) / sizeof (menu_items[0]),
    ggv_accel_group, window,
    &menubar, (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, false, false, 0);

  hbox = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, false, false, 2);

/*-- Run controls --*/
  frame = gtk_frame_new ("Run controls");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (hbox), frame, false, false, 2);

  run_vbox = gtk_vbox_new (false, 6);
  gtk_container_add (GTK_CONTAINER(frame), run_vbox);

  btn = gtk_check_button_new_with_label ("Run MDS");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);
  /*-- stepsize --*/
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (run_vbox), hb, false, false, 2);
  label = gtk_label_new ("Stepsize:");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

  adj = gtk_adjustment_new (0.01, 0.0001, 0.2, 0.02, 0.2, 0.10);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 4);
  gtk_box_pack_start (GTK_BOX (hb), hscale, false, false, 2);
  /*-- --*/
  btn = gtk_button_new_with_label ("Step");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);
  btn = gtk_button_new_with_label ("Reinit");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);


  /*-- Plot of stress function --*/
  frame = gtk_frame_new ("Stress function");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (hbox), frame, false, false, 2);


/*-- notebook --*/

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

  /*-- network tab: cmds --*/
  frame = gtk_frame_new ("MDS Parameters");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

/*-- MDS parameters --*/

  hbox = gtk_hbox_new (false, 1);  /* 2 children: table, drawing area */
  gtk_container_add (GTK_CONTAINER(frame), hbox);

  /*-- MDS Dimension, Data Power, Dist Power, Minkowski, Weight power --*/
  table = gtk_table_new (2, 5, false);
  gtk_box_pack_start (GTK_BOX (hbox), table, false, false, 5);

  top = 0;


  /*-- MDS Dimension --*/
  top++;
  label = gtk_label_new ("Dimension (k)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (3.0, 1.0, 10.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Data power --*/
  top++;
  label = gtk_label_new ("Data Power (D^p)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (1.0, 0.0, 6.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Dist power --*/
  top++;
  label = gtk_label_new ("Dist Power (d^q)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (1.0, 0.0, 6.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Minkowski --*/
  top++;
  label = gtk_label_new ("Minkowski norm (m)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (2.0, 1.0, 6.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Weight power --*/
  top++;
  label = gtk_label_new ("Weight power (w=D^r)");
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (0.0, -4.0, 4.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Metric/Non-Metric option menu --*/
  top++;
  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) metric_lbl,
    sizeof (metric_lbl) / sizeof (gchar *),
    (GtkSignalFunc) metric_cb, "GGobi", gg);
  gtk_table_attach (GTK_TABLE (table), opt, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Kruskal/Classes option menu --*/
  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) kruskal_lbl,
    sizeof (kruskal_lbl) / sizeof (gchar *),
    (GtkSignalFunc) kruskal_cb, "GGobi", gg);
  gtk_table_attach (GTK_TABLE (table), opt, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Parameters");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- Groups --*/
  frame = gtk_frame_new ("Groups");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_check_button_new_with_label ("Use brush groups");
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);

  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) groups_lbl,
    sizeof (groups_lbl) / sizeof (gchar *),
    (GtkSignalFunc) groups_cb, "GGobi", gg);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);

  label = gtk_label_new ("Groups");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- Sensitivity Analysis --*/
  frame = gtk_frame_new ("Sensitivity analysis");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  table = gtk_table_new (2, 3, false);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 5);
  top = 0;

  /*-- selection probability slider and button --*/
  adj = gtk_adjustment_new (1.0, 0.0, 1.0, 0.2, 0.2, 0.10);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Selection probability");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  btn = gtk_button_new_with_label ("Refresh");
  gtk_table_attach (GTK_TABLE (table), btn, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/

  /*-- perturbation slider and button --*/
  top++;
  adj = gtk_adjustment_new (0.0, 0.0, 1.0, 0.2, 0.2, 0.10);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Perturbation");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);

  btn = gtk_button_new_with_label ("Refresh");
  gtk_table_attach (GTK_TABLE (table), btn, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/


  label = gtk_label_new ("Sensitivity");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- Constrained MDS --*/
  frame = gtk_frame_new ("Constrained MDS");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) constrained_lbl,
    sizeof (constrained_lbl) / sizeof (gchar *),
    (GtkSignalFunc) constrained_cb, "GGobi", gg);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);

  label = gtk_label_new ("Constrained MDS");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  gtk_widget_show_all (window);

  return(window);
}


void close_ggvis_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_ggvis_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}
