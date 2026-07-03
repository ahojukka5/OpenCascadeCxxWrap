// occ_bopalgo_builder.cpp — 1:1 CxxWrap bindings for the BOPAlgo "power tool"
// entry points that sit above pairwise BRepAlgoAPI_Fuse/Cut/Common:
//   - BOPAlgo_Builder      — General Fuse: split all arguments against each
//     other in one pass (result = compound of all split parts, not merged
//     into a single fused solid).
//   - BOPAlgo_CellsBuilder — General Fuse plus per-cell selection: pick which
//     split parts (by IN/OUT membership against each argument) end up in the
//     result, and optionally merge same-material cells across boundaries.
//   - BOPAlgo_MakerVolume  — build solids directly from a set of faces/shells
//     (the inverse of exploding a solid into faces).
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_CellsBuilder.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Shape.hxx>

void register_occ_bopalgo_builder(jlcxx::Module& mod) {
  // BOPAlgo_Builder — General Fuse.
  mod.add_type<BOPAlgo_Builder>("BOPAlgo_Builder").constructor<>();
  mod.method("AddArgument", [](BOPAlgo_Builder& b, const TopoDS_Shape& s) { b.AddArgument(s); });
  mod.method("SetArguments", [](BOPAlgo_Builder& b, const TopTools_ListOfShape& args) {
    b.SetArguments(args);
  });
  mod.method("Perform", [](BOPAlgo_Builder& b) { occ_guard([&]{ b.Perform(); return 0; }); });
  mod.method("HasErrors", [](const BOPAlgo_Builder& b) -> bool { return b.HasErrors(); });
  mod.method("Shape", [](const BOPAlgo_Builder& b) -> TopoDS_Shape { return b.Shape(); });

  // BOPAlgo_CellsBuilder — General Fuse + cell selection/merging.
  mod.add_type<BOPAlgo_CellsBuilder>("BOPAlgo_CellsBuilder").constructor<>();
  mod.method("SetArguments", [](BOPAlgo_CellsBuilder& b, const TopTools_ListOfShape& args) {
    b.SetArguments(args);
  });
  mod.method("Perform", [](BOPAlgo_CellsBuilder& b) { occ_guard([&]{ b.Perform(); return 0; }); });
  mod.method("HasErrors", [](const BOPAlgo_CellsBuilder& b) -> bool { return b.HasErrors(); });
  mod.method("Shape", [](const BOPAlgo_CellsBuilder& b) -> TopoDS_Shape { return b.Shape(); });
  mod.method("AddToResult", [](BOPAlgo_CellsBuilder& b, const TopTools_ListOfShape& toTake,
                                const TopTools_ListOfShape& toAvoid, int material, bool update) {
    occ_guard([&]{ b.AddToResult(toTake, toAvoid, material, update); return 0; });
  });
  mod.method("AddAllToResult", [](BOPAlgo_CellsBuilder& b, int material, bool update) {
    occ_guard([&]{ b.AddAllToResult(material, update); return 0; });
  });
  mod.method("RemoveFromResult", [](BOPAlgo_CellsBuilder& b, const TopTools_ListOfShape& toTake,
                                     const TopTools_ListOfShape& toAvoid) {
    b.RemoveFromResult(toTake, toAvoid);
  });
  mod.method("RemoveAllFromResult", [](BOPAlgo_CellsBuilder& b) { b.RemoveAllFromResult(); });
  mod.method("RemoveInternalBoundaries", [](BOPAlgo_CellsBuilder& b) {
    occ_guard([&]{ b.RemoveInternalBoundaries(); return 0; });
  });
  mod.method("GetAllParts", [](const BOPAlgo_CellsBuilder& b) -> TopoDS_Shape { return b.GetAllParts(); });
  mod.method("MakeContainers", [](BOPAlgo_CellsBuilder& b) { b.MakeContainers(); });

  // BOPAlgo_MakerVolume — build solids from a set of faces/shells.
  mod.add_type<BOPAlgo_MakerVolume>("BOPAlgo_MakerVolume").constructor<>();
  mod.method("SetArguments", [](BOPAlgo_MakerVolume& b, const TopTools_ListOfShape& args) {
    b.SetArguments(args);
  });
  mod.method("SetIntersect", [](BOPAlgo_MakerVolume& b, bool v) { b.SetIntersect(v); });
  mod.method("SetAvoidInternalShapes", [](BOPAlgo_MakerVolume& b, bool v) { b.SetAvoidInternalShapes(v); });
  mod.method("Perform", [](BOPAlgo_MakerVolume& b) { occ_guard([&]{ b.Perform(); return 0; }); });
  mod.method("HasErrors", [](const BOPAlgo_MakerVolume& b) -> bool { return b.HasErrors(); });
  mod.method("Shape", [](const BOPAlgo_MakerVolume& b) -> TopoDS_Shape { return b.Shape(); });
}
