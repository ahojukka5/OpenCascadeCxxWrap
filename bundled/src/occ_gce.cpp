// occ_gce.cpp — 1:1 CxxWrap bindings for the `gce_Make*` convenience
// factory family: mechanical, uniform-shape constructors over already-
// bound gp_* value types. Two sub-shapes, confirmed by reading the
// headers directly:
//   - Geometric-primitive makers (Lin/Pln/Circ/Dir/Elips) inherit
//     gce_Root (IsDone()/Status()) since construction can fail (e.g.
//     degenerate input points).
//   - Transform makers (Rotation/Translation/Mirror/Scale) have no
//     failure mode (always succeed from valid gp_* inputs) and skip
//     gce_Root entirely -- just a constructor + Value().
// Only the most commonly useful 3-4 constructor overloads per class are
// bound, not the full set each header offers -- rarer overloads (e.g.
// gce_MakePln's plane-from-4-coefficients form) can be added later if
// actually needed.
#include <jlcxx/jlcxx.hpp>

#include <gce_MakeLin.hxx>
#include <gce_MakePln.hxx>
#include <gce_MakeCirc.hxx>
#include <gce_MakeDir.hxx>
#include <gce_MakeElips.hxx>
#include <gce_MakeRotation.hxx>
#include <gce_MakeTranslation.hxx>
#include <gce_MakeMirror.hxx>
#include <gce_MakeScale.hxx>

void register_occ_gce(jlcxx::Module& mod) {
  // ---- Geometric primitives (gce_Root: IsDone/Status) ----
  mod.add_type<gce_MakeLin>("gce_MakeLin")
     .constructor<const gp_Ax1&>()
     .constructor<const gp_Pnt&, const gp_Dir&>()
     .constructor<const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](const gce_MakeLin& m) -> gp_Lin { return m.Value(); });
  mod.method("IsDone", [](const gce_MakeLin& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<gce_MakePln>("gce_MakePln")
     .constructor<const gp_Ax2&>()
     .constructor<const gp_Pnt&, const gp_Dir&>()
     .constructor<const gp_Pnt&, const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](const gce_MakePln& m) -> gp_Pln { return m.Value(); });
  mod.method("IsDone", [](const gce_MakePln& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<gce_MakeCirc>("gce_MakeCirc")
     .constructor<const gp_Ax2&, double>()
     .constructor<const gp_Pnt&, const gp_Pnt&, const gp_Pnt&>()
     .constructor<const gp_Pnt&, const gp_Dir&, double>()
     .constructor<const gp_Ax1&, double>();
  mod.method("Value", [](const gce_MakeCirc& m) -> gp_Circ { return m.Value(); });
  mod.method("IsDone", [](const gce_MakeCirc& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<gce_MakeDir>("gce_MakeDir")
     .constructor<const gp_Vec&>()
     .constructor<double, double, double>()
     .constructor<const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](const gce_MakeDir& m) -> gp_Dir { return m.Value(); });
  mod.method("IsDone", [](const gce_MakeDir& m) -> bool { return bool(m.IsDone()); });

  mod.add_type<gce_MakeElips>("gce_MakeElips")
     .constructor<const gp_Ax2&, double, double>()
     .constructor<const gp_Pnt&, const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](const gce_MakeElips& m) -> gp_Elips { return m.Value(); });
  mod.method("IsDone", [](const gce_MakeElips& m) -> bool { return bool(m.IsDone()); });

  // ---- Transform makers (no gce_Root: always succeed) ----
  mod.add_type<gce_MakeRotation>("gce_MakeRotation")
     .constructor<const gp_Ax1&, double>()
     .constructor<const gp_Pnt&, const gp_Dir&, double>();
  mod.method("Value", [](const gce_MakeRotation& m) -> gp_Trsf { return m.Value(); });

  mod.add_type<gce_MakeTranslation>("gce_MakeTranslation")
     .constructor<const gp_Vec&>()
     .constructor<const gp_Pnt&, const gp_Pnt&>();
  mod.method("Value", [](const gce_MakeTranslation& m) -> gp_Trsf { return m.Value(); });

  mod.add_type<gce_MakeMirror>("gce_MakeMirror")
     .constructor<const gp_Pnt&>()
     .constructor<const gp_Ax1&>()
     .constructor<const gp_Ax2&>();
  mod.method("Value", [](const gce_MakeMirror& m) -> gp_Trsf { return m.Value(); });

  mod.add_type<gce_MakeScale>("gce_MakeScale")
     .constructor<const gp_Pnt&, double>();
  mod.method("Value", [](const gce_MakeScale& m) -> gp_Trsf { return m.Value(); });
}
