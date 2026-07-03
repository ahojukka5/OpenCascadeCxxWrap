// occ_fillet2d.cpp — 1:1 CxxWrap bindings for BRepFilletAPI_MakeFillet2d:
// post-hoc fillets/chamfers on the vertices of an already-built planar
// TopoDS_Face -- unlike ChFi2d_FilletAPI/ChFi2d_ChamferAPI (occ_chfi2d.cpp),
// which only work during incremental sketch construction (returning new/
// shortened edges the live sketch splices in), this operates on any
// already-built planar face (imported, extracted from a solid, or a
// boolean/offset result) and rebuilds the face as a whole, with
// Modified(shape) history tracking for tag propagation.
#include <jlcxx/jlcxx.hpp>

#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>

void register_occ_fillet2d(jlcxx::Module& mod) {
  mod.add_type<BRepFilletAPI_MakeFillet2d>("BRepFilletAPI_MakeFillet2d")
     .constructor<>()
     .constructor<const TopoDS_Face&>();

  mod.method("Init", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Face& f) { m.Init(f); });
  mod.method("Init", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Face& refFace,
                         const TopoDS_Face& modFace) { m.Init(refFace, modFace); });

  mod.method("AddFillet", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Vertex& v,
                              double radius) -> TopoDS_Edge {
    return m.AddFillet(v, radius);
  });
  mod.method("ModifyFillet", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& fillet,
                                 double radius) -> TopoDS_Edge {
    return m.ModifyFillet(fillet, radius);
  });
  mod.method("RemoveFillet", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& fillet) -> TopoDS_Vertex {
    return m.RemoveFillet(fillet);
  });

  mod.method("AddChamfer", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& e1,
                               const TopoDS_Edge& e2, double d1, double d2) -> TopoDS_Edge {
    return m.AddChamfer(e1, e2, d1, d2);
  });
  mod.method("AddChamfer", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& e,
                               const TopoDS_Vertex& v, double d, double ang) -> TopoDS_Edge {
    return m.AddChamfer(e, v, d, ang);
  });
  mod.method("ModifyChamfer", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& chamfer,
                                  const TopoDS_Edge& e1, const TopoDS_Edge& e2,
                                  double d1, double d2) -> TopoDS_Edge {
    return m.ModifyChamfer(chamfer, e1, e2, d1, d2);
  });
  mod.method("ModifyChamfer", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& chamfer,
                                  const TopoDS_Edge& e, double d, double ang) -> TopoDS_Edge {
    return m.ModifyChamfer(chamfer, e, d, ang);
  });
  mod.method("RemoveChamfer", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& chamfer) -> TopoDS_Vertex {
    return m.RemoveChamfer(chamfer);
  });

  mod.method("IsModified", [](const BRepFilletAPI_MakeFillet2d& m, const TopoDS_Edge& e) -> bool {
    return bool(m.IsModified(e));
  });
  mod.method("NbFillet", [](const BRepFilletAPI_MakeFillet2d& m) -> int { return m.NbFillet(); });
  mod.method("FilletEdge", [](const BRepFilletAPI_MakeFillet2d& m, int i) -> TopoDS_Shape {
    return m.FilletEdges().Value(i);
  });
  mod.method("NbChamfer", [](const BRepFilletAPI_MakeFillet2d& m) -> int { return m.NbChamfer(); });
  mod.method("ChamferEdge", [](const BRepFilletAPI_MakeFillet2d& m, int i) -> TopoDS_Shape {
    return m.ChamferEdges().Value(i);
  });
  mod.method("Modified", [](BRepFilletAPI_MakeFillet2d& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });

  mod.method("BRepFilletAPI_MakeFillet2d_Status", [](const BRepFilletAPI_MakeFillet2d& m) -> int {
    return int(m.Status());
  });
  mod.method("Build", [](BRepFilletAPI_MakeFillet2d& m) { m.Build(); });
  mod.method("IsDone", [](const BRepFilletAPI_MakeFillet2d& m) -> bool { return bool(m.IsDone()); });
  mod.method("Shape", [](BRepFilletAPI_MakeFillet2d& m) -> TopoDS_Shape { return m.Shape(); });

  mod.method("ChFi2d_NotPlanar",             []() { return int(ChFi2d_NotPlanar); });
  mod.method("ChFi2d_NoFace",                []() { return int(ChFi2d_NoFace); });
  mod.method("ChFi2d_InitialisationError",   []() { return int(ChFi2d_InitialisationError); });
  mod.method("ChFi2d_ParametersError",       []() { return int(ChFi2d_ParametersError); });
  mod.method("ChFi2d_Ready",                 []() { return int(ChFi2d_Ready); });
  mod.method("ChFi2d_IsDone",                []() { return int(ChFi2d_IsDone); });
  mod.method("ChFi2d_ComputationError",      []() { return int(ChFi2d_ComputationError); });
  mod.method("ChFi2d_ConnexionError",        []() { return int(ChFi2d_ConnexionError); });
  mod.method("ChFi2d_TangencyError",         []() { return int(ChFi2d_TangencyError); });
  mod.method("ChFi2d_FirstEdgeDegenerated",  []() { return int(ChFi2d_FirstEdgeDegenerated); });
  mod.method("ChFi2d_LastEdgeDegenerated",   []() { return int(ChFi2d_LastEdgeDegenerated); });
  mod.method("ChFi2d_BothEdgesDegenerated",  []() { return int(ChFi2d_BothEdgesDegenerated); });
  mod.method("ChFi2d_NotAuthorized",         []() { return int(ChFi2d_NotAuthorized); });
}
