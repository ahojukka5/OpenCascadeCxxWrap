// occ_geomfill.cpp — CxxWrap bindings for GeomFill_BSplineCurves (fills a
// BSpline surface from 2-4 contiguous BSpline boundary curves) and
// GeomFill_Pipe (sweeps a section along a path curve). Both are plain
// (non-Transient) classes, bound like GeomAPI_Interpolate in occ_geom.cpp.
//
// New, late-registered TU (registered after register_occ_geom, which
// add_types Geom_Curve/Geom_Surface/Geom_BSplineCurve that this file's
// signatures reference).
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"
#include <jlcxx/jlcxx.hpp>

#include <GeomFill_BSplineCurves.hxx>
#include <GeomFill_FillingStyle.hxx>
#include <GeomFill_Pipe.hxx>
#include <GeomFill_PipeError.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAbs_Shape.hxx>

namespace {
  // GeomFill_BSplineCurves needs Handle(Geom_BSplineCurve) specifically,
  // but BRep_Tool_Curve (what Julia callers extract an edge's curve with)
  // returns Handle(Geom_Curve) -- most sketch edges are lines/arcs, not
  // BSplines, so this downcast failing is the single most common misuse
  // case and needs a message naming it explicitly, not OCCT's generic
  // construction-error text.
  Handle(Geom_BSplineCurve) RequireBSplineCurve(const Handle(Geom_Curve)& c) {
    Handle(Geom_BSplineCurve) bs = Handle(Geom_BSplineCurve)::DownCast(c);
    if (bs.IsNull()) {
      throw std::runtime_error("fill_curves: edge's underlying curve is not a Geom_BSplineCurve "
                                "(most sketch lines/arcs aren't -- build the boundary with "
                                "interpolate_curve to get a BSpline-backed edge)");
    }
    return bs;
  }
}

void register_occ_geomfill(jlcxx::Module& mod)
{
  mod.method("GeomFill_StretchStyle", []() { return int(GeomFill_StretchStyle); });
  mod.method("GeomFill_CoonsStyle",   []() { return int(GeomFill_CoonsStyle); });
  mod.method("GeomFill_CurvedStyle",  []() { return int(GeomFill_CurvedStyle); });

  // GeomFill_BSplineCurves is a helper object, not itself a Geom_Surface --
  // construct it on the stack (matching GeomAPI_PointsToBSpline's factory
  // in occ_geom.cpp) and extract .Surface() rather than treating `new
  // GeomFill_BSplineCurves(...)` as directly convertible to Handle(Geom_Surface).
  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(RequireBSplineCurve(c1), RequireBSplineCurve(c2),
                                   GeomFill_FillingStyle(style));
      return fill.Surface();
    });
  });
  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           const Handle(Geom_Curve)& c3, int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(RequireBSplineCurve(c1), RequireBSplineCurve(c2),
                                   RequireBSplineCurve(c3), GeomFill_FillingStyle(style));
      return fill.Surface();
    });
  });
  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           const Handle(Geom_Curve)& c3, const Handle(Geom_Curve)& c4,
                                           int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(RequireBSplineCurve(c1), RequireBSplineCurve(c2),
                                   RequireBSplineCurve(c3), RequireBSplineCurve(c4),
                                   GeomFill_FillingStyle(style));
      return fill.Surface();
    });
  });

  mod.add_type<GeomFill_Pipe>("GeomFill_Pipe")
     .constructor<const Handle(Geom_Curve)&, double>()                                    // constant radius
     .constructor<const Handle(Geom_Curve)&, const Handle(Geom_Curve)&>()                 // constant section
     .constructor<const Handle(Geom_Curve)&, const Handle(Geom_Curve)&, const Handle(Geom_Curve)&>(); // evolving section
  mod.method("Perform", [](GeomFill_Pipe& p, double tol, bool polynomial, int continuity,
                            int maxDegree, int nbMaxSegment) {
    occ_guard([&]{
      p.Perform(tol, polynomial, GeomAbs_Shape(continuity), maxDegree, nbMaxSegment);
      return 0;
    });
  });
  mod.method("IsDone", [](const GeomFill_Pipe& p) -> bool { return bool(p.IsDone()); });
  mod.method("Surface", [](const GeomFill_Pipe& p) -> Handle(Geom_Surface) { return p.Surface(); });
  mod.method("GetStatus", [](const GeomFill_Pipe& p) -> int { return int(p.GetStatus()); });
  mod.method("ErrorOnSurf", [](const GeomFill_Pipe& p) -> double { return p.ErrorOnSurf(); });

  mod.method("GeomFill_PipeOk",                 []() { return int(GeomFill_PipeOk); });
  mod.method("GeomFill_PipeNotOk",              []() { return int(GeomFill_PipeNotOk); });
  mod.method("GeomFill_PlaneNotIntersectGuide", []() { return int(GeomFill_PlaneNotIntersectGuide); });
  mod.method("GeomFill_ImpossibleContact",      []() { return int(GeomFill_ImpossibleContact); });
}
