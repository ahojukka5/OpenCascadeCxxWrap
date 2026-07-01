#include <jlcxx/jlcxx.hpp>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <GeomAbs_JoinType.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_sweep(jlcxx::Module& mod)
{
  // --- Type registrations (all before any mod.method calls) ---

  mod.add_type<BRepOffsetAPI_MakePipe>("BRepOffsetAPI_MakePipe")
    .constructor<const TopoDS_Wire&, const TopoDS_Shape&>();

  mod.add_type<BRepOffsetAPI_ThruSections>("BRepOffsetAPI_ThruSections")
    .constructor<>()
    .constructor<bool>()
    .constructor<bool, bool>();

  mod.add_type<BRepOffsetAPI_MakeOffsetShape>("BRepOffsetAPI_MakeOffsetShape")
    .constructor<>();

  mod.add_type<BRepOffsetAPI_MakeOffset>("BRepOffsetAPI_MakeOffset")
    .constructor<>();

  mod.add_type<BRepOffsetAPI_MakePipeShell>("BRepOffsetAPI_MakePipeShell")
    .constructor<const TopoDS_Wire&>();
  mod.method("Add", [](BRepOffsetAPI_MakePipeShell& p, const TopoDS_Shape& s, bool withContact, bool withCorrection) {
    p.Add(s, withContact, withCorrection);
  });
  mod.method("Build", [](BRepOffsetAPI_MakePipeShell& p) { p.Build(); });
  mod.method("IsDone", [](const BRepOffsetAPI_MakePipeShell& p) -> bool { return bool(p.IsDone()); });
  mod.method("Shape", [](BRepOffsetAPI_MakePipeShell& p) -> TopoDS_Shape { return p.Shape(); });
  mod.method("SetMode", [](BRepOffsetAPI_MakePipeShell& p, bool isFrenet) { p.SetMode(isFrenet); });
  mod.method("SetTransitionMode", [](BRepOffsetAPI_MakePipeShell& p, int mode) {
    p.SetTransitionMode(BRepBuilderAPI_TransitionMode(mode));
  });

  mod.add_type<BRepOffsetAPI_MakeEvolved>("BRepOffsetAPI_MakeEvolved")
    .constructor<>()
    .constructor<const TopoDS_Shape&, const TopoDS_Wire&>();
  mod.method("Build", [](BRepOffsetAPI_MakeEvolved& e) { e.Build(); });
  mod.method("IsDone", [](const BRepOffsetAPI_MakeEvolved& e) -> bool { return bool(e.IsDone()); });
  mod.method("Shape", [](BRepOffsetAPI_MakeEvolved& e) -> TopoDS_Shape { return e.Shape(); });

  // --- BRepOffsetAPI_MakePipe methods ---

  mod.method("Shape", [](BRepOffsetAPI_MakePipe& pipe) -> TopoDS_Shape {
    return pipe.Shape();
  });

  mod.method("Build", [](BRepOffsetAPI_MakePipe& pipe) {
    pipe.Build();
  });

  mod.method("IsDone", [](BRepOffsetAPI_MakePipe& pipe) -> bool {
    return bool(pipe.IsDone());
  });

  mod.method("ErrorOnSurface", [](BRepOffsetAPI_MakePipe& pipe) -> double {
    return pipe.ErrorOnSurface();
  });

  // --- BRepOffsetAPI_ThruSections methods ---

  mod.method("AddWire", [](BRepOffsetAPI_ThruSections& t, const TopoDS_Wire& wire) {
    t.AddWire(wire);
  });

  mod.method("AddVertex", [](BRepOffsetAPI_ThruSections& t, const TopoDS_Vertex& v) {
    t.AddVertex(v);
  });

  mod.method("Build", [](BRepOffsetAPI_ThruSections& t) {
    t.Build();
  });

  mod.method("IsDone", [](BRepOffsetAPI_ThruSections& t) -> bool {
    return bool(t.IsDone());
  });

  mod.method("Shape", [](BRepOffsetAPI_ThruSections& t) -> TopoDS_Shape {
    return t.Shape();
  });

  mod.method("CheckCompatibility", [](BRepOffsetAPI_ThruSections& t, bool check) {
    t.CheckCompatibility(check);
  });

  // --- BRepOffsetAPI_MakeOffsetShape methods ---

  mod.method("PerformBySimple", [](BRepOffsetAPI_MakeOffsetShape& m,
                                    const TopoDS_Shape& shape,
                                    double offset) {
    m.PerformBySimple(shape, offset);
  });

  mod.method("IsDone", [](BRepOffsetAPI_MakeOffsetShape& m) -> bool {
    return bool(m.IsDone());
  });

  mod.method("Shape", [](BRepOffsetAPI_MakeOffsetShape& m) -> TopoDS_Shape {
    return m.Shape();
  });

  // --- BRepOffsetAPI_MakeOffset methods ---

  mod.method("Init", [](BRepOffsetAPI_MakeOffset& m, const TopoDS_Face& f, int jt, bool isOpen) {
    m.Init(f, GeomAbs_JoinType(jt), isOpen);
  });
  mod.method("AddWire", [](BRepOffsetAPI_MakeOffset& m, const TopoDS_Wire& w) {
    m.AddWire(w);
  });
  mod.method("Perform", [](BRepOffsetAPI_MakeOffset& m, double offset, double alt) {
    m.Perform(offset, alt);
  });
  mod.method("Build", [](BRepOffsetAPI_MakeOffset& m) {
    m.Build();
  });
  mod.method("IsDone", [](BRepOffsetAPI_MakeOffset& m) -> bool {
    return bool(m.IsDone());
  });
  mod.method("Shape", [](BRepOffsetAPI_MakeOffset& m) -> TopoDS_Shape {
    return m.Shape();
  });

  // --- GeomAbs_JoinType constants ---

  mod.method("GeomAbs_Arc",          []() { return int(GeomAbs_Arc); });
  mod.method("GeomAbs_Tangent",      []() { return int(GeomAbs_Tangent); });
  mod.method("GeomAbs_Intersection", []() { return int(GeomAbs_Intersection); });

  mod.method("BRepBuilderAPI_Transformed",  []() { return int(BRepBuilderAPI_Transformed); });
  mod.method("BRepBuilderAPI_RightCorner",  []() { return int(BRepBuilderAPI_RightCorner); });
  mod.method("BRepBuilderAPI_RoundCorner",  []() { return int(BRepBuilderAPI_RoundCorner); });
}
