/* 
Provide a very simple spreadsheet-like view of a stage, particularly useful
for debugging the stages.

Long term this should probably become a custom TreeModel decorating a stage, 
but this will work for now.
*/
using GLib;
using Gtk;
using GGobi;

public class Viewer : Window {
  private Stage _stage;
  public GGobi.Stage stage { 
    get;
    set { 
      _stage = value;
      initialise_model();
    }
  }
  public TreeView table;
  public ListStore model;
  
  construct {
    title = "Data viewer";
    set_default_size(400, 600);
    
    create_widgets ();
  }

  public Viewer.new_with_stage(GGobi.Stage stage) {}

  public void create_widgets () {
    initialise_model();
 
    table = new TreeView.with_model(model);
    
    table.set_headers_visible(true);
    table.set_headers_clickable(true);
    add(table);
  }

  public void load_data() {
    TreeIter iter;

    for(uint i = 0; i < stage.n_rows; i++) {
      model.append(out iter);
      for(uint j = 0; j < stage.n_cols; j++) {
        model.set(out iter, j + 1, stage.get_string_value(i, j));
      }
    }
  }
  
  public void initialise_model() {
    uint ncols = stage.n_cols + 1;
    
    GLib.Type[] col_types = new GLib.Type[ncols];
    string[] col_labels = new string[ncols];

    col_types[0] = typeof(string);
    col_labels[0] = "Row Label";

    for(uint j = 0; j < stage.n_cols; j++) {
      Variable v = stage.get_variable(j);
      // switch (v.vartype) {
      //   case VariableType.INTEGER: col_types[j+1] = typeof(int); break;
      //   case VariableType.CATEGORICAL: col_types[j+1] = typeof(string); break;
      //   default: col_types[j+1] = typeof(double); break;
      // }
      col_types[j+ 1] = typeof(string);
      col_labels[j + 1] = v.name;
    }
    
    model = new ListStore((int) ncols, col_types);
    // TreeModel sorted = new TreeModel.with_model(model);

    load_data();
    
  }
}