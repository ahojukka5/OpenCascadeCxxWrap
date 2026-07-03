// occ_feat.cpp — 1:1 CxxWrap bindings for BRepFeat_MakePrism, BRepFeat_MakeRevol
// (feature-tree boss/pocket modeling: extrude/revolve a sketch up to a face),
// and BRepFeat_MakeLinearForm/MakeRevolutionForm/MakePipe (ribs, slots, and
// pipe features -- both MakeLinearForm and MakeRevolutionForm inherit
// BRepFeat_RibSlot; MakePipe inherits BRepFeat_Form directly; all three
// ultimately derive from BRepBuilderAPI_MakeShape, same as MakePrism/MakeRevol
// above, so Shape()/IsDone()/Modified()/Generated()/IsDeleted() are the same
// shape already proven twice in this file).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepFeat_MakePrism.hxx>
#include <BRepFeat_MakeRevol.hxx>
#include <BRepFeat_MakeDPrism.hxx>
#include <BRepFeat_SplitShape.hxx>
#include <BRepFeat_Gluer.hxx>
#include <BRepFeat_MakeLinearForm.hxx>
#include <BRepFeat_MakeRevolutionForm.hxx>
#include <BRepFeat_MakePipe.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>

namespace {
  // BRepFeat_MakeLinearForm/MakeRevolutionForm need Handle(Geom_Plane)
  // specifically, but Monge's Julia-facing plane construction only ever
  // produces a Handle(Geom_Surface) (Geom_Plane is never add_type'd in
  // this codebase, only ever upcast via factory functions) -- same
  // downcast-with-a-clear-error idiom as RequireBSplineCurve in
  // occ_geomfill.cpp.
  Handle(Geom_Plane) RequirePlane(const Handle(Geom_Surface)& s) {
    Handle(Geom_Plane) p = Handle(Geom_Plane)::DownCast(s);
    if (p.IsNull()) {
      throw std::runtime_error("rib_feature/slot_feature: the given surface is not a Geom_Plane "
                                "(build it with a planar face's own surface, e.g. from a flat "
                                "sketch face)");
    }
    return p;
  }
}

void register_occ_feat(jlcxx::Module& mod) {
  mod.add_type<BRepFeat_MakePrism>("BRepFeat_MakePrism")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, const TopoDS_Face&,
                  const gp_Dir&, int, bool>();

  mod.add_type<BRepFeat_MakeRevol>("BRepFeat_MakeRevol")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, const TopoDS_Face&,
                  const gp_Ax1&, int, bool>();

  // ---- BRepFeat_MakePrism ----
  mod.method("Init", [](BRepFeat_MakePrism& m, const TopoDS_Shape& sbase, const TopoDS_Shape& pbase,
                         const TopoDS_Face& skface, const gp_Dir& dir, int fuse, bool modify) {
    m.Init(sbase, pbase, skface, dir, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakePrism& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakePrism& m, double length) { m.Perform(length); });
  mod.method("Perform", [](BRepFeat_MakePrism& m, const TopoDS_Shape& until) { m.Perform(until); });
  mod.method("Perform", [](BRepFeat_MakePrism& m, const TopoDS_Shape& from, const TopoDS_Shape& until) {
    m.Perform(from, until);
  });
  mod.method("PerformUntilEnd",  [](BRepFeat_MakePrism& m) { m.PerformUntilEnd(); });
  mod.method("PerformFromEnd",   [](BRepFeat_MakePrism& m, const TopoDS_Shape& fUntil) {
    m.PerformFromEnd(fUntil);
  });
  mod.method("PerformThruAll",   [](BRepFeat_MakePrism& m) { m.PerformThruAll(); });
  mod.method("PerformUntilHeight", [](BRepFeat_MakePrism& m, const TopoDS_Shape& until, double length) {
    m.PerformUntilHeight(until, length);
  });
  mod.method("Shape",  [](BRepFeat_MakePrism& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakePrism& m) -> bool { return bool(m.IsDone()); });
  mod.method("Modified", [](BRepFeat_MakePrism& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepFeat_MakePrism& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepFeat_MakePrism& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  // ---- BRepFeat_MakeRevol ----
  mod.method("Init", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& sbase, const TopoDS_Shape& pbase,
                         const TopoDS_Face& skface, const gp_Ax1& axis, int fuse, bool modify) {
    m.Init(sbase, pbase, skface, axis, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakeRevol& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, double angle) { m.Perform(angle); });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& until) { m.Perform(until); });
  mod.method("Perform", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& from, const TopoDS_Shape& until) {
    m.Perform(from, until);
  });
  mod.method("PerformThruAll",    [](BRepFeat_MakeRevol& m) { m.PerformThruAll(); });
  mod.method("PerformUntilAngle", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& until, double angle) {
    m.PerformUntilAngle(until, angle);
  });
  mod.method("Shape",  [](BRepFeat_MakeRevol& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeRevol& m) -> bool { return bool(m.IsDone()); });
  mod.method("Modified", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepFeat_MakeRevol& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  mod.add_type<BRepFeat_MakeDPrism>("BRepFeat_MakeDPrism")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Face&, const TopoDS_Face&,
                  double, int, bool>();
  mod.method("Perform", [](BRepFeat_MakeDPrism& m, double length) { m.Perform(length); });
  mod.method("Shape",  [](BRepFeat_MakeDPrism& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeDPrism& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepFeat_SplitShape>("BRepFeat_SplitShape")
     .constructor<const TopoDS_Shape&>();
  mod.method("Add", [](BRepFeat_SplitShape& m, const TopoDS_Wire& w, const TopoDS_Face& f) {
    m.Add(w, f);
  });
  mod.method("Build",  [](BRepFeat_SplitShape& m) { m.Build(); });
  mod.method("Shape",  [](BRepFeat_SplitShape& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_SplitShape& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<BRepFeat_Gluer>("BRepFeat_Gluer")
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&>();
  mod.method("Bind",   [](BRepFeat_Gluer& m, const TopoDS_Face& f1, const TopoDS_Face& f2) {
    m.Bind(f1, f2);
  });
  mod.method("Build",  [](BRepFeat_Gluer& m) { m.Build(); });
  mod.method("Shape",  [](BRepFeat_Gluer& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_Gluer& m) -> bool { return bool(m.IsDone()); });

  // ---- BRepFeat_MakeLinearForm (rib/groove along a planar developable surface) ----
  // The multi-arg constructor takes Handle(Geom_Plane), which is never add_type'd in
  // this codebase (only ever upcast to Handle(Geom_Surface)) -- bind only the default
  // constructor and drive construction through Init, downcasting the incoming
  // Handle(Geom_Surface) via RequirePlane.
  mod.add_type<BRepFeat_MakeLinearForm>("BRepFeat_MakeLinearForm")
     .constructor<>();
  mod.method("Init", [](BRepFeat_MakeLinearForm& m, const TopoDS_Shape& sbase, const TopoDS_Wire& w,
                         const Handle(Geom_Surface)& plane, const gp_Vec& direction,
                         const gp_Vec& direction1, int fuse, bool modify) {
    m.Init(sbase, w, RequirePlane(plane), direction, direction1, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakeLinearForm& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakeLinearForm& m) { m.Perform(); });
  mod.method("Shape",  [](BRepFeat_MakeLinearForm& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeLinearForm& m) -> bool { return bool(m.IsDone()); });
  mod.method("Modified", [](BRepFeat_MakeLinearForm& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepFeat_MakeLinearForm& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepFeat_MakeLinearForm& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  // ---- BRepFeat_MakeRevolutionForm (rib/groove along a revolved surface) ----
  // Same Handle(Geom_Plane) constraint as MakeLinearForm above, plus the multi-arg
  // constructor takes bool& Sliding as an out-param -- skip that constructor entirely
  // and bridge Sliding through Init's return value instead (by-value input hint,
  // mutated internally by reference, returned as the mutated output).
  mod.add_type<BRepFeat_MakeRevolutionForm>("BRepFeat_MakeRevolutionForm")
     .constructor<>();
  mod.method("Init", [](BRepFeat_MakeRevolutionForm& m, const TopoDS_Shape& sbase, const TopoDS_Wire& w,
                         const Handle(Geom_Surface)& plane, const gp_Ax1& axis,
                         double height1, double height2, int fuse, bool sliding) -> bool {
    m.Init(sbase, w, RequirePlane(plane), axis, height1, height2, fuse, sliding);
    return sliding;
  });
  mod.method("Add", [](BRepFeat_MakeRevolutionForm& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakeRevolutionForm& m) { m.Perform(); });
  mod.method("Shape",  [](BRepFeat_MakeRevolutionForm& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakeRevolutionForm& m) -> bool { return bool(m.IsDone()); });
  mod.method("Modified", [](BRepFeat_MakeRevolutionForm& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepFeat_MakeRevolutionForm& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepFeat_MakeRevolutionForm& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  // ---- BRepFeat_MakePipe (pipe/sweep features along a spine wire) ----
  // No Geom_Plane involved, no out-params -- the multi-arg constructor binds directly.
  mod.add_type<BRepFeat_MakePipe>("BRepFeat_MakePipe")
     .constructor<>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, const TopoDS_Face&,
                  const TopoDS_Wire&, int, bool>();
  mod.method("Init", [](BRepFeat_MakePipe& m, const TopoDS_Shape& sbase, const TopoDS_Shape& pbase,
                         const TopoDS_Face& skface, const TopoDS_Wire& spine, int fuse, bool modify) {
    m.Init(sbase, pbase, skface, spine, fuse, modify);
  });
  mod.method("Add", [](BRepFeat_MakePipe& m, const TopoDS_Edge& e, const TopoDS_Face& onFace) {
    m.Add(e, onFace);
  });
  mod.method("Perform", [](BRepFeat_MakePipe& m) { m.Perform(); });
  mod.method("Perform", [](BRepFeat_MakePipe& m, const TopoDS_Shape& until) { m.Perform(until); });
  mod.method("Perform", [](BRepFeat_MakePipe& m, const TopoDS_Shape& from, const TopoDS_Shape& until) {
    m.Perform(from, until);
  });
  mod.method("Shape",  [](BRepFeat_MakePipe& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("IsDone", [](const BRepFeat_MakePipe& m) -> bool { return bool(m.IsDone()); });
  mod.method("Modified", [](BRepFeat_MakePipe& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepFeat_MakePipe& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepFeat_MakePipe& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });
}
