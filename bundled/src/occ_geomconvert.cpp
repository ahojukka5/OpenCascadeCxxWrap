// occ_geomconvert.cpp — CxxWrap bindings for GeomConvert (universal
// analytic-to-BSpline conversion) and the concrete Geom_BSplineSurface/
// Geom_BSplineCurve read accessors needed to extract NURBS/Bezier data for
// isogeometric analysis (poles, weights, knots, multiplicities, degree).
//
// New, late-registered TU (registered after occ_geom.cpp, which add_types
// the abstract Geom_Curve/Geom_Surface bases these signatures reference).
//
// Every previous concrete Geom_* subclass factory in this shim (Geom_
// BSplineCurve, Geom_Plane, ...) immediately upcasts to its abstract base
// and is never touched again as the concrete type -- exactly like
// GeomPlate_Surface, this file hits the other case: callers need concrete
// accessor methods (Poles/UKnots/UDegree/...) that don't exist on the
// abstract Geom_Surface/Geom_Curve base, so Geom_BSplineSurface/
// Geom_BSplineCurve (and, for the Bezier-decomposition cross-check,
// Geom_BezierSurface/Geom_BezierCurve) get their own add_type -- no
// constructor chain, since none of these are ever directly constructed by
// Julia, only obtained via GeomConvert or a GeomConvert_*ToBezier* patch
// query.
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"
#include <jlcxx/array.hpp>

#include <GeomConvert.hxx>
#include <GeomConvert_BSplineSurfaceToBezierSurface.hxx>
#include <GeomConvert_BSplineCurveToBezierCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>

#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <gp_Pnt.hxx>
#include <vector>
#include <cstdint>

namespace {
  // Row-major nu*nv flattening (matches occ_geom.cpp's PolesFromFlat2D
  // convention exactly, for round-trip symmetry with bspline_surface's
  // own poles= input).
  std::vector<double> FlattenPoles2D(const TColgp_Array2OfPnt& poles) {
    int nu = poles.NbRows();
    int nv = poles.NbColumns();
    std::vector<double> flat(size_t(nu) * nv * 3);
    for (int i = 1; i <= nu; ++i) {
      for (int j = 1; j <= nv; ++j) {
        const gp_Pnt& p = poles.Value(i, j);
        size_t k = (size_t(i - 1) * nv + (j - 1)) * 3;
        flat[k] = p.X(); flat[k+1] = p.Y(); flat[k+2] = p.Z();
      }
    }
    return flat;
  }
  std::vector<double> FlattenPoles1D(const TColgp_Array1OfPnt& poles) {
    int n = poles.Length();
    std::vector<double> flat(size_t(n) * 3);
    for (int i = 1; i <= n; ++i) {
      const gp_Pnt& p = poles.Value(i);
      size_t k = size_t(i - 1) * 3;
      flat[k] = p.X(); flat[k+1] = p.Y(); flat[k+2] = p.Z();
    }
    return flat;
  }
  std::vector<double> FlattenWeights2D(const TColStd_Array2OfReal* w, int nu, int nv) {
    std::vector<double> flat(size_t(nu) * nv, 1.0);
    if (w == nullptr) return flat;
    for (int i = 1; i <= nu; ++i)
      for (int j = 1; j <= nv; ++j)
        flat[size_t(i - 1) * nv + (j - 1)] = w->Value(i, j);
    return flat;
  }
  std::vector<double> FlattenWeights1D(const TColStd_Array1OfReal* w, int n) {
    std::vector<double> flat(size_t(n), 1.0);
    if (w == nullptr) return flat;
    for (int i = 1; i <= n; ++i) flat[size_t(i - 1)] = w->Value(i);
    return flat;
  }
  std::vector<double> RealsToVec(const TColStd_Array1OfReal& a) {
    std::vector<double> v(a.Length());
    for (int i = 1; i <= a.Length(); ++i) v[i - 1] = a.Value(i);
    return v;
  }
  std::vector<int32_t> IntsToVec(const TColStd_Array1OfInteger& a) {
    std::vector<int32_t> v(a.Length());
    for (int i = 1; i <= a.Length(); ++i) v[i - 1] = int32_t(a.Value(i));
    return v;
  }
}

void register_occ_geomconvert(jlcxx::Module& mod)
{
  mod.add_type<Geom_BSplineSurface>("Geom_BSplineSurface");
  mod.add_type<Geom_BSplineCurve>("Geom_BSplineCurve");
  mod.add_type<Geom_BezierSurface>("Geom_BezierSurface");
  mod.add_type<Geom_BezierCurve>("Geom_BezierCurve");

  // A raw Geom_Surface/Geom_Curve pulled off a Face/Edge via BRep_Tool
  // (e.g. a planar face's Geom_Plane, a straight edge's Geom_Line) is the
  // UNDERLYING, untrimmed analytic geometry -- often geometrically
  // infinite. GeomConvert::SurfaceToBSplineSurface/CurveToBSplineCurve
  // require a bounded/trimmed input ("infinite surface" Standard_Failure
  // otherwise, confirmed empirically). These wrap the surface/curve to
  // its actual finite parametric extent first (the caller supplies the
  // bounds, typically from BRepAdaptor_Surface/Curve's own First/Last
  // U/VParameter -- already bound and used the same way for GeomPlate's
  // range-restricted GeomAdaptor_Curve last round).
  mod.method("Geom_RectangularTrimmedSurface", [](const Handle(Geom_Surface)& s, double u1, double u2,
                                                    double v1, double v2) -> Handle(Geom_Surface) {
    return occ_guard([&]{ return new Geom_RectangularTrimmedSurface(s, u1, u2, v1, v2); });
  });
  mod.method("Geom_TrimmedCurve", [](const Handle(Geom_Curve)& c, double u1, double u2) -> Handle(Geom_Curve) {
    return occ_guard([&]{ return new Geom_TrimmedCurve(c, u1, u2); });
  });

  // ---- Universal analytic -> BSpline conversion ----
  mod.method("GeomConvert_SurfaceToBSplineSurface", [](const Handle(Geom_Surface)& s) -> Handle(Geom_BSplineSurface) {
    return occ_guard([&]{ return GeomConvert::SurfaceToBSplineSurface(s); });
  });
  mod.method("GeomConvert_CurveToBSplineCurve", [](const Handle(Geom_Curve)& c) -> Handle(Geom_BSplineCurve) {
    return occ_guard([&]{ return GeomConvert::CurveToBSplineCurve(c); });
  });

  // ---- Geom_BSplineSurface read accessors ----
  mod.method("NbUPoles", [](const Handle(Geom_BSplineSurface)& s) -> int { return s->NbUPoles(); });
  mod.method("NbVPoles", [](const Handle(Geom_BSplineSurface)& s) -> int { return s->NbVPoles(); });
  mod.method("UDegree", [](const Handle(Geom_BSplineSurface)& s) -> int { return s->UDegree(); });
  mod.method("VDegree", [](const Handle(Geom_BSplineSurface)& s) -> int { return s->VDegree(); });
  mod.method("IsURational", [](const Handle(Geom_BSplineSurface)& s) -> bool { return bool(s->IsURational()); });
  mod.method("IsVRational", [](const Handle(Geom_BSplineSurface)& s) -> bool { return bool(s->IsVRational()); });
  mod.method("Poles", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<double> {
    return FlattenPoles2D(s->Poles());
  });
  mod.method("Weights", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<double> {
    return FlattenWeights2D(s->Weights(), s->NbUPoles(), s->NbVPoles());
  });
  mod.method("UKnots", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<double> { return RealsToVec(s->UKnots()); });
  mod.method("VKnots", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<double> { return RealsToVec(s->VKnots()); });
  mod.method("UMultiplicities", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<int32_t> {
    return IntsToVec(s->UMultiplicities());
  });
  mod.method("VMultiplicities", [](const Handle(Geom_BSplineSurface)& s) -> std::vector<int32_t> {
    return IntsToVec(s->VMultiplicities());
  });

  // ---- Geom_BSplineCurve read accessors ----
  mod.method("NbPoles", [](const Handle(Geom_BSplineCurve)& c) -> int { return c->NbPoles(); });
  mod.method("Degree", [](const Handle(Geom_BSplineCurve)& c) -> int { return c->Degree(); });
  mod.method("IsRational", [](const Handle(Geom_BSplineCurve)& c) -> bool { return bool(c->IsRational()); });
  mod.method("Poles", [](const Handle(Geom_BSplineCurve)& c) -> std::vector<double> { return FlattenPoles1D(c->Poles()); });
  mod.method("Weights", [](const Handle(Geom_BSplineCurve)& c) -> std::vector<double> {
    return FlattenWeights1D(c->Weights(), c->NbPoles());
  });
  mod.method("Knots", [](const Handle(Geom_BSplineCurve)& c) -> std::vector<double> { return RealsToVec(c->Knots()); });
  mod.method("Multiplicities", [](const Handle(Geom_BSplineCurve)& c) -> std::vector<int32_t> {
    return IntsToVec(c->Multiplicities());
  });

  // ---- Native Bezier decomposition (used only as an independent
  // cross-check on the pure-Julia Bezier-extraction math in src/iga.jl --
  // these patches are geometrically-continuous but DOF-duplicating,
  // unlike a true IGA extraction operator, so they are NOT the primary
  // extraction path). ----
  mod.add_type<GeomConvert_BSplineSurfaceToBezierSurface>("GeomConvert_BSplineSurfaceToBezierSurface")
     .constructor<const Handle(Geom_BSplineSurface)&>();
  mod.method("NbUPatches", [](GeomConvert_BSplineSurfaceToBezierSurface& a) -> int { return a.NbUPatches(); });
  mod.method("NbVPatches", [](GeomConvert_BSplineSurfaceToBezierSurface& a) -> int { return a.NbVPatches(); });
  mod.method("Patch", [](GeomConvert_BSplineSurfaceToBezierSurface& a, int uidx, int vidx) -> Handle(Geom_BezierSurface) {
    return a.Patch(uidx, vidx);
  });
  mod.method("NbUPoles", [](const Handle(Geom_BezierSurface)& s) -> int { return s->NbUPoles(); });
  mod.method("NbVPoles", [](const Handle(Geom_BezierSurface)& s) -> int { return s->NbVPoles(); });
  mod.method("Poles", [](const Handle(Geom_BezierSurface)& s) -> std::vector<double> {
    TColgp_Array2OfPnt p(1, s->NbUPoles(), 1, s->NbVPoles());
    s->Poles(p);
    return FlattenPoles2D(p);
  });

  mod.add_type<GeomConvert_BSplineCurveToBezierCurve>("GeomConvert_BSplineCurveToBezierCurve")
     .constructor<const Handle(Geom_BSplineCurve)&>();
  mod.method("NbArcs", [](GeomConvert_BSplineCurveToBezierCurve& a) -> int { return a.NbArcs(); });
  mod.method("Arc", [](GeomConvert_BSplineCurveToBezierCurve& a, int index) -> Handle(Geom_BezierCurve) {
    return a.Arc(index);
  });
  mod.method("NbPoles", [](const Handle(Geom_BezierCurve)& c) -> int { return c->NbPoles(); });
  mod.method("Poles", [](const Handle(Geom_BezierCurve)& c) -> std::vector<double> { return FlattenPoles1D(c->Poles()); });
}
