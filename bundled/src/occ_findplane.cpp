// occ_findplane.cpp — 1:1 CxxWrap binding for BRepBuilderAPI_FindPlane, a
// planarity check: finds the single plane containing all of a shape's edges
// (within tolerance), if one exists -- useful as a pre-check before
// sketch/profile operations that require a planar wire/face.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepBuilderAPI_FindPlane.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_findplane(jlcxx::Module& mod) {
  mod.add_type<BRepBuilderAPI_FindPlane>("BRepBuilderAPI_FindPlane")
     .constructor<>()
     .constructor<const TopoDS_Shape&, double>();

  mod.method("Init", [](BRepBuilderAPI_FindPlane& fp, const TopoDS_Shape& s, double tol) {
    fp.Init(s, tol);
  });
  mod.method("Found", [](const BRepBuilderAPI_FindPlane& fp) -> bool { return bool(fp.Found()); });
  // Geom_Plane is never add_type'd in this codebase (per occ_geom.cpp's Geom_Plane factory
  // pattern) -- always upcast to the already-bound Handle(Geom_Surface) base.
  mod.method("Plane", [](const BRepBuilderAPI_FindPlane& fp) -> Handle(Geom_Surface) {
    return fp.Plane();
  });
}
