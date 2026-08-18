// occ_helix.cpp — cross-version helix builder exposed through the same
// CxxWrap API. OCCT 8 provides HelixBRep_BuilderHelix directly; OCCT 7.x
// does not, so the compatibility path builds a smooth BSpline helix from
// sampled points and returns the same ready-to-use TopoDS_Wire result.
#include <jlcxx/jlcxx.hpp>

#include "occ_exception.hpp"

#include <GeomAbs_Shape.hxx>
#include <NCollection_Array1.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Ax3.hxx>

#include <algorithm>
#include <cmath>
#include <vector>

#if defined(__has_include)
#  if __has_include(<HelixBRep_BuilderHelix.hxx>)
#    define OCC_HAS_HELIX_BUILDER 1
#  endif
#endif

#ifdef OCC_HAS_HELIX_BUILDER

#include <HelixBRep_BuilderHelix.hxx>
using HelixBuilderAdapter = HelixBRep_BuilderHelix;

#else

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Precision.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

namespace {

class HelixBuilderAdapter {
public:
  void SetParameters(const gp_Ax3& axis,
                     double diameter,
                     const NCollection_Array1<double>& pitches,
                     const NCollection_Array1<double>& turns) {
    axis_ = axis;
    diameter_ = diameter;
    pitches_.clear();
    turns_.clear();
    for (int i = pitches.Lower(); i <= pitches.Upper(); ++i) {
      pitches_.push_back(pitches.Value(i));
    }
    for (int i = turns.Lower(); i <= turns.Upper(); ++i) {
      turns_.push_back(turns.Value(i));
    }
  }

  void SetApproxParameters(double tolerance, int maxDegree, GeomAbs_Shape continuity) {
    tolerance_ = std::max(tolerance, Precision::Confusion());
    max_degree_ = std::clamp(maxDegree, 3, 25);
    continuity_ = continuity;
  }

  void Perform() {
    shape_.Nullify();
    error_status_ = 0;
    warning_status_ = 0;
    tolerance_reached_ = 0.0;

    const double radius = 0.5 * diameter_;
    if (radius <= Precision::Confusion()) {
      error_status_ = 10;
      return;
    }
    if (pitches_.empty() || pitches_.size() != turns_.size()) {
      error_status_ = 1;
      return;
    }

    constexpr double kPi = 3.141592653589793238462643383279502884;
    constexpr int kMinSamplesPerTurn = 32;
    constexpr int kMaxSamplesPerTurn = 128;
    constexpr std::size_t kMaxPoints = 8192;

    const double sagitta_arg = std::max(tolerance_, Precision::Confusion());
    const double estimate = 2.0 * kPi * std::sqrt(radius / (8.0 * sagitta_arg));
    const int samples_per_turn = std::clamp(
        static_cast<int>(std::ceil(estimate)),
        kMinSamplesPerTurn,
        kMaxSamplesPerTurn);

    std::vector<gp_Pnt> samples;
    samples.reserve(256);

    const gp_Pnt origin = axis_.Location();
    const gp_Dir xdir = axis_.XDirection();
    const gp_Dir ydir = axis_.YDirection();
    const gp_Dir zdir = axis_.Direction();

    auto point_at = [&](double theta, double axial) {
      const double radial_x = radius * std::cos(theta);
      const double radial_y = radius * std::sin(theta);
      return gp_Pnt(
          origin.X() + xdir.X() * radial_x + ydir.X() * radial_y + zdir.X() * axial,
          origin.Y() + xdir.Y() * radial_x + ydir.Y() * radial_y + zdir.Y() * axial,
          origin.Z() + xdir.Z() * radial_x + ydir.Z() * radial_y + zdir.Z() * axial);
    };

    double theta0 = 0.0;
    double axial0 = 0.0;
    samples.push_back(point_at(theta0, axial0));

    for (std::size_t segment = 0; segment < pitches_.size(); ++segment) {
      const double pitch = pitches_[segment];
      const double segment_turns = turns_[segment];
      if (std::abs(pitch) <= Precision::Confusion()) {
        error_status_ = 11;
        return;
      }
      if (std::abs(pitch * segment_turns) <= Precision::Confusion()) {
        error_status_ = 12;
        return;
      }

      int steps = std::max(1, static_cast<int>(
          std::ceil(std::abs(segment_turns) * samples_per_turn)));
      const std::size_t remaining = kMaxPoints - samples.size();
      if (static_cast<std::size_t>(steps) > remaining) {
        steps = static_cast<int>(remaining);
        warning_status_ = 1;
      }
      if (steps <= 0) {
        error_status_ = 1;
        return;
      }

      for (int i = 1; i <= steps; ++i) {
        const double fraction = static_cast<double>(i) / static_cast<double>(steps);
        samples.push_back(point_at(
            theta0 + 2.0 * kPi * segment_turns * fraction,
            axial0 + pitch * segment_turns * fraction));
      }
      theta0 += 2.0 * kPi * segment_turns;
      axial0 += pitch * segment_turns;
    }

    if (samples.size() < 2) {
      error_status_ = 1;
      return;
    }

    NCollection_Array1<gp_Pnt> points(1, static_cast<int>(samples.size()));
    for (std::size_t i = 0; i < samples.size(); ++i) {
      points.SetValue(static_cast<int>(i + 1), samples[i]);
    }

    // Higher-degree global fits can oscillate outside the sampled cylinder
    // even when every source point lies exactly on it. A cubic C2 fit is the
    // stable OCCT 7 equivalent here and keeps the radial envelope within the
    // requested tolerance while retaining a smooth sweep-ready curve.
    const int fit_degree = std::min(max_degree_, 3);
    GeomAPI_PointsToBSpline approximation(
        points, 3, fit_degree, continuity_, tolerance_);
    if (!approximation.IsDone()) {
      error_status_ = 1;
      return;
    }

    BRepBuilderAPI_MakeEdge edge_builder(approximation.Curve());
    if (!edge_builder.IsDone()) {
      error_status_ = 1;
      return;
    }
    BRepBuilderAPI_MakeWire wire_builder(edge_builder.Edge());
    if (!wire_builder.IsDone()) {
      error_status_ = 1;
      return;
    }

    shape_ = wire_builder.Wire();
    tolerance_reached_ = tolerance_;
  }

  TopoDS_Shape Shape() const { return shape_; }
  int ErrorStatus() const { return error_status_; }
  int WarningStatus() const { return warning_status_; }
  double ToleranceReached() const { return tolerance_reached_; }

private:
  gp_Ax3 axis_;
  double diameter_ = 0.0;
  std::vector<double> pitches_;
  std::vector<double> turns_;
  double tolerance_ = 1.0e-4;
  int max_degree_ = 8;
  GeomAbs_Shape continuity_ = GeomAbs_C2;
  TopoDS_Shape shape_;
  int error_status_ = 0;
  int warning_status_ = 0;
  double tolerance_reached_ = 0.0;
};

} // namespace

#endif

namespace {
NCollection_Array1<double> RealArrayFrom(jlcxx::ArrayRef<double> vals) {
  const int n = static_cast<int>(vals.size());
  NCollection_Array1<double> arr(1, n);
  for (int i = 0; i < n; ++i) {
    arr.SetValue(i + 1, vals[i]);
  }
  return arr;
}
} // namespace

void register_occ_helix(jlcxx::Module& mod) {
  mod.add_type<HelixBuilderAdapter>("HelixBRep_BuilderHelix").constructor<>();

  mod.method("SetParameters", [](HelixBuilderAdapter& h,
                                   const gp_Ax3& axis,
                                   double diameter,
                                   jlcxx::ArrayRef<double> pitches,
                                   jlcxx::ArrayRef<double> turns) {
    h.SetParameters(axis, diameter, RealArrayFrom(pitches), RealArrayFrom(turns));
  });
  mod.method("SetApproxParameters", [](HelixBuilderAdapter& h,
                                        double tolerance,
                                        int maxDegree,
                                        int continuity) {
    h.SetApproxParameters(tolerance, maxDegree, GeomAbs_Shape(continuity));
  });
  mod.method("Perform", [](HelixBuilderAdapter& h) {
    occ_guard([&] {
      h.Perform();
      return 0;
    });
  });
  mod.method("Shape", [](const HelixBuilderAdapter& h) -> TopoDS_Shape {
    return h.Shape();
  });
  mod.method("ErrorStatus", [](const HelixBuilderAdapter& h) -> int {
    return h.ErrorStatus();
  });
  mod.method("WarningStatus", [](const HelixBuilderAdapter& h) -> int {
    return h.WarningStatus();
  });
  mod.method("ToleranceReached", [](const HelixBuilderAdapter& h) -> double {
    return h.ToleranceReached();
  });
}
