// occ_shape_analysis.cpp — 1:1 CxxWrap bindings for free-boundary and shell analysis.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeAnalysis_Shell.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>

void register_occ_shape_analysis(jlcxx::Module& mod) {
  // ---- add_type calls first ----

  mod.add_type<ShapeAnalysis_FreeBounds>("ShapeAnalysis_FreeBounds")
     .constructor<>()
     .constructor<const TopoDS_Shape&, double, bool, bool>()
     .constructor<const TopoDS_Shape&, bool, bool, bool>();

  mod.add_type<ShapeAnalysis_Shell>("ShapeAnalysis_Shell")
     .constructor<>();

  mod.add_type<ShapeAnalysis_Edge>("ShapeAnalysis_Edge")
     .constructor<>();

  // Scoped subset of ShapeAnalysis_Wire's much larger API (dozens of
  // individual Check* methods plus a generic ShapeExtend_Status bitmask
  // query system shared across the whole Shape Healing module): the
  // checks most useful for diagnosing why a wire is failing to become a
  // valid face, plus Perform() to run all of them at once.
  mod.add_type<ShapeAnalysis_Wire>("ShapeAnalysis_Wire")
     .constructor<>()
     .constructor<const TopoDS_Wire&, const TopoDS_Face&, double>();

  // ShapeAnalysis_Surface is Standard_Transient, only ever constructed from
  // Julia via the NewShapeAnalysisSurface factory below (same "add_type
  // without .constructor<>(), separately-named New* factory" idiom as
  // ShapeCustom_RestrictionParameters/TCollection_HAsciiString).
  mod.add_type<ShapeAnalysis_Surface>("ShapeAnalysis_Surface");

  // ---- ShapeAnalysis_FreeBounds methods ----

  mod.method("GetClosedWires", [](const ShapeAnalysis_FreeBounds& fb) -> TopoDS_Compound {
    return fb.GetClosedWires();
  });
  mod.method("GetOpenWires", [](const ShapeAnalysis_FreeBounds& fb) -> TopoDS_Compound {
    return fb.GetOpenWires();
  });

  // ---- ShapeAnalysis_Shell methods ----

  mod.method("LoadShells", [](ShapeAnalysis_Shell& sh, const TopoDS_Shape& shape) {
    sh.LoadShells(shape);
  });
  mod.method("CheckOrientedShells", [](ShapeAnalysis_Shell& sh, const TopoDS_Shape& shape,
                                       bool alsofree, bool checkinternaledges) -> bool {
    return bool(sh.CheckOrientedShells(shape, alsofree, checkinternaledges));
  });
  mod.method("IsLoaded",   [](ShapeAnalysis_Shell& sh, const TopoDS_Shape& shape) -> bool {
    return bool(sh.IsLoaded(shape));
  });
  mod.method("NbLoaded",   [](ShapeAnalysis_Shell& sh) -> int { return int(sh.NbLoaded()); });
  mod.method("HasBadEdges",[](ShapeAnalysis_Shell& sh) -> bool { return bool(sh.HasBadEdges()); });
  mod.method("BadEdges",   [](ShapeAnalysis_Shell& sh) -> TopoDS_Compound { return sh.BadEdges(); });
  mod.method("HasFreeEdges",[](ShapeAnalysis_Shell& sh) -> bool { return bool(sh.HasFreeEdges()); });
  mod.method("FreeEdges",  [](ShapeAnalysis_Shell& sh) -> TopoDS_Compound { return sh.FreeEdges(); });
  mod.method("HasConnectedEdges", [](ShapeAnalysis_Shell& sh) -> bool {
    return bool(sh.HasConnectedEdges());
  });
  mod.method("Clear", [](ShapeAnalysis_Shell& sh) { sh.Clear(); });

  // ---- ShapeAnalysis_Edge methods ----

  mod.method("HasCurve3d", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e) -> bool {
    return bool(sae.HasCurve3d(e));
  });
  mod.method("IsClosed3d", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e) -> bool {
    return bool(sae.IsClosed3d(e));
  });
  mod.method("HasPCurve", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e,
                              const TopoDS_Face& f) -> bool {
    return bool(sae.HasPCurve(e, f));
  });
  mod.method("IsSeam", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e,
                           const TopoDS_Face& f) -> bool {
    return bool(sae.IsSeam(e, f));
  });
  mod.method("FirstVertex", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e) -> TopoDS_Vertex {
    return sae.FirstVertex(e);
  });
  mod.method("LastVertex", [](const ShapeAnalysis_Edge& sae, const TopoDS_Edge& e) -> TopoDS_Vertex {
    return sae.LastVertex(e);
  });

  // ---- ShapeAnalysis_Wire methods ----
  // Every Check* method below returns True when a PROBLEM is found (matches
  // the doc comment on each, and the convention used throughout the Shape
  // Healing module -- e.g. CheckSelfIntersection: "Returns: True if at
  // least one check returned True", not "check passed").

  mod.method("Load", [](ShapeAnalysis_Wire& w, const TopoDS_Wire& wire) { w.Load(wire); });
  mod.method("SetFace", [](ShapeAnalysis_Wire& w, const TopoDS_Face& f) { w.SetFace(f); });
  mod.method("SetPrecision", [](ShapeAnalysis_Wire& w, double prec) { w.SetPrecision(prec); });
  mod.method("IsLoaded", [](const ShapeAnalysis_Wire& w) -> bool { return bool(w.IsLoaded()); });
  mod.method("ClearStatuses", [](ShapeAnalysis_Wire& w) { w.ClearStatuses(); });
  mod.method("Perform", [](ShapeAnalysis_Wire& w) -> bool { return bool(w.Perform()); });
  mod.method("CheckOrder", [](ShapeAnalysis_Wire& w, bool isClosed, bool mode3d) -> bool {
    return bool(w.CheckOrder(isClosed, mode3d));
  });
  mod.method("CheckConnected", [](ShapeAnalysis_Wire& w, double prec) -> bool {
    return bool(w.CheckConnected(prec));
  });
  mod.method("CheckSmall", [](ShapeAnalysis_Wire& w, double precsmall) -> bool {
    return bool(w.CheckSmall(precsmall));
  });
  mod.method("CheckClosed", [](ShapeAnalysis_Wire& w, double prec) -> bool {
    return bool(w.CheckClosed(prec));
  });
  mod.method("CheckSelfIntersection", [](ShapeAnalysis_Wire& w) -> bool {
    return bool(w.CheckSelfIntersection());
  });
  mod.method("CheckGaps3d", [](ShapeAnalysis_Wire& w) -> bool { return bool(w.CheckGaps3d()); });

  // ---- ShapeAnalysis_Surface ----
  // Complements GeomAPI_ProjectPointOnSurf with pole/seam-aware projection:
  // ValueOfUV/NextValueOfUV specifically improve on naive projection near
  // surface boundaries/poles (spheres/cones), and NextValueOfUV additionally
  // avoids seam-parameter jumps by stepping from a previous UV solution --
  // something GeomAPI_ProjectPointOnSurf structurally cannot do (no
  // "previous solution" input).

  // ShapeAnalysis_Surface is Standard_Transient (genuinely Handle-managed,
  // unlike e.g. XCAFDimTolObjects_Tool which merely has a Handle(Document)
  // constructor argument): methods take a Handle(ShapeAnalysis_Surface)&
  // and dereference explicitly (same idiom as TCollection_HAsciiString's
  // ToCString), since jlcxx has no implicit Handle(T) -> T& conversion at
  // the Julia dispatch layer -- passing a bare ShapeAnalysis_Surface& here
  // does not accept a Handle(ShapeAnalysis_Surface) value from Julia.
  mod.method("NewShapeAnalysisSurface", [](const Handle(Geom_Surface)& s) -> Handle(ShapeAnalysis_Surface) {
    return new ShapeAnalysis_Surface(s);
  });
  mod.method("ValueOfUV", [](const Handle(ShapeAnalysis_Surface)& sa, const gp_Pnt& p, double preci) -> gp_Pnt2d {
    return sa->ValueOfUV(p, preci);
  });
  mod.method("NextValueOfUV", [](const Handle(ShapeAnalysis_Surface)& sa, const gp_Pnt2d& prevUV,
                                  const gp_Pnt& p, double preci, double maxpreci) -> gp_Pnt2d {
    return sa->NextValueOfUV(prevUV, p, preci, maxpreci);
  });
  mod.method("IsDegenerated", [](const Handle(ShapeAnalysis_Surface)& sa, const gp_Pnt& p, double preci) -> bool {
    return bool(sa->IsDegenerated(p, preci));
  });
  mod.method("HasSingularities", [](const Handle(ShapeAnalysis_Surface)& sa, double preci) -> bool {
    return bool(sa->HasSingularities(preci));
  });
  mod.method("IsUClosed", [](const Handle(ShapeAnalysis_Surface)& sa, double preci) -> bool {
    return bool(sa->IsUClosed(preci));
  });
  mod.method("IsVClosed", [](const Handle(ShapeAnalysis_Surface)& sa, double preci) -> bool {
    return bool(sa->IsVClosed(preci));
  });
  mod.method("Gap", [](const Handle(ShapeAnalysis_Surface)& sa) -> double { return sa->Gap(); });
}
