using GLib;

public class GGobi.TransformMinOne : Transform {
  public override double[]? forward (double[] vals, Variable v) 
  { 
    double[] results = new double[vals.length];
    double min = v.get_min();
    for (uint i = 0; i < results.length; i++)
      results[i] = vals[i] - min + 1.0;
    return results; 
  }
  
  public override double[]? reverse (double[] vals, Variable v) 
  {
    double[] results = new double[vals.length];
    double min = v.get_min();
    for (uint i = 0; i < results.length; i++)
      results[i] = vals[i] + min - 1.0;
    return results;
  }

  public override string variable_name(string name)
  {
    return name.printf("%s >= 1");
  }
  
  public override string get_name()
  {
    return "Raise minimum to one";
  }

  public override string get_description()
  {
    return "Subtract the variable minimum from each value, then add one";
  }
}
