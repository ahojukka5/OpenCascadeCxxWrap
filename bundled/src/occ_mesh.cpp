// occ_mesh.cpp — 1:1 CxxWrap bindings for BRepMesh_IncrementalMesh and BRepAlgoAPI_Check.
#include <jlcxx/jlcxx.hpp>
#include "occ_exception.hpp"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepAlgoAPI_Check.hxx>
#include <IMeshTools_Parameters.hxx>
#include <TopoDS_Shape.hxx>

namespace jlcxx {
  template<> struct IsMirroredType<BRepMesh_IncrementalMesh> : std::false_type { };
  // IMeshTools_Parameters is an all-primitive-field POD, but its
  // user-declared default constructor (setting the OCCT defaults) makes it
  // non-trivial, so it fails jlcxx's is_trivial auto-mirroring check --
  // registered as a normal add_type with explicit field accessors instead.
  template<> struct IsMirroredType<IMeshTools_Parameters> : std::false_type { };
}

void register_occ_mesh(jlcxx::Module& mod) {
  // ---- IMeshTools_Parameters: richer mesh-quality control than
  // BRepMesh_IncrementalMesh's 5-arg constructor below (adds a floor on
  // triangle edge length via MinSize, separate interior-vs-boundary
  // deflection, and a deflection-quality guard) -- scoped to the fields
  // called out as relevant in this round's plan, not the full ~14-field
  // struct. Must be add_type'd before BRepMesh_IncrementalMesh, whose
  // constructor overload below takes it by reference.
  mod.add_type<IMeshTools_Parameters>("IMeshTools_Parameters")
     .constructor<>();
  mod.method("GetMinSize", [](const IMeshTools_Parameters& p) -> double { return p.MinSize; });
  mod.method("SetMinSize", [](IMeshTools_Parameters& p, double v) { p.MinSize = v; });
  mod.method("GetDeflection", [](const IMeshTools_Parameters& p) -> double { return p.Deflection; });
  mod.method("SetDeflection", [](IMeshTools_Parameters& p, double v) { p.Deflection = v; });
  mod.method("GetAngle", [](const IMeshTools_Parameters& p) -> double { return p.Angle; });
  mod.method("SetAngle", [](IMeshTools_Parameters& p, double v) { p.Angle = v; });
  mod.method("GetAngleInterior", [](const IMeshTools_Parameters& p) -> double { return p.AngleInterior; });
  mod.method("SetAngleInterior", [](IMeshTools_Parameters& p, double v) { p.AngleInterior = v; });
  mod.method("GetDeflectionInterior",
             [](const IMeshTools_Parameters& p) -> double { return p.DeflectionInterior; });
  mod.method("SetDeflectionInterior", [](IMeshTools_Parameters& p, double v) { p.DeflectionInterior = v; });
  mod.method("GetControlSurfaceDeflection",
             [](const IMeshTools_Parameters& p) -> bool { return bool(p.ControlSurfaceDeflection); });
  mod.method("SetControlSurfaceDeflection",
             [](IMeshTools_Parameters& p, bool v) { p.ControlSurfaceDeflection = v; });
  mod.method("GetAllowQualityDecrease",
             [](const IMeshTools_Parameters& p) -> bool { return bool(p.AllowQualityDecrease); });
  mod.method("SetAllowQualityDecrease",
             [](IMeshTools_Parameters& p, bool v) { p.AllowQualityDecrease = v; });
  mod.method("GetInParallel", [](const IMeshTools_Parameters& p) -> bool { return bool(p.InParallel); });
  mod.method("SetInParallel", [](IMeshTools_Parameters& p, bool v) { p.InParallel = v; });
  mod.method("GetRelative", [](const IMeshTools_Parameters& p) -> bool { return bool(p.Relative); });
  mod.method("SetRelative", [](IMeshTools_Parameters& p, bool v) { p.Relative = v; });

  mod.add_type<BRepMesh_IncrementalMesh>("BRepMesh_IncrementalMesh")
     .constructor<>()
     .constructor<const TopoDS_Shape&, double, bool, double, bool>()
     .constructor<const TopoDS_Shape&, const IMeshTools_Parameters&>();
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
