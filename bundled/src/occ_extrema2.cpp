// occ_extrema2.cpp — 1:1 CxxWrap bindings for BRepExtrema_SelfIntersection
// (3D face-vs-face self-intersection on a solid/shell) and
// BRepExtrema_ShapeProximity (tessellation-based overlap/proximity between
// two independent shapes) -- genuinely new capability: nothing else in this
// codebase detects self-intersection at the 3D-face level, or two-shape
// overlap at all (BRepAlgoAPI_Check is an exact BOP-based validity check;
// ShapeAnalysis_Wire's self-crossing check is 2D, within one wire only).
// Both classes are triangulation-based: the caller must run the
// already-bound BRepMesh_IncrementalMesh first, neither meshes internally.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepExtrema_SelfIntersection.hxx>
#include <BRepExtrema_ShapeProximity.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <NCollection_DataMap.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

#include <utility>

namespace jlcxx {
  // BRepExtrema_ShapeProximity's BRepExtrema_ProximityValueTool member carries an
  // internal NCollection_CellFilter::Cell with a deleted copy constructor (it has a
  // user-declared move constructor instead). std::is_copy_constructible_v reports
  // this type as copy-constructible anyway (the deletion is buried several
  // NCollection template layers deep and isn't resolved until the copy ctor body
  // is actually instantiated), so add_type's default add_copy_constructor<T>
  // (gated on jlcxx::CopyConstructible<T>, not IsMirroredType) tries to generate
  // a "copy" method and hits a hard compile error. Opt out of that generation
  // directly via the CopyConstructible trait instead.
  template<> struct CopyConstructible<BRepExtrema_ShapeProximity> : std::false_type { };
}

namespace {
  // Flattens an overlap map (face-index -> set of overlapping face indices, from
  // either BRepExtrema_ShapeProximity::OverlapSubShapes1/2 or
  // BRepExtrema_SelfIntersection::OverlapElements) into a stable, index-addressable
  // sequence of (key, value) pairs -- mirrors occ_dimtol.cpp's "recompute inside the
  // lambda, expose Count/Get(index)" idiom rather than exposing the packed-map type
  // itself. Iteration order is stable across repeated calls for an unmodified map.
  using OverlapMap = NCollection_DataMap<int, TColStd_PackedMapOfInteger>;

  int CountOverlapPairs(const OverlapMap& m) {
    int count = 0;
    for (OverlapMap::Iterator it(m); it.More(); it.Next()) count += it.Value().Extent();
    return count;
  }

  std::pair<int, int> NthOverlapPair(const OverlapMap& m, int index) {
    int i = 0;
    for (OverlapMap::Iterator it(m); it.More(); it.Next()) {
      for (TColStd_PackedMapOfInteger::Iterator vit(it.Value()); vit.More(); vit.Next()) {
        if (i == index) return {it.Key(), vit.Key()};
        ++i;
      }
    }
    return {-1, -1};
  }
}

void register_occ_extrema2(jlcxx::Module& mod) {
  // ---- BRepExtrema_SelfIntersection ----
  mod.add_type<BRepExtrema_SelfIntersection>("BRepExtrema_SelfIntersection")
     .constructor<double>()
     .constructor<const TopoDS_Shape&, double>();
  mod.method("LoadShape", [](BRepExtrema_SelfIntersection& s, const TopoDS_Shape& shape) -> bool {
    return bool(s.LoadShape(shape));
  });
  mod.method("Perform", [](BRepExtrema_SelfIntersection& s) { s.Perform(); });
  mod.method("IsDone", [](const BRepExtrema_SelfIntersection& s) -> bool { return bool(s.IsDone()); });
  mod.method("BRepExtrema_SelfIntersection_NbOverlaps", [](const BRepExtrema_SelfIntersection& s) -> int {
    return CountOverlapPairs(s.OverlapElements());
  });
  mod.method("BRepExtrema_SelfIntersection_OverlapFace1",
             [](const BRepExtrema_SelfIntersection& s, int index) -> int {
    return NthOverlapPair(s.OverlapElements(), index).first;
  });
  mod.method("BRepExtrema_SelfIntersection_OverlapFace2",
             [](const BRepExtrema_SelfIntersection& s, int index) -> int {
    return NthOverlapPair(s.OverlapElements(), index).second;
  });
  mod.method("BRepExtrema_SelfIntersection_GetSubShape",
             [](const BRepExtrema_SelfIntersection& s, int id) -> TopoDS_Face { return s.GetSubShape(id); });

  // ---- BRepExtrema_ShapeProximity ----
  mod.add_type<BRepExtrema_ShapeProximity>("BRepExtrema_ShapeProximity")
     .constructor<double>()
     .constructor<const TopoDS_Shape&, const TopoDS_Shape&, double>();
  mod.method("LoadShape1", [](BRepExtrema_ShapeProximity& p, const TopoDS_Shape& s) -> bool {
    return bool(p.LoadShape1(s));
  });
  mod.method("LoadShape2", [](BRepExtrema_ShapeProximity& p, const TopoDS_Shape& s) -> bool {
    return bool(p.LoadShape2(s));
  });
  mod.method("SetTolerance", [](BRepExtrema_ShapeProximity& p, double tol) { p.SetTolerance(tol); });
  mod.method("Tolerance", [](const BRepExtrema_ShapeProximity& p) -> double { return p.Tolerance(); });
  mod.method("Perform", [](BRepExtrema_ShapeProximity& p) { p.Perform(); });
  mod.method("IsDone", [](const BRepExtrema_ShapeProximity& p) -> bool { return bool(p.IsDone()); });
  mod.method("BRepExtrema_ShapeProximity_NbOverlaps", [](const BRepExtrema_ShapeProximity& p) -> int {
    return CountOverlapPairs(p.OverlapSubShapes1());
  });
  mod.method("BRepExtrema_ShapeProximity_OverlapFace1",
             [](const BRepExtrema_ShapeProximity& p, int index) -> int {
    return NthOverlapPair(p.OverlapSubShapes1(), index).first;
  });
  mod.method("BRepExtrema_ShapeProximity_OverlapFace2",
             [](const BRepExtrema_ShapeProximity& p, int index) -> int {
    return NthOverlapPair(p.OverlapSubShapes1(), index).second;
  });
  mod.method("BRepExtrema_ShapeProximity_GetSubShape1",
             [](const BRepExtrema_ShapeProximity& p, int id) -> TopoDS_Shape { return p.GetSubShape1(id); });
  mod.method("BRepExtrema_ShapeProximity_GetSubShape2",
             [](const BRepExtrema_ShapeProximity& p, int id) -> TopoDS_Shape { return p.GetSubShape2(id); });
}
