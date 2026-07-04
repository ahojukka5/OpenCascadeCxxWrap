// occ_adaptor.cpp — 1:1 CxxWrap bindings for BRepAdaptor_Curve, BRepAdaptor_Surface,
// and BRepAdaptor_CompCurve.
#include <jlcxx/jlcxx.hpp>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <GeomAbs_SurfaceType.hxx>

void register_occ_adaptor(jlcxx::Module& mod) {
  mod.add_type<BRepAdaptor_Curve>("BRepAdaptor_Curve")
     .constructor<>()
     .constructor<const TopoDS_Edge&>()
     .constructor<const TopoDS_Edge&, const TopoDS_Face&>();

  mod.add_type<BRepAdaptor_Surface>("BRepAdaptor_Surface")
     .constructor<>()
     .constructor<const TopoDS_Face&>();

  mod.add_type<BRepAdaptor_CompCurve>("BRepAdaptor_CompCurve")
     .constructor<>()
     .constructor<const TopoDS_Wire&>()
     .constructor<const TopoDS_Wire&, bool>()
     .constructor<const TopoDS_Wire&, bool, double, double, double>();

  // BRepAdaptor_Curve methods
  mod.method("FirstParameter", [](const BRepAdaptor_Curve& c) -> double {
    return c.FirstParameter();
  });
  mod.method("LastParameter", [](const BRepAdaptor_Curve& c) -> double {
    return c.LastParameter();
  });
  mod.method("Tolerance", [](const BRepAdaptor_Curve& c) -> double {
    return c.Tolerance();
  });
  mod.method("IsClosed", [](const BRepAdaptor_Curve& c) -> bool {
    return c.IsClosed();
  });
  mod.method("IsPeriodic", [](const BRepAdaptor_Curve& c) -> bool {
    return c.IsPeriodic();
  });
  mod.method("Value", [](const BRepAdaptor_Curve& c, double t) -> gp_Pnt {
    return c.Value(t);
  });
  mod.method("D0", [](const BRepAdaptor_Curve& c, double t, gp_Pnt& p) {
    c.D0(t, p);
  });
  mod.method("D1", [](const BRepAdaptor_Curve& c, double t, gp_Pnt& p, gp_Vec& v) {
    c.D1(t, p, v);
  });
  mod.method("D2", [](const BRepAdaptor_Curve& c, double t, gp_Pnt& p, gp_Vec& v1, gp_Vec& v2) {
    c.D2(t, p, v1, v2);
  });

  // BRepAdaptor_Surface methods
  mod.method("FirstUParameter", [](const BRepAdaptor_Surface& s) -> double {
    return s.FirstUParameter();
  });
  mod.method("LastUParameter", [](const BRepAdaptor_Surface& s) -> double {
    return s.LastUParameter();
  });
  mod.method("FirstVParameter", [](const BRepAdaptor_Surface& s) -> double {
    return s.FirstVParameter();
  });
  mod.method("LastVParameter", [](const BRepAdaptor_Surface& s) -> double {
    return s.LastVParameter();
  });
  mod.method("Tolerance", [](const BRepAdaptor_Surface& s) -> double {
    return s.Tolerance();
  });
  mod.method("Value", [](const BRepAdaptor_Surface& s, double u, double v) -> gp_Pnt {
    return s.Value(u, v);
  });
  mod.method("D0", [](const BRepAdaptor_Surface& s, double u, double v, gp_Pnt& p) {
    s.D0(u, v, p);
  });
  mod.method("D1", [](const BRepAdaptor_Surface& s, double u, double v, gp_Pnt& p, gp_Vec& du, gp_Vec& dv) {
    s.D1(u, v, p, du, dv);
  });
  mod.method("Normal", [](const BRepAdaptor_Surface& s, double u, double v) -> gp_Dir {
    gp_Pnt p; gp_Vec d1u, d1v;
    s.D1(u, v, p, d1u, d1v);
    return gp_Dir(d1u.Crossed(d1v));
  });

  // Typed surface classification -- previously only inferred heuristically
  // on the Julian side (planar-vs-not by normal variation at UV corners),
  // this exposes OCCT's own honest classification and axis/radius extraction.
  mod.method("GetType", [](const BRepAdaptor_Surface& s) -> int { return int(s.GetType()); });
  mod.method("Plane",    [](const BRepAdaptor_Surface& s) -> gp_Pln { return s.Plane(); });
  mod.method("Cylinder", [](const BRepAdaptor_Surface& s) -> gp_Cylinder { return s.Cylinder(); });
  mod.method("Cone",     [](const BRepAdaptor_Surface& s) -> gp_Cone { return s.Cone(); });
  mod.method("Sphere",   [](const BRepAdaptor_Surface& s) -> gp_Sphere { return s.Sphere(); });
  mod.method("Torus",    [](const BRepAdaptor_Surface& s) -> gp_Torus { return s.Torus(); });

  mod.method("GeomAbs_Plane",              []() { return int(GeomAbs_Plane); });
  mod.method("GeomAbs_Cylinder",           []() { return int(GeomAbs_Cylinder); });
  mod.method("GeomAbs_Cone",               []() { return int(GeomAbs_Cone); });
  mod.method("GeomAbs_Sphere",             []() { return int(GeomAbs_Sphere); });
  mod.method("GeomAbs_Torus",              []() { return int(GeomAbs_Torus); });
  mod.method("GeomAbs_BezierSurface",      []() { return int(GeomAbs_BezierSurface); });
  mod.method("GeomAbs_BSplineSurface",     []() { return int(GeomAbs_BSplineSurface); });
  mod.method("GeomAbs_SurfaceOfRevolution",[]() { return int(GeomAbs_SurfaceOfRevolution); });
  mod.method("GeomAbs_SurfaceOfExtrusion", []() { return int(GeomAbs_SurfaceOfExtrusion); });
  mod.method("GeomAbs_OffsetSurface",      []() { return int(GeomAbs_OffsetSurface); });
  mod.method("GeomAbs_OtherSurface",       []() { return int(GeomAbs_OtherSurface); });

  // BRepAdaptor_CompCurve methods -- treats a whole (non-periodic) multi-edge
  // wire as one continuous parametrized curve. Value/D0/D1/D2 are inherited,
  // non-virtual Adaptor3d_Curve base members (they just call EvalD0/EvalD1/...
  // underneath) so the exact same overload names already bound for
  // BRepAdaptor_Curve above dispatch correctly here too.
  mod.method("Initialize", [](BRepAdaptor_CompCurve& c, const TopoDS_Wire& w, bool knotByCurvilinearAbcissa) {
    c.Initialize(w, knotByCurvilinearAbcissa);
  });
  mod.method("Initialize", [](BRepAdaptor_CompCurve& c, const TopoDS_Wire& w, bool knotByCurvilinearAbcissa,
                               double first, double last, double tol) {
    c.Initialize(w, knotByCurvilinearAbcissa, first, last, tol);
  });
  mod.method("Wire", [](const BRepAdaptor_CompCurve& c) -> TopoDS_Wire { return c.Wire(); });
  mod.method("Edge", [](const BRepAdaptor_CompCurve& c, double u, TopoDS_Edge& e, double& uOnE) {
    c.Edge(u, e, uOnE);
  });
  mod.method("FirstParameter", [](const BRepAdaptor_CompCurve& c) -> double { return c.FirstParameter(); });
  mod.method("LastParameter", [](const BRepAdaptor_CompCurve& c) -> double { return c.LastParameter(); });
  mod.method("IsClosed", [](const BRepAdaptor_CompCurve& c) -> bool { return c.IsClosed(); });
  mod.method("IsPeriodic", [](const BRepAdaptor_CompCurve& c) -> bool { return c.IsPeriodic(); });
  mod.method("Value", [](const BRepAdaptor_CompCurve& c, double t) -> gp_Pnt { return c.Value(t); });
  mod.method("D0", [](const BRepAdaptor_CompCurve& c, double t, gp_Pnt& p) { c.D0(t, p); });
  mod.method("D1", [](const BRepAdaptor_CompCurve& c, double t, gp_Pnt& p, gp_Vec& v) { c.D1(t, p, v); });
  mod.method("D2", [](const BRepAdaptor_CompCurve& c, double t, gp_Pnt& p, gp_Vec& v1, gp_Vec& v2) {
    c.D2(t, p, v1, v2);
  });
  mod.method("GetType", [](const BRepAdaptor_CompCurve& c) -> int { return int(c.GetType()); });
  mod.method("Continuity", [](const BRepAdaptor_CompCurve& c) -> int { return int(c.Continuity()); });
  mod.method("Degree", [](const BRepAdaptor_CompCurve& c) -> int { return c.Degree(); });
  mod.method("IsRational", [](const BRepAdaptor_CompCurve& c) -> bool { return c.IsRational(); });
}
