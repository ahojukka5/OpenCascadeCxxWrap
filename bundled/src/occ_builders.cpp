// occ_builders.cpp — 1:1 CxxWrap bindings for BRepPrimAPI, BRepBuilderAPI, BRepAlgoAPI.
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"
#include <jlcxx/jlcxx.hpp>

#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom2d_Curve.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_GTrsf.hxx>

#include <TopoDS_CompSolid.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_WireError.hxx>

#include <BRepBuilderAPI_GTransform.hxx>

#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_Copy.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Section.hxx>

void register_occ_builders(jlcxx::Module& mod) {
  // ===== BRepPrimAPI: primitive solids ======================================
  mod.add_type<BRepPrimAPI_MakeBox>("BRepPrimAPI_MakeBox")
     .constructor([](double dx, double dy, double dz) -> BRepPrimAPI_MakeBox* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeBox(dx, dy, dz); });
     })
     .constructor([](const gp_Pnt& p, double dx, double dy, double dz) -> BRepPrimAPI_MakeBox* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeBox(p, dx, dy, dz); });
     })
     .constructor([](const gp_Pnt& p1, const gp_Pnt& p2) -> BRepPrimAPI_MakeBox* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeBox(p1, p2); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeBox& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeBox& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakeCylinder>("BRepPrimAPI_MakeCylinder")
     .constructor([](double r, double h) -> BRepPrimAPI_MakeCylinder* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCylinder(r, h); });
     })
     .constructor([](double r, double h, double angle) -> BRepPrimAPI_MakeCylinder* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCylinder(r, h, angle); });
     })
     .constructor([](const gp_Ax2& ax, double r, double h) -> BRepPrimAPI_MakeCylinder* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCylinder(ax, r, h); });
     })
     .constructor([](const gp_Ax2& ax, double r, double h, double angle) -> BRepPrimAPI_MakeCylinder* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCylinder(ax, r, h, angle); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeCylinder& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeCylinder& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakeSphere>("BRepPrimAPI_MakeSphere")
     .constructor([](double r) -> BRepPrimAPI_MakeSphere* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeSphere(r); });
     })
     .constructor([](const gp_Pnt& c, double r) -> BRepPrimAPI_MakeSphere* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeSphere(c, r); });
     })
     .constructor([](const gp_Ax2& ax, double r) -> BRepPrimAPI_MakeSphere* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeSphere(ax, r); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeSphere& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeSphere& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakeCone>("BRepPrimAPI_MakeCone")
     .constructor([](double r1, double r2, double h) -> BRepPrimAPI_MakeCone* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCone(r1, r2, h); });
     })
     .constructor([](const gp_Ax2& ax, double r1, double r2, double h) -> BRepPrimAPI_MakeCone* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeCone(ax, r1, r2, h); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeCone& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeCone& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakeTorus>("BRepPrimAPI_MakeTorus")
     .constructor([](double r1, double r2) -> BRepPrimAPI_MakeTorus* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeTorus(r1, r2); });
     })
     .constructor([](const gp_Ax2& ax, double r1, double r2) -> BRepPrimAPI_MakeTorus* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeTorus(ax, r1, r2); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeTorus& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeTorus& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakePrism>("BRepPrimAPI_MakePrism")
     .constructor<const TopoDS_Shape&, const gp_Vec&>();
  mod.method("Shape", [](BRepPrimAPI_MakePrism& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });

  mod.add_type<BRepPrimAPI_MakeRevol>("BRepPrimAPI_MakeRevol")
     .constructor<const TopoDS_Shape&, const gp_Ax1&, double>();
  mod.method("Shape", [](BRepPrimAPI_MakeRevol& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });

  // ===== BRepBuilderAPI: builders ===========================================
  mod.add_type<BRepBuilderAPI_MakeVertex>("BRepBuilderAPI_MakeVertex")
     .constructor<const gp_Pnt&>();
  mod.method("Shape",  [](BRepBuilderAPI_MakeVertex& m) -> TopoDS_Shape  { return m.Shape(); });
  mod.method("Vertex", [](BRepBuilderAPI_MakeVertex& m) -> TopoDS_Vertex { return m.Vertex(); });

  mod.add_type<BRepBuilderAPI_MakeEdge>("BRepBuilderAPI_MakeEdge")
     .constructor<const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Lin&>()
     .constructor<const gp_Lin&, double, double>()
     .constructor<const gp_Lin&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Circ&>()
     .constructor<const gp_Circ&, double, double>()
     .constructor<const gp_Circ&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Elips&>()
     .constructor<const gp_Elips&, double, double>()
     .constructor<const gp_Elips&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Hypr&>()
     .constructor<const gp_Hypr&, double, double>()
     .constructor<const gp_Hypr&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Parab&>()
     .constructor<const gp_Parab&, double, double>()
     .constructor<const gp_Parab&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const Handle(Geom_Curve)&>()
     .constructor<const Handle(Geom_Curve)&, double, double>()
     .constructor<const Handle(Geom_Curve)&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const TopoDS_Vertex&, const TopoDS_Vertex&>();
  mod.method("Shape", [](BRepBuilderAPI_MakeEdge& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Edge",  [](BRepBuilderAPI_MakeEdge& m) -> TopoDS_Edge  { return m.Edge(); });

  mod.add_type<BRepBuilderAPI_MakeEdge2d>("BRepBuilderAPI_MakeEdge2d")
     .constructor<const Handle(Geom2d_Curve)&>()
     .constructor<const Handle(Geom2d_Curve)&, double, double>()
     .constructor<const gp_Pnt2d&, const gp_Pnt2d&>();
  mod.method("Shape", [](BRepBuilderAPI_MakeEdge2d& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Edge",  [](BRepBuilderAPI_MakeEdge2d& m) -> TopoDS_Edge  { return m.Edge(); });
  mod.method("IsDone", [](const BRepBuilderAPI_MakeEdge2d& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepBuilderAPI_MakeWire>("BRepBuilderAPI_MakeWire").constructor<>()
     .constructor<const TopoDS_Edge&>()
     .constructor<const TopoDS_Wire&>();
  mod.method("Add",   [](BRepBuilderAPI_MakeWire& m, const TopoDS_Edge& e) { m.Add(e); });
  mod.method("Add",   [](BRepBuilderAPI_MakeWire& m, const TopoDS_Wire& w) { m.Add(w); });
  mod.method("Add",   [](BRepBuilderAPI_MakeWire& m, const TopTools_ListOfShape& l) { m.Add(l); });
  mod.method("IsDone",[](const BRepBuilderAPI_MakeWire& m) -> bool { return bool(m.IsDone()); });
  mod.method("Error", [](const BRepBuilderAPI_MakeWire& m) -> int { return int(m.Error()); });
  mod.method("Shape", [](BRepBuilderAPI_MakeWire& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Wire",  [](BRepBuilderAPI_MakeWire& m) -> TopoDS_Wire  { return m.Wire(); });
  mod.method("Edge",  [](const BRepBuilderAPI_MakeWire& m) -> TopoDS_Edge { return m.Edge(); });
  mod.method("Vertex",[](const BRepBuilderAPI_MakeWire& m) -> TopoDS_Vertex { return m.Vertex(); });

  mod.method("BRepBuilderAPI_WireDone",           []() { return int(BRepBuilderAPI_WireDone); });
  mod.method("BRepBuilderAPI_EmptyWire",          []() { return int(BRepBuilderAPI_EmptyWire); });
  mod.method("BRepBuilderAPI_DisconnectedWire",   []() { return int(BRepBuilderAPI_DisconnectedWire); });
  mod.method("BRepBuilderAPI_NonManifoldWire",    []() { return int(BRepBuilderAPI_NonManifoldWire); });

  mod.add_type<BRepBuilderAPI_MakeFace>("BRepBuilderAPI_MakeFace")
     .constructor<const gp_Pln&>()
     .constructor<const gp_Pln&, double, double, double, double>()
     .constructor<const gp_Pln&, const TopoDS_Wire&, bool>()
     .constructor<const gp_Cylinder&>()
     .constructor<const gp_Cylinder&, double, double, double, double>()
     .constructor<const gp_Cylinder&, const TopoDS_Wire&, bool>()
     .constructor<const gp_Cone&>()
     .constructor<const gp_Cone&, double, double, double, double>()
     .constructor<const gp_Cone&, const TopoDS_Wire&, bool>()
     .constructor<const gp_Sphere&>()
     .constructor<const gp_Sphere&, double, double, double, double>()
     .constructor<const gp_Sphere&, const TopoDS_Wire&, bool>()
     .constructor<const gp_Torus&>()
     .constructor<const gp_Torus&, double, double, double, double>()
     .constructor<const gp_Torus&, const TopoDS_Wire&, bool>()
     .constructor<const Handle(Geom_Surface)&, double>()
     .constructor<const Handle(Geom_Surface)&, double, double, double, double, double>()
     .constructor<const Handle(Geom_Surface)&, const TopoDS_Wire&, bool>()
     .constructor<const TopoDS_Wire&, bool>();
  mod.method("Shape", [](BRepBuilderAPI_MakeFace& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Face",  [](BRepBuilderAPI_MakeFace& m) -> TopoDS_Face  { return m.Face(); });

  mod.add_type<BRepBuilderAPI_MakeShell>("BRepBuilderAPI_MakeShell").constructor<>();
  mod.method("Init", [](BRepBuilderAPI_MakeShell& m, const Handle(Geom_Surface)& s,
                         double umin, double umax, double vmin, double vmax, bool seg) {
    m.Init(s, umin, umax, vmin, vmax, seg);
  });
  mod.method("IsDone", [](const BRepBuilderAPI_MakeShell& m) -> bool { return bool(m.IsDone()); });
  mod.method("Shape", [](BRepBuilderAPI_MakeShell& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Shell", [](BRepBuilderAPI_MakeShell& m) -> TopoDS_Shell { return m.Shell(); });
  mod.method("Error", [](const BRepBuilderAPI_MakeShell& m) -> int { return int(m.Error()); });

  mod.add_type<BRepBuilderAPI_MakeSolid>("BRepBuilderAPI_MakeSolid").constructor<>()
     .constructor<const TopoDS_CompSolid&>()
     .constructor<const TopoDS_Shell&>()
     .constructor<const TopoDS_Shell&, const TopoDS_Shell&>()
     .constructor<const TopoDS_Shell&, const TopoDS_Shell&, const TopoDS_Shell&>()
     .constructor<const TopoDS_Solid&>()
     .constructor<const TopoDS_Solid&, const TopoDS_Shell&>();
  mod.method("Shape",     [](BRepBuilderAPI_MakeSolid& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Solid",     [](BRepBuilderAPI_MakeSolid& m) -> TopoDS_Solid { return m.Solid(); });
  mod.method("IsDone",    [](const BRepBuilderAPI_MakeSolid& m) -> bool { return bool(m.IsDone()); });
  mod.method("IsDeleted", [](BRepBuilderAPI_MakeSolid& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  mod.add_type<BRepBuilderAPI_MakePolygon>("BRepBuilderAPI_MakePolygon").constructor<>();
  mod.method("Add",   [](BRepBuilderAPI_MakePolygon& m, const gp_Pnt& p) { m.Add(p); });
  mod.method("Close", [](BRepBuilderAPI_MakePolygon& m) { m.Close(); });
  mod.method("Shape", [](BRepBuilderAPI_MakePolygon& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Wire",  [](BRepBuilderAPI_MakePolygon& m) -> TopoDS_Wire  { return m.Wire(); });

  mod.add_type<BRepBuilderAPI_Transform>("BRepBuilderAPI_Transform")
     .constructor<const TopoDS_Shape&, const gp_Trsf&, bool>();
  mod.method("Shape", [](BRepBuilderAPI_Transform& m) -> TopoDS_Shape { return m.Shape(); });

  mod.add_type<BRepBuilderAPI_Copy>("BRepBuilderAPI_Copy")
     .constructor<const TopoDS_Shape&>();
  mod.method("Shape", [](BRepBuilderAPI_Copy& m) -> TopoDS_Shape { return m.Shape(); });

  mod.add_type<BRepBuilderAPI_GTransform>("BRepBuilderAPI_GTransform")
     .constructor<const gp_GTrsf&>()
     .constructor<const TopoDS_Shape&, const gp_GTrsf&, bool>();
  mod.method("Perform", [](BRepBuilderAPI_GTransform& m, const TopoDS_Shape& s, bool copy) {
    m.Perform(s, copy);
  });
  mod.method("Shape", [](BRepBuilderAPI_GTransform& m) -> TopoDS_Shape { return m.Shape(); });

  // ===== BRepPrimAPI: half-space and wedge ==================================
  mod.add_type<BRepPrimAPI_MakeHalfSpace>("BRepPrimAPI_MakeHalfSpace")
     .constructor<const TopoDS_Face&, const gp_Pnt&>()
     .constructor<const TopoDS_Shell&, const gp_Pnt&>();
  mod.method("Shape", [](BRepPrimAPI_MakeHalfSpace& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeHalfSpace& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  mod.add_type<BRepPrimAPI_MakeWedge>("BRepPrimAPI_MakeWedge")
     .constructor([](double dx, double dy, double dz, double ltx) -> BRepPrimAPI_MakeWedge* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeWedge(dx, dy, dz, ltx); });
     })
     .constructor([](const gp_Ax2& ax, double dx, double dy, double dz, double ltx) -> BRepPrimAPI_MakeWedge* {
       return occ_guard([&]{ return new BRepPrimAPI_MakeWedge(ax, dx, dy, dz, ltx); });
     });
  mod.method("Shape", [](BRepPrimAPI_MakeWedge& m) -> TopoDS_Shape { return occ_guard([&]{ return m.Shape(); }); });
  mod.method("Solid", [](BRepPrimAPI_MakeWedge& m) -> TopoDS_Solid { return occ_guard([&]{ return m.Solid(); }); });

  // ===== BRepAlgoAPI: booleans ==============================================
  // Default constructor + SetArguments/SetTools/SetFuzzyValue/Build (below)
  // is the path used when a caller wants fuzzy-tolerance control -- the
  // eager 2-shape constructor computes the result immediately, too early
  // for SetFuzzyValue to have any effect. SetFuzzyValue itself comes from
  // BOPAlgo_Options via `protected` inheritance, re-exposed public through
  // BRepAlgoAPI_Algo's `using BOPAlgo_Options::SetFuzzyValue;` -- confirmed
  // callable directly on Fuse/Cut/Common the same way SetRunParallel
  // already works on BRepAlgoAPI_Check (occ_mesh.cpp) via the identical
  // using-declaration pattern.
  mod.add_type<BRepAlgoAPI_Fuse>("BRepAlgoAPI_Fuse")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>()
     .constructor<>();
  mod.method("Shape", [](BRepAlgoAPI_Fuse& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Modified", [](BRepAlgoAPI_Fuse& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepAlgoAPI_Fuse& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepAlgoAPI_Fuse& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });
  mod.method("IsDone", [](const BRepAlgoAPI_Fuse& m) -> bool { return bool(m.IsDone()); });
  mod.method("SetArguments", [](BRepAlgoAPI_Fuse& m, const TopTools_ListOfShape& a) { m.SetArguments(a); });
  mod.method("SetTools",     [](BRepAlgoAPI_Fuse& m, const TopTools_ListOfShape& t) { m.SetTools(t); });
  mod.method("SetFuzzyValue",[](BRepAlgoAPI_Fuse& m, double v) { m.SetFuzzyValue(v); });
  mod.method("Build",        [](BRepAlgoAPI_Fuse& m) { m.Build(); });

  mod.add_type<BRepAlgoAPI_Cut>("BRepAlgoAPI_Cut")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>()
     .constructor<>();
  mod.method("Shape", [](BRepAlgoAPI_Cut& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Modified", [](BRepAlgoAPI_Cut& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepAlgoAPI_Cut& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepAlgoAPI_Cut& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });
  mod.method("IsDone", [](const BRepAlgoAPI_Cut& m) -> bool { return bool(m.IsDone()); });
  mod.method("SetArguments", [](BRepAlgoAPI_Cut& m, const TopTools_ListOfShape& a) { m.SetArguments(a); });
  mod.method("SetTools",     [](BRepAlgoAPI_Cut& m, const TopTools_ListOfShape& t) { m.SetTools(t); });
  mod.method("SetFuzzyValue",[](BRepAlgoAPI_Cut& m, double v) { m.SetFuzzyValue(v); });
  mod.method("Build",        [](BRepAlgoAPI_Cut& m) { m.Build(); });

  mod.add_type<BRepAlgoAPI_Common>("BRepAlgoAPI_Common")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>()
     .constructor<>();
  mod.method("Shape", [](BRepAlgoAPI_Common& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Modified", [](BRepAlgoAPI_Common& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepAlgoAPI_Common& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepAlgoAPI_Common& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });
  mod.method("IsDone", [](const BRepAlgoAPI_Common& m) -> bool { return bool(m.IsDone()); });
  mod.method("SetArguments", [](BRepAlgoAPI_Common& m, const TopTools_ListOfShape& a) { m.SetArguments(a); });
  mod.method("SetTools",     [](BRepAlgoAPI_Common& m, const TopTools_ListOfShape& t) { m.SetTools(t); });
  mod.method("SetFuzzyValue",[](BRepAlgoAPI_Common& m, double v) { m.SetFuzzyValue(v); });
  mod.method("Build",        [](BRepAlgoAPI_Common& m) { m.Build(); });

  mod.add_type<BRepAlgoAPI_Section>("BRepAlgoAPI_Section")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>();
  mod.method("Shape", [](BRepAlgoAPI_Section& m) -> TopoDS_Shape { return m.Shape(); });
}
