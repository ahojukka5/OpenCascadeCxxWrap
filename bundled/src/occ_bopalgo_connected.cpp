// occ_bopalgo_connected.cpp — 1:1 CxxWrap bindings for multi-body
// conditioning, distinct from the General-Fuse "power tools" in
// occ_bopalgo_builder.cpp:
//   - BOPAlgo_MakeConnected — glue/share coincident geometry across a set of
//     touching shapes without a full boolean fuse (result stays multiple
//     bodies as one compound, but is now topologically connected); also the
//     entry point for making a shape periodic/tileable (delegates to
//     BOPAlgo_MakePeriodic below).
//   - BOPAlgo_MakePeriodic — makes an arbitrary shape periodic/tileable in
//     given directions, with RepeatShape to stamp out tiled copies.
// Both are plain BOPAlgo_Options-derived classes (no Standard_Transient, no
// Handle-factory needed), same idiom as BOPAlgo_Builder/CellsBuilder.
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <BOPAlgo_MakeConnected.hxx>
#include <BOPAlgo_MakePeriodic.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_bopalgo_connected(jlcxx::Module& mod) {
  // ---- BOPAlgo_MakePeriodic (registered first: BOPAlgo_MakeConnected's
  // MakePeriodic method takes a reference to its nested PeriodicityParams
  // type, but we sidestep that struct entirely via the inline
  // MakeXPeriodic/MakeYPeriodic/MakeZPeriodic convenience setters below) ----
  mod.add_type<BOPAlgo_MakePeriodic>("BOPAlgo_MakePeriodic").constructor<>();
  mod.method("SetShape", [](BOPAlgo_MakePeriodic& p, const TopoDS_Shape& s) { p.SetShape(s); });
  mod.method("MakeXPeriodic", [](BOPAlgo_MakePeriodic& p, bool isPeriodic, double period) {
    p.MakeXPeriodic(isPeriodic, period);
  });
  mod.method("MakeYPeriodic", [](BOPAlgo_MakePeriodic& p, bool isPeriodic, double period) {
    p.MakeYPeriodic(isPeriodic, period);
  });
  mod.method("MakeZPeriodic", [](BOPAlgo_MakePeriodic& p, bool isPeriodic, double period) {
    p.MakeZPeriodic(isPeriodic, period);
  });
  mod.method("Perform", [](BOPAlgo_MakePeriodic& p) { occ_guard([&]{ p.Perform(); return 0; }); });
  mod.method("HasErrors", [](const BOPAlgo_MakePeriodic& p) -> bool { return p.HasErrors(); });
  mod.method("Shape", [](const BOPAlgo_MakePeriodic& p) -> TopoDS_Shape { return p.Shape(); });
  mod.method("BOPAlgo_MakePeriodic_RepeatShape",
             [](BOPAlgo_MakePeriodic& p, int directionId, int times) -> TopoDS_Shape {
    return occ_guard([&]{ return p.RepeatShape(directionId, times); });
  });
  mod.method("RepeatedShape", [](const BOPAlgo_MakePeriodic& p) -> TopoDS_Shape { return p.RepeatedShape(); });

  // ---- BOPAlgo_MakeConnected ----
  mod.add_type<BOPAlgo_MakeConnected>("BOPAlgo_MakeConnected").constructor<>();
  mod.method("AddArgument", [](BOPAlgo_MakeConnected& c, const TopoDS_Shape& s) { c.AddArgument(s); });
  mod.method("Perform", [](BOPAlgo_MakeConnected& c) { occ_guard([&]{ c.Perform(); return 0; }); });
  mod.method("HasErrors", [](const BOPAlgo_MakeConnected& c) -> bool { return c.HasErrors(); });
  mod.method("Shape", [](const BOPAlgo_MakeConnected& c) -> TopoDS_Shape { return c.Shape(); });
  mod.method("BOPAlgo_MakeConnected_RepeatShape",
             [](BOPAlgo_MakeConnected& c, int directionId, int times) {
    occ_guard([&]{ c.RepeatShape(directionId, times); return 0; });
  });
  mod.method("PeriodicShape", [](const BOPAlgo_MakeConnected& c) -> TopoDS_Shape { return c.PeriodicShape(); });
  mod.method("BOPAlgo_MakeConnected_MakePeriodic",
             [](BOPAlgo_MakeConnected& c, const BOPAlgo_MakePeriodic& params) {
    occ_guard([&]{ c.MakePeriodic(params.PeriodicityParameters()); return 0; });
  });
}
