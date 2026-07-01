// occ_feat.cpp — 1:1 CxxWrap bindings for BRepFeat_MakePrism, BRepFeat_MakeRevol
// (feature-tree boss/pocket modeling: extrude/revolve a sketch up to a face).
#include <jlcxx/jlcxx.hpp>

#include <BRepFeat_MakePrism.hxx>
#include <BRepFeat_MakeRevol.hxx>
#include <BRepFeat_MakeDPrism.hxx>
#include <BRepFeat_SplitShape.hxx>
#include <BRepFeat_Gluer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>

void register_occ_feat(jlcxx::Module& mod) {
  mod.add_type<BRepFeat_MakePrism>("BRepFeat_MakePrism")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, const TopoDS_Face&,
                  const gp_Dir&, int, bool>();

  mod.add_type<BRepFeat_MakeRevol>("BRepFeat_MakeRevol")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, const TopoDS_Face&,
                  const gp_Ax1&, int, bool>();

  // ---- BRepFeat_MakePrism ----
  mod.method("Init", [](BRepFeat_MakePrism& m, const TopoDS_Shape& sbase, const TopoDS_Shape& pbase,
                         const TopoDS_Face& skface, const gp_Dir& dir, int fuse, bool modify) {
    m.Init(sbase, pbase, skface, dir, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakePrism& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakePrism& m, double length) { m.Perform(length); });
  mod.method("Perform", [](BRepFeat_MakePrism& m, const TopoDS_Shape& until) { m.Perform(until); });
  mod.method("Perform", [](BRepFeat_MakePrism& m, const TopoDS_Shape& from, const TopoDS_Shape& until) {
    m.Perform(from, until);
  });
  mod.method("PerformUntilEnd",  [](BRepFeat_MakePrism& m) { m.PerformUntilEnd(); });
  mod.method("PerformFromEnd",   [](BRepFeat_MakePrism& m, const TopoDS_Shape& fUntil) {
    m.PerformFromEnd(fUntil);
  });
  mod.method("PerformThruAll",   [](BRepFeat_MakePrism& m) { m.PerformThruAll(); });
  mod.method("PerformUntilHeight", [](BRepFeat_MakePrism& m, const TopoDS_Shape& until, double length) {
    m.PerformUntilHeight(until, length);
  });
  mod.method("Shape",  [](BRepFeat_MakePrism& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakePrism& m) -> bool { return bool(m.IsDone()); });

  // ---- BRepFeat_MakeRevol ----
  mod.method("Init", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& sbase, const TopoDS_Shape& pbase,
                         const TopoDS_Face& skface, const gp_Ax1& axis, int fuse, bool modify) {
    m.Init(sbase, pbase, skface, axis, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakeRevol& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, double angle) { m.Perform(angle); });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& until) { m.Perform(until); });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& from, const TopoDS_Shape& until) {
    m.Perform(from, until);
  });
  mod.method("PerformThruAll",    [](BRepFeat_MakeRevol& m) { m.PerformThruAll(); });
  mod.method("PerformUntilAngle", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& until, double angle) {
    m.PerformUntilAngle(until, angle);
  });
  mod.method("Shape",  [](BRepFeat_MakeRevol& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeRevol& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepFeat_MakeDPrism>("BRepFeat_MakeDPrism")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Face&, const TopoDS_Face&,
                  double, int, bool>();
  mod.method("Perform", [](BRepFeat_MakeDPrism& m, double length) { m.Perform(length); });
  mod.method("Shape",  [](BRepFeat_MakeDPrism& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeDPrism& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepFeat_SplitShape>("BRepFeat_SplitShape")
     .constructor<const TopoDS_Shape&>();
  mod.method("Add", [](BRepFeat_SplitShape& m, const TopoDS_Wire& w, const TopoDS_Face& f) {
    m.Add(w, f);
  });
  mod.method("Build",  [](BRepFeat_SplitShape& m) { m.Build(); });
  mod.method("Shape",  [](BRepFeat_SplitShape& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_SplitShape& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepFeat_Gluer>("BRepFeat_Gluer")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>();
  mod.method("Bind",   [](BRepFeat_Gluer& m, const TopoDS_Face& f1, const TopoDS_Face& f2) {
    m.Bind(f1, f2);
  });
  mod.method("Build",  [](BRepFeat_Gluer& m) { m.Build(); });
  mod.method("Shape",  [](BRepFeat_Gluer& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_Gluer& m) -> bool { return bool(m.IsDone()); });
}
