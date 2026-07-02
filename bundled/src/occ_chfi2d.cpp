// occ_chfi2d.cpp — 1:1 CxxWrap bindings for ChFi2d_FilletAPI and
// ChFi2d_ChamferAPI: sketch-level (2D, planar) fillets and chamfers
// between two edges, distinct from the already-bound 3D
// BRepFilletAPI_MakeFillet/MakeChamfer. Both are plain classes, no
// Standard_Transient inheritance, no Handle-management concerns.
#include <jlcxx/jlcxx.hpp>

#include <ChFi2d_FilletAPI.hxx>
#include <ChFi2d_ChamferAPI.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>

void register_occ_chfi2d(jlcxx::Module& mod) {
  mod.add_type<ChFi2d_FilletAPI>("ChFi2d_FilletAPI")
     .constructor<>()
     .constructor<const TopoDS_Wire&, const gp_Pln&>()
     .constructor<const TopoDS_Edge&, const TopoDS_Edge&, const gp_Pln&>();
  mod.method("Init", [](ChFi2d_FilletAPI& f, const TopoDS_Wire& w, const gp_Pln& pln) {
    f.Init(w, pln);
  });
  mod.method("Init", [](ChFi2d_FilletAPI& f, const TopoDS_Edge& e1, const TopoDS_Edge& e2,
                         const gp_Pln& pln) {
    f.Init(e1, e2, pln);
  });
  mod.method("Perform", [](ChFi2d_FilletAPI& f, double radius) -> bool {
    return bool(f.Perform(radius));
  });
  mod.method("NbResults", [](ChFi2d_FilletAPI& f, const gp_Pnt& p) -> int { return f.NbResults(p); });
  // Result()'s modified-edge out-params are returned alongside the fillet
  // edge as a 3-tuple (fillet_edge, modified_edge1, modified_edge2) rather
  // than via Julia-side mutable references, matching this shim's existing
  // convention for OCCT out-parameters (e.g. GProp/Extrema methods).
  mod.method("Result", [](ChFi2d_FilletAPI& f, const gp_Pnt& p, int iSolution) {
    TopoDS_Edge e1, e2;
    TopoDS_Edge filletEdge = f.Result(p, e1, e2, iSolution);
    return std::make_tuple(filletEdge, e1, e2);
  });

  mod.add_type<ChFi2d_ChamferAPI>("ChFi2d_ChamferAPI")
     .constructor<>()
     .constructor<const TopoDS_Wire&>()
     .constructor<const TopoDS_Edge&, const TopoDS_Edge&>();
  mod.method("Init", [](ChFi2d_ChamferAPI& c, const TopoDS_Wire& w) { c.Init(w); });
  mod.method("Init", [](ChFi2d_ChamferAPI& c, const TopoDS_Edge& e1, const TopoDS_Edge& e2) {
    c.Init(e1, e2);
  });
  mod.method("Perform", [](ChFi2d_ChamferAPI& c) -> bool { return bool(c.Perform()); });
  mod.method("Result", [](ChFi2d_ChamferAPI& c, double length1, double length2) {
    TopoDS_Edge e1, e2;
    TopoDS_Edge chamferEdge = c.Result(e1, e2, length1, length2);
    return std::make_tuple(chamferEdge, e1, e2);
  });
}
