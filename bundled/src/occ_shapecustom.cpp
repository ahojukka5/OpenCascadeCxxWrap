// occ_shapecustom.cpp — 1:1 CxxWrap bindings for ShapeCustom, analytic-surface
// <-> B-spline conversion utilities useful for downstream export
// compatibility (some legacy STEP/IGES/CAM consumers need bounded-degree
// B-splines, or fully-converted analytic surfaces).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <ShapeCustom.hxx>
#include <ShapeCustom_RestrictionParameters.hxx>
#include <TopoDS_Shape.hxx>
#include <GeomAbs_Shape.hxx>

void register_occ_shapecustom(jlcxx::Module& mod) {
  mod.method("ShapeCustom_ConvertToBSpline",
             [](const TopoDS_Shape& s, bool extrMode, bool revolMode, bool offsetMode, bool planeMode) -> TopoDS_Shape {
    return ShapeCustom::ConvertToBSpline(s, extrMode, revolMode, offsetMode, planeMode);
  });

  // ShapeCustom_RestrictionParameters is Standard_Transient-derived and must be passed
  // to BSplineRestriction as a Handle -- same NewX() factory idiom as
  // XCAFDoc_VisMaterial (occ_xcaf_material.cpp): add_type without .constructor<>() (a
  // default ctor collision pitfall discovered there), factory under a distinct name.
  mod.add_type<ShapeCustom_RestrictionParameters>("ShapeCustom_RestrictionParameters");
  mod.method("NewRestrictionParameters", []() -> Handle(ShapeCustom_RestrictionParameters) {
    return new ShapeCustom_RestrictionParameters();
  });

  mod.method("ShapeCustom_BSplineRestriction",
             [](const TopoDS_Shape& s, double tol3d, double tol2d, int maxDegree, int maxNbSegment,
                int continuity3d, int continuity2d, bool degreePriority, bool rational,
                const Handle(ShapeCustom_RestrictionParameters)& params) -> TopoDS_Shape {
    return ShapeCustom::BSplineRestriction(s, tol3d, tol2d, maxDegree, maxNbSegment,
                                           GeomAbs_Shape(continuity3d), GeomAbs_Shape(continuity2d),
                                           degreePriority, rational, params);
  });
}
