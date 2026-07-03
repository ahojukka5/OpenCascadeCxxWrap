// occ_feat_hole.cpp — 1:1 CxxWrap bindings for BRepFeat_MakeCylindricalHole:
// a feature-aware cylindrical hole tool, distinct from a plain boolean cut
// with an infinite cylinder in that it understands "through next surface",
// "until end of solid", and "blind to a given length" directly, with an
// optional post-operation validity control.
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <BRepFeat_MakeCylindricalHole.hxx>
#include <BRepFeat_Status.hxx>
#include <gp_Ax1.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_feat_hole(jlcxx::Module& mod) {
  mod.add_type<BRepFeat_MakeCylindricalHole>("BRepFeat_MakeCylindricalHole").constructor<>();

  mod.method("Init", [](BRepFeat_MakeCylindricalHole& h, const TopoDS_Shape& s, const gp_Ax1& axis) {
    h.Init(s, axis);
  });

  mod.method("Perform", [](BRepFeat_MakeCylindricalHole& h, double radius) {
    occ_guard([&]{ h.Perform(radius); return 0; });
  });
  mod.method("Perform", [](BRepFeat_MakeCylindricalHole& h, double radius, double pFrom, double pTo,
                            bool withControl) {
    occ_guard([&]{ h.Perform(radius, pFrom, pTo, withControl); return 0; });
  });
  mod.method("PerformThruNext", [](BRepFeat_MakeCylindricalHole& h, double radius, bool withControl) {
    occ_guard([&]{ h.PerformThruNext(radius, withControl); return 0; });
  });
  mod.method("PerformUntilEnd", [](BRepFeat_MakeCylindricalHole& h, double radius, bool withControl) {
    occ_guard([&]{ h.PerformUntilEnd(radius, withControl); return 0; });
  });
  mod.method("PerformBlind", [](BRepFeat_MakeCylindricalHole& h, double radius, double length,
                                 bool withControl) {
    occ_guard([&]{ h.PerformBlind(radius, length, withControl); return 0; });
  });

  mod.method("Build", [](BRepFeat_MakeCylindricalHole& h) { occ_guard([&]{ h.Build(); return 0; }); });
  mod.method("Shape", [](const BRepFeat_MakeCylindricalHole& h) -> TopoDS_Shape { return h.Shape(); });
  mod.method("HasErrors", [](const BRepFeat_MakeCylindricalHole& h) -> bool { return h.HasErrors(); });
  mod.method("Status", [](const BRepFeat_MakeCylindricalHole& h) -> int { return int(h.Status()); });

  mod.method("BRepFeat_NoError", []() -> int { return int(BRepFeat_NoError); });
  mod.method("BRepFeat_InvalidPlacement", []() -> int { return int(BRepFeat_InvalidPlacement); });
  mod.method("BRepFeat_HoleTooLong", []() -> int { return int(BRepFeat_HoleTooLong); });
}
