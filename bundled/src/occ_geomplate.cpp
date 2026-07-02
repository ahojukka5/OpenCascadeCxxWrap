// occ_geomplate.cpp — CxxWrap bindings for GeomPlate_BuildPlateSurface, OCCT's
// energy-minimizing "plate" surface fitter: deform an optional initial
// surface to satisfy independently-weighted curve/point constraints, each
// with its own G0/G1/G2 continuity order -- a different algorithm from
// BRepOffsetAPI_MakeFilling (occ_fill.cpp), useful when per-constraint
// tangency/curvature control is needed.
//
// New, late-registered TU (registered after register_occ_geom, which
// add_type's Geom_Curve/Geom_Surface). Five distinct binding shapes appear
// here, each following an established precedent:
//   - Adaptor3d_Curve: Standard_Transient-derived ABSTRACT base, never
//     directly constructed -- add_type with no constructor, exactly like
//     Geom_Curve/Geom_Surface in occ_geom.cpp.
//   - GeomAdaptor_Curve: concrete subclass; its factory is named
//     "GeomAdaptor_Curve", distinct from the abstract base name
//     "Adaptor3d_Curve" it upcasts to, so there is no collision with
//     CxxWrap's own (::Type{Adaptor3d_Curve})() slot (mirrors the
//     Geom_BSplineCurve/Geom_Curve relationship).
//   - GeomPlate_CurveConstraint / GeomPlate_PointConstraint: concrete,
//     Standard_Transient-derived, and directly constructed by name --
//     the HLRBRep_Algo situation (occ_hlr.cpp): add_type with NO
//     constructor, a differently-named factory (new_curve_constraint /
//     new_point_constraint) returning the Handle, to avoid the same-name
//     collision with CxxWrap's own calling convention.
//   - GeomPlate_Surface: Standard_Transient-derived (via Geom_Surface) but
//     never directly constructed by Julia -- only received from
//     BuildPlateSurface::Surface() and passed into MakeApprox's
//     constructor. Pure pass-through, like BRepTools_History
//     (occ_history.cpp): add_type with no constructor at all.
//   - GeomPlate_BuildPlateSurface / GeomPlate_MakeApprox: plain
//     (non-Transient) classes, bound like BRepOffsetAPI_MakeFilling
//     (occ_fill.cpp).
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"

#include <Adaptor3d_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomPlate_CurveConstraint.hxx>
#include <GeomPlate_PointConstraint.hxx>
#include <GeomPlate_Surface.hxx>
#include <GeomPlate_BuildPlateSurface.hxx>
#include <GeomPlate_MakeApprox.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomAbs_Shape.hxx>
#include <gp_Pnt.hxx>

void register_occ_geomplate(jlcxx::Module& mod)
{
  mod.add_type<Adaptor3d_Curve>("Adaptor3d_Curve");
  mod.method("GeomAdaptor_Curve", [](const Handle(Geom_Curve)& c) -> Handle(Adaptor3d_Curve) {
    return new GeomAdaptor_Curve(c);
  });
  // Range-restricted overload -- required for unbounded underlying curves
  // (e.g. a straight edge's Geom_Line has an infinite natural parameter
  // domain) so the adaptor only sees the edge's actual [first, last]
  // range; without it GeomPlate_BuildPlateSurface::Perform() samples
  // points across the unbounded range and fails downstream (confirmed
  // empirically: "Geom_RectangularTrimmedSurface::U parameters out of
  // range" on a straight-line boundary edge before this overload existed).
  mod.method("GeomAdaptor_Curve", [](const Handle(Geom_Curve)& c, double first, double last) -> Handle(Adaptor3d_Curve) {
    return occ_guard([&]{ return new GeomAdaptor_Curve(c, first, last); });
  });

  // Named new_curve_constraint(), not GeomPlate_CurveConstraint() --
  // add_type<GeomPlate_CurveConstraint>(...) itself reserves that exact
  // string for CxxWrap's own (::Type{T})() calling convention regardless
  // of whether .constructor<>() is chained (confirmed empirically for
  // HLRBRep_Algo in occ_hlr.cpp). Order must be -1, 0, 1, or 2 --
  // Standard_ConstructionError otherwise per the header, a real
  // Standard_Failure path, so guarded.
  mod.add_type<GeomPlate_CurveConstraint>("GeomPlate_CurveConstraint");
  mod.method("new_curve_constraint", [](const Handle(Adaptor3d_Curve)& boundary, int order, int npt,
                                         double tolDist, double tolAng, double tolCurv) -> Handle(GeomPlate_CurveConstraint) {
    return occ_guard([&]{ return new GeomPlate_CurveConstraint(boundary, order, npt, tolDist, tolAng, tolCurv); });
  });

  // Same naming rationale as above. Two overloads mirroring the two OCCT
  // constructors: a free 3D point (Order restricted to -1/0 by the
  // header) and a point pinned to (u, v) on a support surface (full
  // -1..2 range) -- the latter bound for completeness even though the
  // Julia layer (surfaces.jl) only exposes the free-point form for now.
  mod.add_type<GeomPlate_PointConstraint>("GeomPlate_PointConstraint");
  mod.method("new_point_constraint", [](const gp_Pnt& pt, int order, double tolDist) -> Handle(GeomPlate_PointConstraint) {
    return occ_guard([&]{ return new GeomPlate_PointConstraint(pt, order, tolDist); });
  });
  mod.method("new_point_constraint", [](double u, double v, const Handle(Geom_Surface)& surf, int order,
                                         double tolDist, double tolAng, double tolCurv) -> Handle(GeomPlate_PointConstraint) {
    return occ_guard([&]{ return new GeomPlate_PointConstraint(u, v, surf, order, tolDist, tolAng, tolCurv); });
  });

  // Pure pass-through: never constructed by Julia, only received from
  // GeomPlate_BuildPlateSurface::Surface() and passed into
  // GeomPlate_MakeApprox's constructor below.
  mod.add_type<GeomPlate_Surface>("GeomPlate_Surface");

  mod.add_type<GeomPlate_BuildPlateSurface>("GeomPlate_BuildPlateSurface")
     .constructor<const Handle(Geom_Surface)&, int, int, int, double, double, double, double, bool>()
     .constructor<int, int, int, double, double, double, double, bool>();

  mod.method("LoadInitSurface", [](GeomPlate_BuildPlateSurface& b, const Handle(Geom_Surface)& surf) {
    b.LoadInitSurface(surf);
  });
  mod.method("Add", [](GeomPlate_BuildPlateSurface& b, const Handle(GeomPlate_CurveConstraint)& c) {
    b.Add(c);
  });
  mod.method("Add", [](GeomPlate_BuildPlateSurface& b, const Handle(GeomPlate_PointConstraint)& c) {
    b.Add(c);
  });
  mod.method("SetNbBounds", [](GeomPlate_BuildPlateSurface& b, int n) { b.SetNbBounds(n); });
  // Perform() drives an iterative energy-minimization solve over the given
  // constraint network -- degenerate/incompatible constraints are a real
  // failure path here, same caution as BRepOffsetAPI_MakeFilling::Build().
  mod.method("Perform", [](GeomPlate_BuildPlateSurface& b) {
    occ_guard([&]{ b.Perform(); return 0; });
  });
  mod.method("IsDone", [](const GeomPlate_BuildPlateSurface& b) -> bool { return bool(b.IsDone()); });
  mod.method("Surface", [](const GeomPlate_BuildPlateSurface& b) -> Handle(GeomPlate_Surface) { return b.Surface(); });
  mod.method("G0Error", [](const GeomPlate_BuildPlateSurface& b) -> double { return b.G0Error(); });
  mod.method("G1Error", [](const GeomPlate_BuildPlateSurface& b) -> double { return b.G1Error(); });

  mod.add_type<GeomPlate_MakeApprox>("GeomPlate_MakeApprox")
     .constructor([](const Handle(GeomPlate_Surface)& surfPlate, double tol3d, int nbmax, int dgmax,
                      double dmax, int critOrder, int continuity, double enlargeCoeff) -> GeomPlate_MakeApprox* {
       return occ_guard([&]{
         return new GeomPlate_MakeApprox(surfPlate, tol3d, nbmax, dgmax, dmax, critOrder,
                                          GeomAbs_Shape(continuity), enlargeCoeff);
       });
     });
  // Geom_BSplineSurface itself is never separately add_type'd in this shim
  // (only the abstract Geom_Surface base is, per occ_geom.cpp) -- upcast
  // here exactly like every Geom_* concrete-surface factory already does.
  mod.method("Surface", [](const GeomPlate_MakeApprox& a) -> Handle(Geom_Surface) { return a.Surface(); });
  mod.method("ApproxError", [](const GeomPlate_MakeApprox& a) -> double { return a.ApproxError(); });
}
