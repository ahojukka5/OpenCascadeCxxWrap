// occ_fill.cpp — CxxWrap bindings for BRepOffsetAPI_MakeFilling, OCCT's
// N-sided/curve-network surface builder (energy-minimizing deformation of an
// optional initial surface to satisfy edge/face/point constraints).
//
// New, late-registered TU (registered after register_occ_geom and
// register_occ_builders) -- no new Handle(X) type is introduced here, so
// occ_handle_traits.hpp is not needed: constraints are TopoDS_Edge/
// TopoDS_Face/gp_Pnt, all already bound by the time this registers.
#include <jlcxx/jlcxx.hpp>
#include "occ_exception.hpp"

#include <BRepOffsetAPI_MakeFilling.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <GeomAbs_Shape.hxx>
#include <gp_Pnt.hxx>

void register_occ_fill(jlcxx::Module& mod)
{
  mod.add_type<BRepOffsetAPI_MakeFilling>("BRepOffsetAPI_MakeFilling")
     .constructor<>()
     .constructor<int, int, int, bool, double, double, double, double, int, int>();

  mod.method("LoadInitSurface", [](BRepOffsetAPI_MakeFilling& f, const TopoDS_Face& surf) {
    f.LoadInitSurface(surf);
  });

  // Edge constraint, order relative to the edge's own first face.
  mod.method("Add", [](BRepOffsetAPI_MakeFilling& f, const TopoDS_Edge& constr,
                        int order, bool isBound) -> int {
    return f.Add(constr, GeomAbs_Shape(order), isBound);
  });
  // Edge constraint, order relative to an explicit support face.
  mod.method("Add", [](BRepOffsetAPI_MakeFilling& f, const TopoDS_Edge& constr,
                        const TopoDS_Face& support, int order, bool isBound) -> int {
    return f.Add(constr, support, GeomAbs_Shape(order), isBound);
  });
  // Free face constraint -- bounding edge is auto-recomputed.
  mod.method("Add", [](BRepOffsetAPI_MakeFilling& f, const TopoDS_Face& support, int order) -> int {
    return f.Add(support, GeomAbs_Shape(order));
  });
  // Punctual constraint.
  mod.method("Add", [](BRepOffsetAPI_MakeFilling& f, const gp_Pnt& pt) -> int {
    return f.Add(pt);
  });
  // Punctual constraint pinned to a (u, v) location on a support face.
  mod.method("Add", [](BRepOffsetAPI_MakeFilling& f, double u, double v,
                        const TopoDS_Face& support, int order) -> int {
    return f.Add(u, v, support, GeomAbs_Shape(order));
  });

  // Build() drives an iterative energy-minimization solve over the given
  // constraint network -- degenerate/incompatible constraints are a real
  // Standard_Failure path here (unlike the more mechanical builders in
  // occ_sweep.cpp), so this is guarded like occ_builders.cpp's primitives.
  mod.method("Build", [](BRepOffsetAPI_MakeFilling& f) {
    occ_guard([&]{ f.Build(); return 0; });
  });
  mod.method("IsDone", [](const BRepOffsetAPI_MakeFilling& f) -> bool { return bool(f.IsDone()); });
  mod.method("Shape",  [](BRepOffsetAPI_MakeFilling& f) -> TopoDS_Shape {
    return occ_guard([&]{ return f.Shape(); });
  });
  mod.method("Generated", [](BRepOffsetAPI_MakeFilling& f, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return f.Generated(s);
  });
  mod.method("G0Error", [](const BRepOffsetAPI_MakeFilling& f) -> double { return f.G0Error(); });
  mod.method("G1Error", [](const BRepOffsetAPI_MakeFilling& f) -> double { return f.G1Error(); });
}
