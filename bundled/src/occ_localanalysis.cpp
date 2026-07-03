// occ_localanalysis.cpp — 1:1 CxxWrap binding for LocalAnalysis_SurfaceContinuity:
// checks C0/C1/C2/G0/G1/G2 continuity between two points on two surfaces
// (e.g. two adjacent patches meeting at a shared edge) -- a genuine gap,
// no continuity-CHECKING tool existed previously (only continuity-
// requesting parameters on fitting/approximation algorithms).
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <LocalAnalysis_SurfaceContinuity.hxx>
#include <Geom_Surface.hxx>

void register_occ_localanalysis(jlcxx::Module& mod) {
  mod.add_type<LocalAnalysis_SurfaceContinuity>("LocalAnalysis_SurfaceContinuity")
     .constructor([](const Handle(Geom_Surface)& s1, double u1, double v1,
                      const Handle(Geom_Surface)& s2, double u2, double v2, int order,
                      double epsNul, double epsC0, double epsC1, double epsC2, double epsG1,
                      double percent, double maxLen) -> LocalAnalysis_SurfaceContinuity* {
       return new LocalAnalysis_SurfaceContinuity(s1, u1, v1, s2, u2, v2, GeomAbs_Shape(order),
                                                   epsNul, epsC0, epsC1, epsC2, epsG1, percent, maxLen);
     });

  mod.method("IsDone", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsDone()); });
  mod.method("ContinuityStatus", [](const LocalAnalysis_SurfaceContinuity& c) -> int {
    return int(c.ContinuityStatus());
  });
  mod.method("IsC0", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsC0()); });
  mod.method("IsC1", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsC1()); });
  mod.method("IsC2", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsC2()); });
  mod.method("IsG1", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsG1()); });
  mod.method("IsG2", [](const LocalAnalysis_SurfaceContinuity& c) -> bool { return bool(c.IsG2()); });
  mod.method("C0Value", [](const LocalAnalysis_SurfaceContinuity& c) -> double { return c.C0Value(); });
  mod.method("G1Angle", [](const LocalAnalysis_SurfaceContinuity& c) -> double { return c.G1Angle(); });
  mod.method("G2CurvatureGap", [](const LocalAnalysis_SurfaceContinuity& c) -> double {
    return c.G2CurvatureGap();
  });
}
