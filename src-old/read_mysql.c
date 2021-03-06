/*-- read_mysql.c --*/
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

#include "properties.h"
#include "read_mysql.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>


static int initialized = 0;

MySQLLoginInfo DefaultMySQLInfo = {
  NULL,
  NULL,
  NULL,                         /* If present in the default file, will be filled in, even if no value is specified. */
  NULL,
  0,
  NULL,
  0,
  NULL,
  NULL,
  NULL,
  NULL
};


MySQLLoginInfo *initMySQLLoginInfo (MySQLLoginInfo * login);

void GGOBI (cancelMySQLGUI) (GtkButton * button, MySQLGUIInput * guiInput);
void GGOBI (getMySQLGUIInfo) (GtkButton * button, MySQLGUIInput * guiInput);
void GGOBI (getMySQLGUIHelp) (GtkButton * button, MySQLGUIInput * guiInput);

char *getMySQLLoginElement (int i, int *isCopy, MySQLLoginInfo * info);


/*
  This attempts to establish the SQL connection and read the data 
  based on the dataQuery field of the speified login.
  This checks that the user has set the password field (at least
  to an empty string). If not, the GUI dialog is created.
  There is a potential for this to be done recursively, but
  it shouldn't happen. Firstly, the event structure is such that 
  the calls will not be in the same call stack.
  Secondly, it is based on the difference between NULL and ""
  and the fact that getting the text in an Gtk entry gives "" if it is
  empty.
 */
int
read_mysql_data (MySQLLoginInfo * login, gint init, ggobid * gg)
{
  MYSQL *conn;
  GGobiData *d;

  if (login->password == NULL) {
    /* The user hasn't specified a password.
       Even if they don't need one, they should declare
       this by leaving the password empty in the GUI
       dialog. This is the difference between NULL and "".

       Note that when the gui's ok button is clicked,
       this routine will be called again. This is not recursive.
     */
    GGOBI (get_mysql_login_info) (login, gg);
    return (-1);
  }

  /* Now connect to the SQL. */
  conn = GGOBI (mysql_connect) (login, gg);
  if (conn == NULL) {
    if (login != &DefaultMySQLInfo)
      free (login);             // maybe we should never be doing this here.
    return (0);
  }

  /* Execute the query specified by the user to get the data. */
  if ((d = GGOBI (get_mysql_data) (conn, login->dataQuery, gg)) != NULL) {
    return (-1);
  }


/*
  segments_alloc (gg->nsegments, gg);
 *if(gg->nsegments < 1)
 * segments_create(gg);
*/


  /* Now we have to read in the glyph and color data. */
  if (init && d)
    datad_init (d, gg, true);

  return (1);                   /* everything was ok */
}


/*
   Creates the connection to the SQL server.
   This doesn't run any queries, just attempts to login
   and gain access to the specified database.
 */

MYSQL *GGOBI (mysql_connect) (MySQLLoginInfo * login, ggobid * gg)
{
  MYSQL *conn;
  if (initialized == 0) {

  }

  conn = mysql_init (NULL);
  if (conn == NULL) {
    GGOBI (mysql_warning) ("Can't initialize mysql!", conn, gg);
    return (NULL);
  }

  conn = mysql_real_connect (conn, login->host, login->user, login->password,
                             login->dbname, login->port, login->socket,
                             login->flags);

  if (conn == NULL) {
    GGOBI (mysql_warning) ("Can't connect to mysql!", conn, gg);
    return (NULL);
  }

  return (conn);
}


/*
   Performs the specified query in the SQL server 
   using the connection and then reads the data and 
   puts it in the ggobid raw.data array.
 */
GGobiData *GGOBI (get_mysql_data) (MYSQL * conn, const char *query,
                                   ggobid * gg)
{
  MYSQL_RES *res;
  int status;
  GGobiData *d = NULL;

  if (query == NULL || query[0] == '\0')
    return (NULL);

  status = mysql_query (conn, query);

  if (status || (res = mysql_store_result (conn)) == NULL) {
    GGOBI (mysql_warning) ("Error from query", conn, gg);
    return (NULL);
  }

  d = ggobi_data_new ();
  d->input = fileset_generate (query, "mysql", gg);
  if (d->input) {
    d->input->baseName = g_strdup (query);
  }
  GGOBI (register_mysql_data) (conn, res, 1, gg);

  return (d);
}

/**
   Takes the result set and  reads the names of the variables
   from its meta-data and then the values for each observation.
 */
int
GGOBI (register_mysql_data) (MYSQL * conn, MYSQL_RES * res, gint preFetched,
                             GGobiData * d, ggobid * gg)
{
  unsigned glong j, rownum = 0;
  unsigned glong nrows, ncols;
  MYSQL_ROW row;
  vartabled *vt;

  nrows = mysql_num_rows (res);
  ncols = mysql_num_fields (res);

  GGOBI (setDimensions) (nrows, ncols, gg);

  for (j = 0; j < ncols; j++) {
    MYSQL_FIELD *field = mysql_fetch_field (res);
    vt = vartable_element_get (j, d);
    vt->collab = g_strdup (field->name);
    vt->collab_tform = g_strdup (field->name);
    //XXX    vt->groupid = vt->groupid_ori = i;
  }

  while ((row = mysql_fetch_row (res)) != NULL) {
    for (j = 0; j < ncols; j++) {
      d->raw.vals[rownum][i] = atof (row[j]);
    }
    rownum++;
  }

  return (j);
}

/**
  Displays a warning/error from MySQL's error message using a dialog.
  Uses quick_message.
 */
void GGOBI (mysql_warning) (const char *msg, MYSQL * conn, ggobid * gg)
{
  char *errmsg = NULL;
  char *buf;
  if (conn) {
    errmsg = mysql_error (conn);
    if (errmsg == NULL)
      errmsg = "";
  }
  else
    errmsg = "";

  buf =
    (char *) g_malloc (sizeof (char) * (strlen (errmsg) + strlen (msg) + 2));
  sprintf (buf, "%s %s", msg, errmsg);

  quick_message (buf, true);
  free (buf);
}

/*
  Once we know the dimension of the data (number of observations
  and columns), we call this to setup up the space to store the data,
  etc.
  This should go elsewhere also.

  Can now use ggobi_data_new();
 */
void GGOBI (setDimensions) (gint nrow, gint ncol, GGobiData * d, ggobid * gg)
{
  d->nrows = nrow;
  d->nrows_in_plot = d->nrows;  /*-- for now --*/

  rowlabels_alloc (d, gg);
  br_glyph_ids_alloc (d);
  br_glyph_ids_init (d, gg);

  br_color_ids_alloc (d, gg);
  br_color_ids_init (d, gg);

  d->ncols = ncol;

  arrayf_alloc (&d->raw, d->nrows, d->ncols);

  vartable_alloc (d);
  vartable_init (d);
  br_hidden_alloc (d);
}

/*
  Optionally allocate and initialize the MySQLLoginInfo
  instance by copying the values from the DefaultMySQLInfo.
 */
MySQLLoginInfo *
initMySQLLoginInfo (MySQLLoginInfo * login)
{
  if (login == NULL)
    login = (MySQLLoginInfo *) g_malloc0 (sizeof (MySQLLoginInfo));

  *login = DefaultMySQLInfo;

  return (login);
}



const char *fieldNames[] = { "Host",
  "User",
  "Password",
  "Database",
  "Port",
  "Socket",
  "Flags",
  NULL,
  "Data query",
  "Segments query",
  "Appearance query",
  "Color query"
};


/*
  This maps the "field" name of the MySQLLoginInfo
 */
MySQLInfoElement
getMySQLLoginElementIndex (const char *name)
{
  unsigned int i;
  for (i = 0; i < sizeof (fieldNames) / sizeof (fieldNames[0]); i++) {
    if (fieldNames[i] == NULL)
      continue;
    if (strcmp (fieldNames[i], name) == 0)
      return ((MySQLInfoElement) i);
  }

  return (MISS);
}

int
setMySQLLoginElement (MySQLInfoElement i, char *val, MySQLLoginInfo * info)
{
  switch (i) {
  case HOST:
    info->host = val;
    break;
  case USER:
    info->user = val;
    break;
  case PASSWORD:
    info->password = val;
    break;
  case DATABASE:
    info->dbname = val;
    break;
  case PORT:
    info->port = atoi (val);
    break;
  case SOCKET:
    info->socket = val;
    break;
  case FLAGS:
    info->flags = atoi (val);
    break;
  case DATA_QUERY:
    info->dataQuery = val;
    break;
  case COLOR_QUERY:
    info->colorQuery = val;
    break;
  case SEGMENTS_QUERY:
    info->segmentsQuery = val;
    break;
  default:
    break;
  }

  return (i);
}

/*
  Retrieves the value of the element in the MySQLLoginInfo
  associated with the identifier (i).
 */
char *
getMySQLLoginElement (MySQLInfoElement i, int *isCopy, MySQLLoginInfo * info)
{
  char *val = NULL;
  switch (i) {
  case HOST:
    val = info->host;
    break;
  case USER:
    val = info->user;
    break;
  case PASSWORD:
    val = info->password;
    break;
  case DATABASE:
    val = info->dbname;
    break;
  case PORT:
    val = NULL;
    break;
  case SOCKET:
    val = NULL;
    break;
  case FLAGS:
    val = NULL;
    break;
  case DATA_QUERY:
    val = info->dataQuery;
    break;
  case COLOR_QUERY:
    val = info->colorQuery;
    break;
  case SEGMENTS_QUERY:
    val = info->segmentsQuery;
    break;
  default:
    break;
  }

  return (val);
}

/*
   Read the MySQL defaults from the specified file
   and fill in the DefaultMySQLInfo with these values.

   This reads the specified file using the TrimmedProperties
   class and then iterates over the values to add them to the
   DefaultMySQLInfo object.
 */
int
getDefaultValuesFromFile (char *fileName)
{
  Properties *props = new TrimmedProperties (fileName);
  Property *prop;
  unsigned int i, ctr = 0;
  MySQLInfoElement id;

  for (i = 0; i < props->size (); i++) {
    prop = props->element (i);
    id = getMySQLLoginElementIndex (prop->getName ());
    if (id > -1) {
      setMySQLLoginElement (id, prop->getValue (), &DefaultMySQLInfo);
      ctr++;
    }
  }

  return (ctr);
}

/* 
  From here on is mainly GUI related material.
 */


/*
   This creates the interactive dialog with which the user can specify the
   different parameters for the SQL connection, including the host, user,
   password, etc. and the query to get the data, color table, etc.
   The default values to display are taken from the MySQLLoginInfo
   object passed to this routine. If it is null, the default info
   is used. This allows the values read from an input file to be used
   as partial specification.
 */
MySQLGUIInput *GGOBI (get_mysql_login_info) (MySQLLoginInfo * info,
                                             ggobid * gg)
{
  int i, ctr;
  GtkWidget *dialog, *lab, *input, *table;
  GtkWidget *okay_button, *cancel_button, *help_button;
  MySQLGUIInput *guiInputs;
  char *tmpVal;
  int isCopy;
  int n = sizeof (fieldNames) / sizeof (fieldNames[0]);

  if (info == NULL)
    info = &DefaultMySQLInfo;

  guiInputs = (MySQLGUIInput *) g_malloc (sizeof (MySQLGUIInput));

  /* Create the GUI and its components. */
  dialog =
    gtk_dialog_new_with_buttons ("MySQL Login & Query Settings", NULL, 0,
                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                 GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);
  //gtk_window_set_title(GTK_WINDOW(dialog), "MySQL Login & Query Settings");

  guiInputs->gg = gg;
  guiInputs->dialog = dialog;
  guiInputs->textInput = (GtkWidget **) g_malloc (sizeof (GtkWidget *) * n);
  guiInputs->numInputs = n;

  table = gtk_table_new (n, 2, 0);
  /* Now run through all the entries of interest and generate  
     the label, text entry pair. Store the entry widget
     in the guiInputs array of textInput elements.
     Then they can be queried in the handler of the Ok button click.
   */
  for (i = 0, ctr = 0; i < n; i++) {
    if (fieldNames[i] == NULL) {
      guiInputs->textInput[i] = NULL;
      continue;
    }
    lab = gtk_label_new (fieldNames[i]);
    gtk_label_set_justify (GTK_LABEL (lab), GTK_JUSTIFY_LEFT);
    input = gtk_entry_new ();
    if (i == PASSWORD)
      gtk_entry_set_visibility (GTK_ENTRY (input), FALSE);
    guiInputs->textInput[i] = input;

    tmpVal = getMySQLLoginElement ((MySQLInfoElement) i, &isCopy, info);
    if (tmpVal)
      gtk_entry_set_text (GTK_ENTRY (input), tmpVal);

    gtk_table_attach_defaults (GTK_TABLE (table), lab, 0, 1, ctr, ctr + 1);
    gtk_table_attach_defaults (GTK_TABLE (table), input, 1, 2, ctr, ctr + 1);
    ctr++;
  }

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, TRUE, TRUE,
                      0);

  while (true) {
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    if (response == GTK_RESPONSE_HELP)
      GGOBI (getMySQLGUIHelp) (guiInputs);
    else if (response == GTK_RESPONSE_CANCEL
             || GGOBI (getMySQLGUIInfo) (guiInputs))
      break;
  }

  /* Now add the buttons at the bottom of the dialog. */
  /*okay_button = gtk_button_new_with_label("Okay");
     cancel_button = gtk_button_new_with_label("Cancel");
     help_button = gtk_button_new_with_label("Help");
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel_button);
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), help_button);

     gtk_widget_show_all(dialog);
   */
  /* Now setup the action/signal handlers. */
  /*g_signal_connect (G_OBJECT (cancel_button), "clicked",
     G_CALLBACK (GGOBI(cancelMySQLGUI)), guiInputs);

     g_signal_connect (G_OBJECT (okay_button), "clicked",
     G_CALLBACK (GGOBI(getMySQLGUIInfo)), guiInputs);
     g_signal_connect (G_OBJECT (help_button), "clicked",
     G_CALLBACK (GGOBI(getMySQLGUIHelp)), guiInputs);
   */

  gtk_widget_destroy (dialog);
  g_free (guiInputs);

  return (NULL);
}


/*
   Callback for the Ok button which processes the user's
   entries for all of the fields and packages them up into
   an MySQLLoginInfo object. Then it calls the read_mysql_data
   with this information.
   The guiInput argument contains the ggobid object reference
   and the array of input/entry widgets.
 */
void GGOBI (getMySQLGUIInfo) (MySQLGUIInput * guiInput)
{
  ggobid *gg = guiInput->gg;
  gint i;
  gchar *val = NULL;
  MySQLLoginInfo *info = initMySQLLoginInfo (NULL);

  for (i = 0; i < guiInput->numInputs; i++) {
    if (guiInput->textInput[i] == NULL)
      continue;

    val =
      gtk_editable_get_chars (GTK_EDITABLE (guiInput->textInput[i]), 0, -1);

    if (val)
      /*val = g_strdup(val); */
      /* Is this necessary with gtk_editable_get_chars? I bet not.  dfs */
      ;
    else
      continue;

    setMySQLLoginElement ((MySQLInfoElement) i, val, info);
    val = NULL;
  }

  /* Only cancel if we read something. Otherwise,
     leave the display for the user to edit.
   */
  if (read_mysql_data (info, TRUE, gg) > 0) {
    GGOBI (cancelMySQLGUI) (button, guiInput);
    /* Can we free the info here. */
  }
}

/*
  Close the specified dialog and free up the associated GUI info.
 */
/*
void
GGOBI(cancelMySQLGUI)(GtkButton *button, MySQLGUIInput *guiInput)
{
  gtk_widget_destroy (guiInput->dialog);
  g_free (guiInput);
}
*/
void GGOBI (getMySQLGUIHelp) (MySQLGUIInput * guiInput)
{
  quick_message ("GGobi/MySQL help not implemented yet!", false);
}
