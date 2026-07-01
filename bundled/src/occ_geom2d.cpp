// occ_geom2d.cpp — Handle(Geom2d_Curve), GCE2d/GC curve makers for sketch workflows.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>

#include <GCE2d_MakeSegment.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>
#include <GCE2d_MakeCircle.hxx>

#include <GC_MakeSegment.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>

#include <Geom_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Circ.hxx>

void register_occ_geom2d(jlcxx::Module& mod) {
  mod.add_type<gp_Circ2d>("gp_Circ2d").constructor<>()
     .constructor<const gp_Ax2d&, double>();
  mod.add_type<Geom2d_Curve>("Geom2d_Curve");
  mod.method("Value2d", [](const Handle(Geom2d_Curve)& c, double u) -> gp_Pnt2d { return c->Value(u); });
  mod.method("FirstParameter2d", [](const Handle(Geom2d_Curve)& c) -> double { return c->FirstParameter(); });
  mod.method("LastParameter2d",  [](const Handle(Geom2d_Curve)& c) -> double { return c->LastParameter(); });
  mod.method("IsClosed2d",   [](const Handle(Geom2d_Curve)& c) -> bool { return bool(c->IsClosed()); });
  mod.method("IsPeriodic2d", [](const Handle(Geom2d_Curve)& c) -> bool { return bool(c->IsPeriodic()); });

  mod.method("Geom2d_Line", [](const gp_Ax2d& ax) -> Handle(Geom2d_Curve) {
    return new Geom2d_Line(ax);
  });
  mod.method("Geom2d_Circle", [](const gp_Ax2d& ax, double r) -> Handle(Geom2d_Curve) {
    return new Geom2d_Circle(ax, r);
  });
  mod.method("Geom2d_Ellipse", [](const gp_Ax2d& ax, double major, double minor) -> Handle(Geom2d_Curve) {
    return new Geom2d_Ellipse(ax, major, minor);
  });

  mod.add_type<GCE2d_MakeSegment>("GCE2d_MakeSegment")
     .constructor<const gp_Pnt2d&, const gp_Pnt2d&>();
  mod.method("Value2d", [](GCE2d_MakeSegment& m) -> Handle(Geom2d_Curve) { return m.Value(); });
  mod.method("IsDone2d", [](const GCE2d_MakeSegment& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<GCE2d_MakeArcOfCircle>("GCE2d_MakeArcOfCircle")
     .constructor<const gp_Circ2d&, double, double, bool>()
     .constructor<const gp_Pnt2d&, const gp_Pnt2d&, const gp_Pnt2d>();
  mod.method("Value2d", [](GCE2d_MakeArcOfCircle& m) -> Handle(Geom2d_Curve) { return m.Value(); });
  mod.method("IsDone2d", [](const GCE2d_MakeArcOfCircle& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<GCE2d_MakeCircle>("GCE2d_MakeCircle")
     .constructor<const gp_Ax2d&, double>()
     .constructor<const gp_Circ2d&>();
  mod.method("Value2d", [](GCE2d_MakeCircle& m) -> Handle(Geom2d_Curve) { return m.Value(); });
  mod.method("IsDone2d", [](const GCE2d_MakeCircle& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<GC_MakeSegment>("GC_MakeSegment")
     .constructor<const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](GC_MakeSegment& m) -> Handle(Geom_Curve) { return m.Value(); });
  mod.method("IsDone", [](const GC_MakeSegment& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<GC_MakeArcOfCircle>("GC_MakeArcOfCircle")
     .constructor<const gp_Circ&, double, double, bool>()
     .constructor<const gp_Pnt&, const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](GC_MakeArcOfCircle& m) -> Handle(Geom_Curve) { return m.Value(); });
  mod.method("IsDone", [](const GC_MakeArcOfCircle& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<GC_MakeCircle>("GC_MakeCircle")
     .constructor<const gp_Ax2&, double>()
     .constructor<const gp_Circ&>();
  mod.method("Value", [](GC_MakeCircle& m) -> Handle(Geom_Curve) { return m.Value(); });
  mod.method("IsDone", [](const GC_MakeCircle& m) -> bool { return bool(m.IsDone()); });
}
