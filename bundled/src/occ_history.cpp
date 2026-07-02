// occ_history.cpp — 1:1 CxxWrap bindings for BRepTools_History, and the
// .History() accessor on the boolean-operation classes that provide it
// natively (BRepAlgoAPI_Fuse/Cut/Common/Splitter, via their common
// BRepAlgoAPI_BuilderAlgo base). History is filled by default
// (BOPAlgo_BuilderShape's myFillHistory defaults to Standard_True), so no
// extra setup call is needed before Build().
//
// Must be registered (register_occ_history) after register_occ_geom (which
// registers the opencascade::handle<T> smart-pointer template) and after
// register_occ_builders / register_occ_advanced (which add_type the boolean
// operation classes this file attaches .History() to).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepTools_History.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>

void register_occ_history(jlcxx::Module& mod) {
  mod.add_type<BRepTools_History>("BRepTools_History");

  mod.method("Modified", [](const Handle(BRepTools_History)& h, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return h->Modified(s);
  });
  mod.method("Generated", [](const Handle(BRepTools_History)& h, const TopoDS_Shape& s) -> TopTools_ListOfShape {
    return h->Generated(s);
  });
  mod.method("IsRemoved", [](const Handle(BRepTools_History)& h, const TopoDS_Shape& s) -> bool {
    return bool(h->IsRemoved(s));
  });
  mod.method("HasModified", [](const Handle(BRepTools_History)& h) -> bool {
    return bool(h->HasModified());
  });
  mod.method("HasGenerated", [](const Handle(BRepTools_History)& h) -> bool {
    return bool(h->HasGenerated());
  });
  mod.method("HasRemoved", [](const Handle(BRepTools_History)& h) -> bool {
    return bool(h->HasRemoved());
  });
  mod.method("IsNull", [](const Handle(BRepTools_History)& h) -> bool {
    return h.IsNull();
  });

  mod.method("History", [](BRepAlgoAPI_Fuse& m) -> Handle(BRepTools_History) { return m.History(); });
  mod.method("History", [](BRepAlgoAPI_Cut& m) -> Handle(BRepTools_History) { return m.History(); });
  mod.method("History", [](BRepAlgoAPI_Common& m) -> Handle(BRepTools_History) { return m.History(); });
  mod.method("History", [](BRepAlgoAPI_Splitter& m) -> Handle(BRepTools_History) { return m.History(); });

  // ShapeBuild_ReShape (via ShapeFix_Root::Context()) tracks old-shape ->
  // new-shape substitutions made during healing, exposed through the same
  // BRepTools_History interface so Naming v2 can remap tags through heal()
  // the same way it does through any other operation.
  mod.add_type<ShapeBuild_ReShape>("ShapeBuild_ReShape");
  mod.method("Context", [](const ShapeFix_Shape& fixer) -> Handle(ShapeBuild_ReShape) {
    return fixer.Context();
  });
  mod.method("History", [](const Handle(ShapeBuild_ReShape)& r) -> Handle(BRepTools_History) {
    return r->History();
  });

  mod.method("History", [](const ShapeUpgrade_UnifySameDomain& u) -> Handle(BRepTools_History) {
    return u.History();
  });
}
