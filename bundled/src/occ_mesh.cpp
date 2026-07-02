// occ_mesh.cpp — 1:1 CxxWrap bindings for BRepMesh_IncrementalMesh and BRepAlgoAPI_Check.
#include <jlcxx/jlcxx.hpp>
#include "occ_exception.hpp"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepAlgoAPI_Check.hxx>
#include <TopoDS_Shape.hxx>

namespace jlcxx {
  template<> struct IsMirroredType<BRepMesh_IncrementalMesh> : std::false_type { };
}

void register_occ_mesh(jlcxx::Module& mod) {
  mod.add_type<BRepMesh_IncrementalMesh>("BRepMesh_IncrementalMesh")
     .constructor<>()
     .constructor<const TopoDS_Shape&, double, bool, double, bool>();
  mod.method("Perform",
      [](BRepMesh_IncrementalMesh& m) { m.Perform(); });
  mod.method("IsModified",
      [](const BRepMesh_IncrementalMesh& m) -> bool { return bool(m.IsModified()); });
  mod.method("GetStatusFlags",
      [](const BRepMesh_IncrementalMesh& m) -> int { return int(m.GetStatusFlags()); });

  mod.add_type<BRepAlgoAPI_Check>("BRepAlgoAPI_Check")
     .constructor<>()
     .constructor<const TopoDS_Shape&, bool, bool>();
  mod.method("SetData",
      [](BRepAlgoAPI_Check& c, const TopoDS_Shape& s, bool bTestSE, bool bTestSI) {
        c.SetData(s, bTestSE, bTestSI);
      });
  // Self-intersection testing on degenerate geometry has been observed to
  // segfault inside OCCT's parallel executor (OSD_Parallel::For /
  // BOPAlgo_CheckerSI) rather than throw a catchable Standard_Failure --
  // exposed so callers can force sequential execution before Perform().
  mod.method("SetRunParallel",
      [](BRepAlgoAPI_Check& c, bool flag) { c.SetRunParallel(flag); });
  mod.method("Perform",
      [](BRepAlgoAPI_Check& c) { occ_guard([&]{ c.Perform(); return 0; }); });
  mod.method("IsValid",
      [](BRepAlgoAPI_Check& c) -> bool { return bool(c.IsValid()); });
}
