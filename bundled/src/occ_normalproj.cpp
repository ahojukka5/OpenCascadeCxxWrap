// occ_normalproj.cpp — 1:1 CxxWrap binding for BRepAlgo_NormalProjection:
// projects a wire/edge onto a target shape's surface(s) along the normal
// direction. Unlocks curve-on-surface, sketch-on-curved-face, and
// split-face workflows. A plain class (no Standard_Transient inheritance,
// no Handle-management concerns at all).
#include <jlcxx/jlcxx.hpp>

#include <BRepAlgo_NormalProjection.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_normalproj(jlcxx::Module& mod) {
  mod.add_type<BRepAlgo_NormalProjection>("BRepAlgo_NormalProjection")
     .constructor<>()
     .constructor<const TopoDS_Shape&>();

  mod.method("Init", [](BRepAlgo_NormalProjection& p, const TopoDS_Shape& s) { p.Init(s); });
  mod.method("Add", [](BRepAlgo_NormalProjection& p, const TopoDS_Shape& toProj) { p.Add(toProj); });
  mod.method("SetDefaultParams", [](BRepAlgo_NormalProjection& p) { p.SetDefaultParams(); });
  mod.method("SetMaxDistance", [](BRepAlgo_NormalProjection& p, double maxDist) {
    p.SetMaxDistance(maxDist);
  });
  mod.method("Compute3d", [](BRepAlgo_NormalProjection& p, bool with3d) { p.Compute3d(with3d); });
  mod.method("SetLimit", [](BRepAlgo_NormalProjection& p, bool faceBoundaries) {
    p.SetLimit(faceBoundaries);
  });
  mod.method("Build", [](BRepAlgo_NormalProjection& p) { p.Build(); });
  mod.method("IsDone", [](const BRepAlgo_NormalProjection& p) -> bool { return bool(p.IsDone()); });
  mod.method("Projection", [](const BRepAlgo_NormalProjection& p) -> TopoDS_Shape {
    return p.Projection();
  });
}
