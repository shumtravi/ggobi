#ifndef READ_XML_H
#define READ_XML_H

#include <gtk/gtk.h>

#include "ggobi.h"

/*#include <libxml/parser.h>*/
#include <parser.h>

enum HiddenType {ROW, LINE};
enum xmlDataState { TOP = 0, DESCRIPTION, RECORD, RECORDS, VARIABLES, VARIABLE, DATA, CONNECTIONS, CONNECTION, COLORMAP, COLOR, UNKNOWN};


typedef struct {

  int color;
  int glyphType;
  int glyphSize;
  int lineColor;
  int lineWidth;
  int lineHidden;
  int hidden;

} DataOptions;

typedef struct _XMLUserData {
  enum xmlDataState state;
  int current_variable; /* Indexes the current variable. */
  int current_record;   /* Indexes the record we are currently working on. */
  int current_element;  /* Indexes the values within a record. */
  int current_segment;  /* Current segment being added. */

  int current_color;    /* The index of the current element being processed in the colormap */
 
  /* Flag that says we are reading color entries from another file via a sub-parser.
     This allows us to reuse the same instance of user data and the same handlers.
   */
  gboolean reading_colormap_file_p;

  /* A boolean indicating whether the transformation name of a variable
     was stored as an attribute.
   */
  gboolean variable_transform_name_as_attribute;

  /* The ggobi instance that is being initialized. */
  ggobid *gg;

  /* Flag indicating whether we should convert
     char arrays into null-terminated strings
     before passing them to the sub-handlers
     (e.g. setColorValue, setVariableName).
   */
  gboolean terminateStrings_p;

   /* The datasets global missing value identifier. */
  gchar *NA_identifier;
  /* The identifier for a missing value that is currently in effect.
     This might be specified per record and will be discarded
     at the end of that record.
     We could also do this columnwise.
   */
  gchar *current_NA_identifier;

    /* A set of values that apply to records when an attribute
       is not specified for that specific record but is set
       in the ggobidata tag.
     */
  DataOptions defaults;

  /* Local set of record identifiers that are used here
     for matching purposes when specifying segments.
     These are not set in the ggobid structure and 
     are different from the record's label attribute.
   */
  gchar **rowIds;

  /* Reference to the handlers being used as callbacks.
     Need this so that we can re-specify it when creating
     new sub-parsers.
   */
  xmlSAXHandlerPtr handlers;

} XMLParserData;



enum xmlDataState tagType(const gchar *name, gboolean endTag);
gboolean newVariable(const CHAR **attrs, XMLParserData *data);
gboolean setDatasetInfo(const CHAR **attrs, XMLParserData *data);
gboolean allocVariables(const CHAR **attrs, XMLParserData *data);
gboolean  newRecord(const CHAR **attrs, XMLParserData *data);

gboolean setRecordValues(XMLParserData *data, const CHAR *line, int len);
gboolean setVariableName(XMLParserData *data, const CHAR *name, int len);

const char *skipWhiteSpace(const CHAR *ch, int *len);

const gchar *getAttribute(const CHAR **attrs, char *name);


void xml_warning(const char *attribute, const char *value, const char *msg, XMLParserData *data);

void initParserData(XMLParserData *data, xmlSAXHandlerPtr handler, ggobid *gg);

gboolean setGlyph(const CHAR **attrs, XMLParserData *data, int i);
gboolean setColor(const CHAR **attrs, XMLParserData *data, int i);

gboolean allocSegments(const CHAR **attrs, XMLParserData *data);
gboolean addConnection(const CHAR **attrs, XMLParserData *data);
int rowId(const char *tmp, XMLParserData *data);

gboolean data_xml_read(const gchar *filename, ggobid *gg);

gboolean setHidden(const CHAR **attrs, XMLParserData *data, int i, enum HiddenType);

gboolean setColorValue(XMLParserData *data, const CHAR *name, int len);
gboolean setColormapEntry(const CHAR **attrs, XMLParserData *data);
gboolean setColorMap(const CHAR **attrs, XMLParserData *data);
void setColorValues(GdkColor *color, double *values);

gboolean registerColorMap(ggobid *gg);

gboolean xmlParseColorMap(const gchar *fileName, int size, XMLParserData *data);
gboolean asciiParseColorMap(const gchar *fileName, int size, XMLParserData *data);

gchar *find_xml_file(const gchar *filename, const gchar *dir, ggobid *gg);
gchar *getFileDirectory(const gchar *filename);



int asInteger(const gchar *tmp);
double asNumber(const char *sval);
gboolean asLogical(const gchar *sval);

#endif
