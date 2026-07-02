// occ_hlr.cpp — CxxWrap bindings for hidden-line removal (HLR): projecting a
// 3D shape into 2D visible/hidden edge compounds from a given viewpoint.
//
// New, late-registered TU (registered after gp_* and TopoDS_* are
// available). HLRBRep_Algo is Standard_Transient-derived (via
// HLRBRep_InternalAlgo) and -- unlike ShapeFix_Shape, which is only ever
// used by-reference within its own TU -- its Handle must flow into
// HLRBRep_HLRToShape's constructor. A plain .constructor<>() produces a raw
// CxxWrap-allocated object, not a Handle, which HLRBRep_HLRToShape's
// constructor can't accept (confirmed empirically). Follow the
// BRepTools_History convention instead: no direct add_type constructor,
// a free-function factory returning Handle(HLRBRep_Algo) (mirrors the
// Geom_BSplineCurve/Geom_Plane factory pattern in occ_geom.cpp), and every
// method taking const Handle(HLRBRep_Algo)& (mirrors BRepTools_History's
// own Modified/IsRemoved/HasModified signatures in occ_history.cpp) so the
// Julia-side object is consistently Handle-wrapped from construction
// onward. HLRBRep_HLRToShape itself is a plain (non-Transient) class,
// bound like GeomAPI_Interpolate.
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"

#include <HLRAlgo_Projector.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_hlr(jlcxx::Module& mod)
{
  mod.add_type<HLRAlgo_Projector>("HLRAlgo_Projector")
     .constructor<const gp_Ax2&>();

  mod.add_type<HLRBRep_Algo>("HLRBRep_Algo");
  // Named new_hlr_algo(), not HLRBRep_Algo() -- add_type<HLRBRep_Algo>("HLRBRep_Algo")
  // itself reserves that exact string for CxxWrap's own (::Type{HLRBRep_Algo})()
  // calling convention regardless of whether .constructor<>() is chained;
  // defining a same-named mod.method collides with it ("Method definition
  // ... overwritten" at precompile time, confirmed empirically). Every other
  // Handle-factory in this shim (Geom_BSplineCurve, Geom_Plane, ...) avoids
  // this because its factory name is a concrete-subclass name distinct from
  // the abstract add_type'd base (Geom_Curve/Geom_Surface); HLRBRep_Algo has
  // no such distinct base to borrow a name from.
  mod.method("new_hlr_algo", []() -> Handle(HLRBRep_Algo) { return new HLRBRep_Algo(); });
  mod.method("Add", [](const Handle(HLRBRep_Algo)& a, const TopoDS_Shape& s, int nbIso) {
    a->Add(s, nbIso);
  });
  mod.method("Projector", [](const Handle(HLRBRep_Algo)& a, const HLRAlgo_Projector& p) {
    a->Projector(p);
  });
  // Update()/Hide() run the actual visible/hidden-edge computation --
  // guarded since this is a substantial geometric algorithm over arbitrary
  // input, the same caution applied to every other non-trivial OCCT
  // builder call in this shim.
  mod.method("Update", [](const Handle(HLRBRep_Algo)& a) {
    occ_guard([&]{ a->Update(); return 0; });
  });
  mod.method("Hide", [](const Handle(HLRBRep_Algo)& a) {
    occ_guard([&]{ a->Hide(); return 0; });
  });

  mod.add_type<HLRBRep_HLRToShape>("HLRBRep_HLRToShape")
     .constructor<const Handle(HLRBRep_Algo)&>();
  mod.method("VCompound",         [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.VCompound(); });
  mod.method("OutLineVCompound",  [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.OutLineVCompound(); });
  mod.method("Rg1LineVCompound",  [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.Rg1LineVCompound(); });
  mod.method("HCompound",         [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.HCompound(); });
  mod.method("OutLineHCompound",  [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.OutLineHCompound(); });
  mod.method("Rg1LineHCompound",  [](HLRBRep_HLRToShape& s) -> TopoDS_Shape { return s.Rg1LineHCompound(); });
}
