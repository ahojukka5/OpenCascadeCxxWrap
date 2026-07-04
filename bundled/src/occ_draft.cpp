// occ_draft.cpp — 1:1 CxxWrap bindings for BRepOffsetAPI_DraftAngle and
// BRepOffsetAPI_MakeDraft. Not to be confused with each other:
// DraftAngle drafts faces of an *existing* solid; MakeDraft builds a new
// draft/taper *shell* from a profile wire toward a limiting length,
// surface, or stop-shape (mold/die/casting draft-face generation).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>
#include "occ_exception.hpp"

#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeDraft.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <Geom_Surface.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopTools_ListOfShape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>

void register_occ_draft(jlcxx::Module& mod) {
  mod.add_type<BRepOffsetAPI_DraftAngle>("BRepOffsetAPI_DraftAngle")
     .constructor<>()
     .constructor<const TopoDS_Shape&>();

  mod.method("Init", [](BRepOffsetAPI_DraftAngle& m, const TopoDS_Shape& s) { m.Init(s); });
  mod.method("Clear", [](BRepOffsetAPI_DraftAngle& m) { m.Clear(); });
  mod.method("Add", [](BRepOffsetAPI_DraftAngle& m, const TopoDS_Face& f, const gp_Dir& dir,
                        double angle, const gp_Pln& neutralPlane, bool flag) {
    m.Add(f, dir, angle, neutralPlane, flag);
  });
  mod.method("AddDone", [](const BRepOffsetAPI_DraftAngle& m) -> bool { return bool(m.AddDone()); });
  mod.method("Build", [](BRepOffsetAPI_DraftAngle& m) { m.Build(); });
  mod.method("Shape", [](BRepOffsetAPI_DraftAngle& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Modified", [](BRepOffsetAPI_DraftAngle& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Modified(s);
  });
  mod.method("Generated", [](BRepOffsetAPI_DraftAngle& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
  mod.method("IsDeleted", [](BRepOffsetAPI_DraftAngle& m, const TopoDS_Shape& s) -> bool {
    return bool(m.IsDeleted(s));
  });

  // ---- BRepOffsetAPI_MakeDraft ----
  mod.add_type<BRepOffsetAPI_MakeDraft>("BRepOffsetAPI_MakeDraft")
     .constructor([](const TopoDS_Shape& shape, const gp_Dir& dir, double angle) -> BRepOffsetAPI_MakeDraft* {
       return occ_guard([&]{ return new BRepOffsetAPI_MakeDraft(shape, dir, angle); });
     });
  mod.method("SetOptions", [](BRepOffsetAPI_MakeDraft& m, int style, double angleMin, double angleMax) {
    m.SetOptions(BRepBuilderAPI_TransitionMode(style), angleMin, angleMax);
  });
  mod.method("SetDraft", [](BRepOffsetAPI_MakeDraft& m, bool isInternal) { m.SetDraft(isInternal); });
  mod.method("Perform", [](BRepOffsetAPI_MakeDraft& m, double lengthMax) {
    occ_guard([&]{ m.Perform(lengthMax); return 0; });
  });
  mod.method("Perform", [](BRepOffsetAPI_MakeDraft& m, const Handle(Geom_Surface)& surf, bool keepInsideSurface) {
    occ_guard([&]{ m.Perform(surf, keepInsideSurface); return 0; });
  });
  mod.method("Perform", [](BRepOffsetAPI_MakeDraft& m, const TopoDS_Shape& stopShape, bool keepOutSide) {
    occ_guard([&]{ m.Perform(stopShape, keepOutSide); return 0; });
  });
  mod.method("Shell", [](const BRepOffsetAPI_MakeDraft& m) -> TopoDS_Shell { return m.Shell(); });
  mod.method("Shape", [](BRepOffsetAPI_MakeDraft& m) -> TopoDS_Shape { return m.Shape(); });
  mod.method("Generated", [](BRepOffsetAPI_MakeDraft& m, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return m.Generated(s);
  });
}
