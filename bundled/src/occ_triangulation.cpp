// occ_triangulation.cpp — 1:1 CxxWrap bindings for Poly_Triangulation,
// TopLoc_Location, and BRep_Tool::Triangulation, giving in-memory access to
// a face's mesh without going through a temp-file mesh format (STL/OBJ) --
// the Julian layer previously always round-tripped through a temp STL file
// even for in-process uses like export_glb. Points are returned in the
// face's OWN local reference frame (not yet placed); TopLoc_Location's
// Transformation() lets the caller compose the placement, matching the
// strict-1:1 philosophy (Monge.OCC mirrors OCCT; the Julian layer owns
// ergonomics like auto-applying the transform).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

namespace jlcxx {
  template<> struct IsMirroredType<TopLoc_Location> : std::false_type { };
}

void register_occ_triangulation(jlcxx::Module& mod) {
  // The gp_Trsf-taking constructor is needed to build a non-identity
  // placement from Julia (e.g. an XCAF assembly component's location) --
  // previously only the default (identity) constructor was bound, since
  // Transformation() (read-only) was all export_glb's mesh-placement path
  // needed.
  mod.add_type<TopLoc_Location>("TopLoc_Location").constructor<>().constructor<const gp_Trsf&>();
  mod.method("IsIdentity", [](const TopLoc_Location& l) -> bool { return bool(l.IsIdentity()); });
  mod.method("Transformation", [](const TopLoc_Location& l) -> gp_Trsf { return l.Transformation(); });

  mod.add_type<Poly_Triangulation>("Poly_Triangulation");
  mod.method("NbNodes",     [](const Handle(Poly_Triangulation)& t) -> int { return t->NbNodes(); });
  mod.method("NbTriangles", [](const Handle(Poly_Triangulation)& t) -> int { return t->NbTriangles(); });
  mod.method("IsNull",      [](const Handle(Poly_Triangulation)& t) -> bool { return t.IsNull(); });
  mod.method("Node", [](const Handle(Poly_Triangulation)& t, int i) -> gp_Pnt {
    return t->Node(i);
  });
  mod.method("Triangle", [](const Handle(Poly_Triangulation)& t, int i) {
    int n1 = 0, n2 = 0, n3 = 0;
    t->Triangle(i).Get(n1, n2, n3);
    return std::make_tuple(n1, n2, n3);
  });

  mod.method("BRep_Tool_Triangulation", [](const TopoDS_Face& f) {
    TopLoc_Location loc;
    Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(f, loc);
    return std::make_tuple(tri, loc);
  });
}
