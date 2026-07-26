// occ_geomfill.cpp — CxxWrap bindings for GeomFill_BSplineCurves (fills a
// BSpline surface from 2-4 contiguous BSpline boundary curves) and
// GeomFill_Pipe (sweeps a section along a path curve). Both are plain
// (non-Transient) classes, bound like GeomAPI_Interpolate in occ_geom.cpp.
//
// GeomFill_Gordon is available in OCCT 8. On OCCT 7.x the compatibility
// adapter below preserves the same Julia-facing API for the common boundary
// network case by constructing a boundary patch from the outer four curves.
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"
#include <jlcxx/jlcxx.hpp>

#include <GeomFill_BSplineCurves.hxx>
#include <GeomFill_FillingStyle.hxx>
#include <GeomFill_Generator.hxx>
#include <GeomFill_Pipe.hxx>
#include <GeomFill_PipeError.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAbs_Shape.hxx>
#include <GeomConvert.hxx>
#include <NCollection_Array1.hxx>
#include <Standard_Failure.hxx>
#include <gp_Pnt.hxx>

#include <stdexcept>
#include <string>
#include <vector>

#if defined(__has_include)
#  if __has_include(<GeomFill_Gordon.hxx>)
#    define MONGE_HAS_GEOMFILL_GORDON 1
#  endif
#endif

#ifdef MONGE_HAS_GEOMFILL_GORDON
#include <GeomFill_Gordon.hxx>
#endif

namespace {
  using GeomCurveVector = std::vector<Handle(Geom_Curve)>;

  NCollection_Array1<Handle(Geom_Curve)> ArrayFromVector(const GeomCurveVector& v) {
    NCollection_Array1<Handle(Geom_Curve)> arr(1, int(v.size()));
    for (size_t i = 0; i < v.size(); ++i) arr.SetValue(int(i + 1), v[i]);
    return arr;
  }

  // GeomFill_BSplineCurves needs Handle(Geom_BSplineCurve) specifically,
  // but BRep_Tool_Curve returns Handle(Geom_Curve). Convert finite analytic
  // and trimmed curves instead of requiring callers to pre-build BSplines.
  Handle(Geom_BSplineCurve) ToBSplineCurve(const Handle(Geom_Curve)& c) {
    if (c.IsNull()) {
      throw std::runtime_error("fill_curves: received a null curve; expected a Geom_BSplineCurve-compatible curve");
    }
    Handle(Geom_BSplineCurve) bs = Handle(Geom_BSplineCurve)::DownCast(c);
    if (!bs.IsNull()) {
      return Handle(Geom_BSplineCurve)::DownCast(bs->Copy());
    }
    try {
      Handle(Geom_BSplineCurve) converted = GeomConvert::CurveToBSplineCurve(c);
      if (converted.IsNull()) {
        throw std::runtime_error("conversion returned a null handle");
      }
      return converted;
    } catch (const Standard_Failure& err) {
      const char* detail = err.GetMessageString();
      throw std::runtime_error(
          std::string("fill_curves: curve cannot be converted to Geom_BSplineCurve")
          + (detail != nullptr && detail[0] != '\0' ? std::string(": ") + detail : std::string()));
    }
  }

  Handle(Geom_BSplineCurve) OrientedFrom(
      const Handle(Geom_Curve)& curve,
      const gp_Pnt& desired_start) {
    Handle(Geom_BSplineCurve) bs = ToBSplineCurve(curve);
    const gp_Pnt first = bs->Value(bs->FirstParameter());
    const gp_Pnt last = bs->Value(bs->LastParameter());
    if (last.Distance(desired_start) < first.Distance(desired_start)) {
      bs->Reverse();
    }
    return bs;
  }

#ifdef MONGE_HAS_GEOMFILL_GORDON
  using GordonAdapter = GeomFill_Gordon;
#else
  class GordonAdapter {
  public:
    void Init(const NCollection_Array1<Handle(Geom_Curve)>& profiles,
              const NCollection_Array1<Handle(Geom_Curve)>& guides,
              double tolerance) {
      profiles_.clear();
      guides_.clear();
      for (int i = profiles.Lower(); i <= profiles.Upper(); ++i) {
        profiles_.push_back(profiles.Value(i));
      }
      for (int i = guides.Lower(); i <= guides.Upper(); ++i) {
        guides_.push_back(guides.Value(i));
      }
      tolerance_ = tolerance;
      done_ = false;
      status_ = 0; // ResultStatus::NotStarted
      surface_.Nullify();
    }

    void Perform() {
      done_ = false;
      status_ = 0;
      surface_.Nullify();
      if (profiles_.size() < 2 || guides_.size() < 2) {
        status_ = 2; // ResultStatus::InvalidInput
        return;
      }

      // The first/last profile and guide curves form the outer boundary of
      // a Gordon network. Internal network curves are an OCCT 8 enhancement;
      // OCCT 7's best equivalent is a four-boundary filling patch.
      Handle(Geom_BSplineCurve) c1 = ToBSplineCurve(profiles_.front());
      const gp_Pnt c1_start = c1->Value(c1->FirstParameter());
      const gp_Pnt c1_end = c1->Value(c1->LastParameter());
      Handle(Geom_BSplineCurve) c2 = OrientedFrom(guides_.back(), c1_end);
      const gp_Pnt c2_end = c2->Value(c2->LastParameter());
      Handle(Geom_BSplineCurve) c3 = OrientedFrom(profiles_.back(), c2_end);
      const gp_Pnt c3_end = c3->Value(c3->LastParameter());
      Handle(Geom_BSplineCurve) c4 = OrientedFrom(guides_.front(), c3_end);
      const gp_Pnt c4_end = c4->Value(c4->LastParameter());

      const double closure_tol = std::max(tolerance_, 1.0e-7);
      if (c4_end.Distance(c1_start) > closure_tol) {
        // Reversing the starting profile can resolve a consistently oriented
        // network whose first curve happened to point the opposite way.
        c1->Reverse();
        const gp_Pnt retry_start = c1->Value(c1->FirstParameter());
        const gp_Pnt retry_end = c1->Value(c1->LastParameter());
        c2 = OrientedFrom(guides_.back(), retry_end);
        c3 = OrientedFrom(profiles_.back(), c2->Value(c2->LastParameter()));
        c4 = OrientedFrom(guides_.front(), c3->Value(c3->LastParameter()));
        if (c4->Value(c4->LastParameter()).Distance(retry_start) > closure_tol) {
          status_ = 5; // ResultStatus::OrderingFailed
          return;
        }
      }

      // CoonsStyle in OCCT 7 requires at least four poles in both directions
      // and rejects straight-line boundaries (two poles). StretchStyle works
      // for both low-degree rectangular networks and richer boundaries.
      GeomFill_BSplineCurves fill(c1, c2, c3, c4, GeomFill_StretchStyle);
      surface_ = fill.Surface();
      done_ = !surface_.IsNull();
      status_ = done_ ? 1 : 18; // Done or ConstructionFailed
    }

    bool IsDone() const { return done_; }
    int Status() const { return status_; }
    Handle(Geom_BSplineSurface) Surface() const { return surface_; }

  private:
    GeomCurveVector profiles_;
    GeomCurveVector guides_;
    double tolerance_ = 1.0e-6;
    bool done_ = false;
    int status_ = 0;
    Handle(Geom_BSplineSurface) surface_;
  };
#endif
}

void register_occ_geomfill(jlcxx::Module& mod)
{
  mod.method("GeomFill_StretchStyle", []() { return int(GeomFill_StretchStyle); });
  mod.method("GeomFill_CoonsStyle",   []() { return int(GeomFill_CoonsStyle); });
  mod.method("GeomFill_CurvedStyle",  []() { return int(GeomFill_CurvedStyle); });

  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(ToBSplineCurve(c1), ToBSplineCurve(c2),
                                   GeomFill_FillingStyle(style));
      return Handle(Geom_Surface)(fill.Surface());
    });
  });
  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           const Handle(Geom_Curve)& c3, int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(ToBSplineCurve(c1), ToBSplineCurve(c2),
                                   ToBSplineCurve(c3), GeomFill_FillingStyle(style));
      return Handle(Geom_Surface)(fill.Surface());
    });
  });
  mod.method("GeomFill_BSplineCurves", [](const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2,
                                           const Handle(Geom_Curve)& c3, const Handle(Geom_Curve)& c4,
                                           int style) -> Handle(Geom_Surface) {
    return occ_guard([&]() -> Handle(Geom_Surface) {
      GeomFill_BSplineCurves fill(ToBSplineCurve(c1), ToBSplineCurve(c2),
                                   ToBSplineCurve(c3), ToBSplineCurve(c4),
                                   GeomFill_FillingStyle(style));
      return Handle(Geom_Surface)(fill.Surface());
    });
  });

  mod.add_type<GeomFill_Generator>("GeomFill_Generator").constructor<>();
  mod.method("AddCurve", [](GeomFill_Generator& g, const Handle(Geom_Curve)& c) {
    occ_guard([&]{ g.AddCurve(c); return 0; });
  });
  mod.method("Perform", [](GeomFill_Generator& g, double tol) {
    occ_guard([&]{ g.Perform(tol); return 0; });
  });
  mod.method("Surface", [](const GeomFill_Generator& g) -> Handle(Geom_Surface) { return g.Surface(); });

  mod.add_type<GeomCurveVector>("GeomCurveArray").constructor<>();
  mod.method("Append", [](GeomCurveVector& v, const Handle(Geom_Curve)& c) { v.push_back(c); });
  mod.method("Extent", [](const GeomCurveVector& v) -> int { return int(v.size()); });

  mod.add_type<GordonAdapter>("GeomFill_Gordon").constructor<>();
  mod.method("GeomFill_Gordon_Init",
             [](GordonAdapter& g, const GeomCurveVector& profiles, const GeomCurveVector& guides, double tol) {
    occ_guard([&]{ g.Init(ArrayFromVector(profiles), ArrayFromVector(guides), tol); return 0; });
  });
  mod.method("Perform", [](GordonAdapter& g) { occ_guard([&]{ g.Perform(); return 0; }); });
  mod.method("IsDone", [](const GordonAdapter& g) -> bool { return bool(g.IsDone()); });
  mod.method("GeomFill_Gordon_Status", [](const GordonAdapter& g) -> int {
    return int(g.Status());
  });
  mod.method("Surface", [](const GordonAdapter& g) -> Handle(Geom_Surface) {
    return Handle(Geom_Surface)(g.Surface());
  });

  mod.add_type<GeomFill_Pipe>("GeomFill_Pipe")
     .constructor<const Handle(Geom_Curve)&, double>()
     .constructor<const Handle(Geom_Curve)&, const Handle(Geom_Curve)&>()
     .constructor<const Handle(Geom_Curve)&, const Handle(Geom_Curve)&, const Handle(Geom_Curve)&>();
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
