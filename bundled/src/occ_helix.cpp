// occ_helix.cpp — 1:1 CxxWrap binding for HelixBRep_BuilderHelix: real
// helical/spring/thread-path curve geometry, returning a ready-to-use
// TopoDS_Wire (no manual curve-to-wire assembly needed). Only the simplest
// "pure helix, given pitch(es) and number of turns" SetParameters overload
// is bound -- HelixBRep_BuilderHelix has five more (composite/tapered
// diameter schedules, explicit heights instead of turn counts) left for a
// future round if a caller actually needs them.
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <HelixBRep_BuilderHelix.hxx>
#include <NCollection_Array1.hxx>
#include <gp_Ax3.hxx>
#include <TopoDS_Shape.hxx>

namespace {
NCollection_Array1<double> RealArrayFrom(jlcxx::ArrayRef<double> vals) {
  int n = int(vals.size());
  NCollection_Array1<double> arr(1, n);
  for (int i = 0; i < n; ++i) arr.SetValue(i + 1, vals[i]);
  return arr;
}
}

void register_occ_helix(jlcxx::Module& mod) {
  mod.add_type<HelixBRep_BuilderHelix>("HelixBRep_BuilderHelix").constructor<>();

  mod.method("SetParameters", [](HelixBRep_BuilderHelix& h, const gp_Ax3& axis, double diam,
                                  jlcxx::ArrayRef<double> pitches, jlcxx::ArrayRef<double> nbTurns) {
    h.SetParameters(axis, diam, RealArrayFrom(pitches), RealArrayFrom(nbTurns));
  });
  mod.method("SetApproxParameters", [](HelixBRep_BuilderHelix& h, double tolerance, int maxDegree,
                                        int continuity) {
    h.SetApproxParameters(tolerance, maxDegree, GeomAbs_Shape(continuity));
  });
  mod.method("Perform", [](HelixBRep_BuilderHelix& h) { occ_guard([&]{ h.Perform(); return 0; }); });
  mod.method("Shape", [](const HelixBRep_BuilderHelix& h) -> TopoDS_Shape { return h.Shape(); });
  mod.method("ErrorStatus", [](const HelixBRep_BuilderHelix& h) -> int { return h.ErrorStatus(); });
  mod.method("WarningStatus", [](const HelixBRep_BuilderHelix& h) -> int { return h.WarningStatus(); });
  mod.method("ToleranceReached", [](const HelixBRep_BuilderHelix& h) -> double { return h.ToleranceReached(); });
}
