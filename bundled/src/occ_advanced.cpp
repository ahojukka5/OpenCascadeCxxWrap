// occ_advanced.cpp — Splitter, Defeaturing, STL export.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepAlgoAPI_Defeaturing.hxx>
#include <StlAPI_Writer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_advanced(jlcxx::Module& mod) {
  mod.add_type<BRepAlgoAPI_Splitter>("BRepAlgoAPI_Splitter").constructor<>();
  mod.method("SetArguments", [](BRepAlgoAPI_Splitter& s, const TopTools_ListOfShape& args) {
    s.SetArguments(args);
  });
  mod.method("SetTools", [](BRepAlgoAPI_Splitter& s, const TopTools_ListOfShape& tools) {
    s.SetTools(tools);
  });
  mod.method("SetFuzzyValue", [](BRepAlgoAPI_Splitter& s, double v) { s.SetFuzzyValue(v); });
  mod.method("Build",       [](BRepAlgoAPI_Splitter& s) { s.Build(); });
  mod.method("Shape",       [](BRepAlgoAPI_Splitter& s) -> TopoDS_Shape { return s.Shape(); });
  mod.method("IsDone",      [](const BRepAlgoAPI_Splitter& s) -> bool { return bool(s.IsDone()); });
  mod.method("Modified",    [](BRepAlgoAPI_Splitter& s, const TopoDS_Shape& a) -> TopTools_ListOfShape {
    return s.Modified(a);
  });
  mod.method("Generated",   [](BRepAlgoAPI_Splitter& s, const TopoDS_Shape& a) -> TopTools_ListOfShape {
    return s.Generated(a);
  });
  mod.method("IsDeleted",   [](BRepAlgoAPI_Splitter& s, const TopoDS_Shape& a) -> bool {
    return bool(s.IsDeleted(a));
  });

  mod.add_type<BRepAlgoAPI_Defeaturing>("BRepAlgoAPI_Defeaturing").constructor<>();
  mod.method("SetShape", [](BRepAlgoAPI_Defeaturing& d, const TopoDS_Shape& s) { d.SetShape(s); });
  mod.method("AddFaceToRemove", [](BRepAlgoAPI_Defeaturing& d, const TopoDS_Face& f) {
    d.AddFaceToRemove(f);
  });
  mod.method("Build",  [](BRepAlgoAPI_Defeaturing& d) { d.Build(); });
  mod.method("Shape",  [](BRepAlgoAPI_Defeaturing& d) -> TopoDS_Shape { return d.Shape(); });
  mod.method("IsDone", [](const BRepAlgoAPI_Defeaturing& d) -> bool { return bool(d.IsDone()); });

  mod.add_type<StlAPI_Writer>("StlAPI_Writer").constructor<>();
  mod.method("Write", [](StlAPI_Writer& w, const TopoDS_Shape& s, const std::string& f) {
    return bool(w.Write(s, f.c_str()));
  });
}
