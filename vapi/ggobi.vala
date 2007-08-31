[CCode (cheader_filename = "ggobi-stage.h")]
public class GGobi.Stage {
  public uint n_rows { get; set construct; }
  public uint n_cols { get; set construct; }
  public string name { get; set construct; }
  
  protected Stage parent;
  
  public virtual void set_col_name(uint j, string name);
  public virtual void set_row_id(uint i, string id);
  public void set_string_value(uint i, uint j, string val);
  
  public virtual double get_raw_value(uint i, uint j);
  public virtual void set_raw_value(uint i, uint j, double value);
  
  
  public Variable get_variable(uint j);
  
  public void col_data_changed(uint j);
  public void flush_changes_here();
  
  virtual void process_incoming(PipelineMessage msg);
  
}


[CCode (cheader_filename = "ggobi-data.h")]
public class GGobi.Data : GGobi.Stage {
  public GGobi.InputSource source { get; set construct; }
  
  public void add_attributes();
  
  public Data(uint nrows, uint ncols);
}

[CCode (cheader_filename = "ggobi-pipeline-message.h")]
public class GGobi.PipelineMessage {
  public GLib.SList get_changed_cols();
  public GLib.SList get_removed_cols();
  public GLib.SList get_removed_rows();
  public uint get_n_added_rows();
  public uint get_n_removed_rows();
  public uint get_n_added_cols();
  public uint get_n_changed_cols();
  public uint get_n_removed_cols();
  

}


[CCode (cheader_filename = "ggobi-variable.h")]
public class GGobi.Variable {
  public float get_range();
  

}