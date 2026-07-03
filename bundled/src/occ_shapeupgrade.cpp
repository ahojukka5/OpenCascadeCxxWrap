// occ_shapeupgrade.cpp — 1:1 CxxWrap bindings for ShapeUpgrade_UnifySameDomain,
// ShapeUpgrade_ShapeDivideClosed, and ShapeUpgrade_RemoveInternalWires (import
// healing: splitting closed/periodic faces, removing stray internal wires).
#include <jlcxx/jlcxx.hpp>

#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <ShapeUpgrade_ShapeDivideClosed.hxx>
#include <ShapeUpgrade_RemoveInternalWires.hxx>
#include <TopoDS_Shape.hxx>

namespace jlcxx {
  template<> struct IsMirroredType<ShapeUpgrade_UnifySameDomain> : std::false_type { };
  template<> struct IsMirroredType<ShapeUpgrade_ShapeDivideClosed> : std::false_type { };
  template<> struct IsMirroredType<ShapeUpgrade_RemoveInternalWires> : std::false_type { };
}

void register_occ_shapeupgrade(jlcxx::Module& mod) {
  mod.add_type<ShapeUpgrade_UnifySameDomain>("ShapeUpgrade_UnifySameDomain")
     .constructor<>()
     .constructor<const TopoDS_Shape&, bool, bool, bool>();

  mod.method("Initialize", [](ShapeUpgrade_UnifySameDomain& u, const TopoDS_Shape& shape,
                               bool unifyEdges, bool unifyFaces, bool concatBSplines) {
    u.Initialize(shape, unifyEdges, unifyFaces, concatBSplines);
  });
  mod.method("AllowInternalEdges", [](ShapeUpgrade_UnifySameDomain& u, bool value) {
    u.AllowInternalEdges(value);
  });
  mod.method("KeepShape", [](ShapeUpgrade_UnifySameDomain& u, const TopoDS_Shape& shape) {
    u.KeepShape(shape);
  });
  mod.method("SetSafeInputMode", [](ShapeUpgrade_UnifySameDomain& u, bool value) {
    u.SetSafeInputMode(value);
  });
  mod.method("SetLinearTolerance", [](ShapeUpgrade_UnifySameDomain& u, double value) {
    u.SetLinearTolerance(value);
  });
  mod.method("SetAngularTolerance", [](ShapeUpgrade_UnifySameDomain& u, double value) {
    u.SetAngularTolerance(value);
  });
  mod.method("Build", [](ShapeUpgrade_UnifySameDomain& u) {
    u.Build();
  });
  mod.method("Shape", [](const ShapeUpgrade_UnifySameDomain& u) -> TopoDS_Shape {
    return u.Shape();
  });

  mod.add_type<ShapeUpgrade_ShapeDivideClosed>("ShapeUpgrade_ShapeDivideClosed")
     .constructor<const TopoDS_Shape&>();
  mod.method("SetNbSplitPoints", [](ShapeUpgrade_ShapeDivideClosed& d, int num) {
    d.SetNbSplitPoints(num);
  });
  mod.method("SetPrecision", [](ShapeUpgrade_ShapeDivideClosed& d, double prec) {
    d.SetPrecision(prec);
  });
  mod.method("Perform", [](ShapeUpgrade_ShapeDivideClosed& d, bool newContext) -> bool {
    return bool(d.Perform(newContext));
  });
  mod.method("Result", [](const ShapeUpgrade_ShapeDivideClosed& d) -> TopoDS_Shape {
    return d.Result();
  });

  mod.add_type<ShapeUpgrade_RemoveInternalWires>("ShapeUpgrade_RemoveInternalWires")
     .constructor<const TopoDS_Shape&>();
  mod.method("SetMinArea", [](ShapeUpgrade_RemoveInternalWires& r, double area) {
    r.MinArea() = area;
  });
  mod.method("SetRemoveFaceMode", [](ShapeUpgrade_RemoveInternalWires& r, bool value) {
    r.RemoveFaceMode() = value;
  });
  mod.method("Perform", [](ShapeUpgrade_RemoveInternalWires& r) -> bool {
    return bool(r.Perform());
  });
  mod.method("GetResult", [](const ShapeUpgrade_RemoveInternalWires& r) -> TopoDS_Shape {
    return r.GetResult();
  });
}
