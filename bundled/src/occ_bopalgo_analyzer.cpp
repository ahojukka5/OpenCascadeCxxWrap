// occ_bopalgo_analyzer.cpp — 1:1 CxxWrap bindings for BOPAlgo_ArgumentAnalyzer,
// which diagnoses WHY a boolean operation would fail (self-intersection, bad
// argument type, incompatible vertices/edges/faces, ...) rather than just
// reporting IsDone() == false. Run it on the same operands before/after a
// BRepAlgoAPI_Fuse/Cut/Common/Splitter attempt to get an actionable reason.
#include <jlcxx/jlcxx.hpp>

#include <BOPAlgo_ArgumentAnalyzer.hxx>
#include <BOPAlgo_CheckResult.hxx>
#include <BOPAlgo_ListOfCheckResult.hxx>
#include <BOPAlgo_Operation.hxx>
#include <BOPAlgo_CheckStatus.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>

namespace jlcxx {
  template<> struct IsMirroredType<BOPAlgo_CheckResult> : std::false_type { };
}

void register_occ_bopalgo_analyzer(jlcxx::Module& mod) {
  mod.add_type<BOPAlgo_CheckResult>("BOPAlgo_CheckResult");
  mod.method("GetCheckStatus",   [](const BOPAlgo_CheckResult& r) -> int { return int(r.GetCheckStatus()); });
  mod.method("GetShape1",        [](const BOPAlgo_CheckResult& r) -> TopoDS_Shape { return r.GetShape1(); });
  mod.method("GetShape2",        [](const BOPAlgo_CheckResult& r) -> TopoDS_Shape { return r.GetShape2(); });
  mod.method("GetFaultyShapes1", [](const BOPAlgo_CheckResult& r) -> TopTools_ListOfShape {
    return r.GetFaultyShapes1();
  });
  mod.method("GetFaultyShapes2", [](const BOPAlgo_CheckResult& r) -> TopTools_ListOfShape {
    return r.GetFaultyShapes2();
  });

  mod.add_type<BOPAlgo_ArgumentAnalyzer>("BOPAlgo_ArgumentAnalyzer").constructor<>();
  mod.method("SetShape1", [](BOPAlgo_ArgumentAnalyzer& a, const TopoDS_Shape& s) { a.SetShape1(s); });
  mod.method("SetShape2", [](BOPAlgo_ArgumentAnalyzer& a, const TopoDS_Shape& s) { a.SetShape2(s); });
  mod.method("SetOperationType", [](BOPAlgo_ArgumentAnalyzer& a, int op) {
    a.OperationType() = BOPAlgo_Operation(op);
  });
  mod.method("SetStopOnFirstFaulty", [](BOPAlgo_ArgumentAnalyzer& a, bool v) {
    a.StopOnFirstFaulty() = v;
  });
  mod.method("Perform", [](BOPAlgo_ArgumentAnalyzer& a) { a.Perform(); });
  mod.method("HasFaulty", [](const BOPAlgo_ArgumentAnalyzer& a) -> bool { return bool(a.HasFaulty()); });
  mod.method("NbCheckResults", [](const BOPAlgo_ArgumentAnalyzer& a) -> int {
    return a.GetCheckResult().Extent();
  });
  mod.method("CheckResultValue", [](const BOPAlgo_ArgumentAnalyzer& a, int index) -> BOPAlgo_CheckResult {
    if (index < 1) return BOPAlgo_CheckResult();
    int i = 0;
    for (BOPAlgo_ListIteratorOfListOfCheckResult it(a.GetCheckResult()); it.More(); it.Next()) {
      ++i;
      if (i == index) return it.Value();
    }
    return BOPAlgo_CheckResult();
  });

  mod.method("BOPAlgo_COMMON",   []() { return int(BOPAlgo_COMMON); });
  mod.method("BOPAlgo_FUSE",     []() { return int(BOPAlgo_FUSE); });
  mod.method("BOPAlgo_CUT",      []() { return int(BOPAlgo_CUT); });
  mod.method("BOPAlgo_CUT21",    []() { return int(BOPAlgo_CUT21); });
  mod.method("BOPAlgo_SECTION",  []() { return int(BOPAlgo_SECTION); });
  mod.method("BOPAlgo_UNKNOWN",  []() { return int(BOPAlgo_UNKNOWN); });

  mod.method("BOPAlgo_CheckUnknown",            []() { return int(BOPAlgo_CheckUnknown); });
  mod.method("BOPAlgo_BadType",                 []() { return int(BOPAlgo_BadType); });
  mod.method("BOPAlgo_SelfIntersect",           []() { return int(BOPAlgo_SelfIntersect); });
  mod.method("BOPAlgo_TooSmallEdge",            []() { return int(BOPAlgo_TooSmallEdge); });
  mod.method("BOPAlgo_NonRecoverableFace",      []() { return int(BOPAlgo_NonRecoverableFace); });
  mod.method("BOPAlgo_IncompatibilityOfVertex", []() { return int(BOPAlgo_IncompatibilityOfVertex); });
  mod.method("BOPAlgo_IncompatibilityOfEdge",   []() { return int(BOPAlgo_IncompatibilityOfEdge); });
  mod.method("BOPAlgo_IncompatibilityOfFace",   []() { return int(BOPAlgo_IncompatibilityOfFace); });
  mod.method("BOPAlgo_OperationAborted",        []() { return int(BOPAlgo_OperationAborted); });
  mod.method("BOPAlgo_GeomAbs_C0",              []() { return int(BOPAlgo_GeomAbs_C0); });
  mod.method("BOPAlgo_InvalidCurveOnSurface",   []() { return int(BOPAlgo_InvalidCurveOnSurface); });
  mod.method("BOPAlgo_NotValid",                []() { return int(BOPAlgo_NotValid); });
}
