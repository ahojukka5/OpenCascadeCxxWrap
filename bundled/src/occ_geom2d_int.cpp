// occ_geom2d_int.cpp — 1:1 CxxWrap bindings closing the 2D/3D curve
// intersection/projection gap at the GeomAPI level:
//   - Geom2dAPI_InterCurveCurve — 2D curve-curve intersection/self-intersection.
//   - Geom2dAPI_ProjectPointOnCurve — 2D point-to-curve projection.
//   - GeomAPI_IntCS — 3D curve-surface intersection.
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>

void register_occ_geom2d_int(jlcxx::Module& mod) {
  // Geom2dAPI_InterCurveCurve
  mod.add_type<Geom2dAPI_InterCurveCurve>("Geom2dAPI_InterCurveCurve").constructor<>();
  mod.method("Init", [](Geom2dAPI_InterCurveCurve& icc, const Handle(Geom2d_Curve)& c1,
                         const Handle(Geom2d_Curve)& c2, double tol) {
    occ_guard([&]{ icc.Init(c1, c2, tol); return 0; });
  });
  mod.method("Init", [](Geom2dAPI_InterCurveCurve& icc, const Handle(Geom2d_Curve)& c1, double tol) {
    occ_guard([&]{ icc.Init(c1, tol); return 0; });
  });
  mod.method("NbPoints2d", [](const Geom2dAPI_InterCurveCurve& icc) -> int { return icc.NbPoints(); });
  mod.method("Point2d", [](const Geom2dAPI_InterCurveCurve& icc, int i) -> gp_Pnt2d {
    return occ_guard([&]{ return icc.Point(i); });
  });
  mod.method("NbSegments2d", [](const Geom2dAPI_InterCurveCurve& icc) -> int { return icc.NbSegments(); });

  // Geom2dAPI_ProjectPointOnCurve
  mod.add_type<Geom2dAPI_ProjectPointOnCurve>("Geom2dAPI_ProjectPointOnCurve").constructor<>();
  mod.method("Init", [](Geom2dAPI_ProjectPointOnCurve& p, const gp_Pnt2d& pt, const Handle(Geom2d_Curve)& c) {
    occ_guard([&]{ p.Init(pt, c); return 0; });
  });
  mod.method("NbPoints2d", [](const Geom2dAPI_ProjectPointOnCurve& p) -> int { return p.NbPoints(); });
  mod.method("Point2d", [](const Geom2dAPI_ProjectPointOnCurve& p, int i) -> gp_Pnt2d {
    return occ_guard([&]{ return p.Point(i); });
  });
  mod.method("Parameter2d", [](const Geom2dAPI_ProjectPointOnCurve& p, int i) -> double {
    return occ_guard([&]{ return p.Parameter(i); });
  });
  mod.method("Distance2d", [](const Geom2dAPI_ProjectPointOnCurve& p, int i) -> double {
    return occ_guard([&]{ return p.Distance(i); });
  });
  mod.method("NearestPoint2d", [](const Geom2dAPI_ProjectPointOnCurve& p) -> gp_Pnt2d {
    return occ_guard([&]{ return p.NearestPoint(); });
  });
  mod.method("LowerDistanceParameter2d", [](const Geom2dAPI_ProjectPointOnCurve& p) -> double {
    return occ_guard([&]{ return p.LowerDistanceParameter(); });
  });
  mod.method("LowerDistance2d", [](const Geom2dAPI_ProjectPointOnCurve& p) -> double {
    return occ_guard([&]{ return p.LowerDistance(); });
  });

  // GeomAPI_IntCS — 3D curve-surface intersection.
  mod.add_type<GeomAPI_IntCS>("GeomAPI_IntCS").constructor<>();
  mod.method("Perform", [](GeomAPI_IntCS& ics, const Handle(Geom_Curve)& c, const Handle(Geom_Surface)& s) {
    occ_guard([&]{ ics.Perform(c, s); return 0; });
  });
  mod.method("IsDone", [](const GeomAPI_IntCS& ics) -> bool { return bool(ics.IsDone()); });
  mod.method("NbPoints", [](const GeomAPI_IntCS& ics) -> int { return ics.NbPoints(); });
  mod.method("Point", [](const GeomAPI_IntCS& ics, int i) -> gp_Pnt {
    return occ_guard([&]{ return ics.Point(i); });
  });
  mod.method("NbSegments", [](const GeomAPI_IntCS& ics) -> int { return ics.NbSegments(); });
}
