// occ_analysis.cpp — 1:1 CxxWrap bindings for BRepCheck_Analyzer, ShapeFix_Shape,
// and BRep_Tool free functions.
#include "occ_handle_traits.hpp"
#include "occ_exception.hpp"
#include <jlcxx/jlcxx.hpp>

#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

#include <stdexcept>

void register_occ_analysis(jlcxx::Module& mod)
{
    // ------------------------------------------------------------------ types
    mod.add_type<BRepCheck_Analyzer>("BRepCheck_Analyzer")
        .constructor([](const TopoDS_Shape& s) -> BRepCheck_Analyzer* {
          return occ_guard([&]{ return new BRepCheck_Analyzer(s); });
        })
        .constructor([](const TopoDS_Shape& s, bool geomControls) -> BRepCheck_Analyzer* {
          return occ_guard([&]{ return new BRepCheck_Analyzer(s, geomControls); });
        });

    mod.add_type<ShapeFix_Shape>("ShapeFix_Shape")
        .constructor<>()
        .constructor<const TopoDS_Shape&>();
    // ShapeFix_Shape's Context()/History() (-> BRepTools_History) are bound in
    // occ_history.cpp, which is registered after both this file (ShapeFix_Shape
    // must already be add_type'd) and BRepTools_History's own add_type.

    // --------------------------------------------------------------- methods

    // BRepCheck_Analyzer
    mod.method("IsValid", [](const BRepCheck_Analyzer& a) {
        return bool(a.IsValid());
    });
    mod.method("IsValid", [](const BRepCheck_Analyzer& a, const TopoDS_Shape& shape) {
        return bool(a.IsValid(shape));
    });

    // ShapeFix_Shape
    mod.method("Init", [](ShapeFix_Shape& fixer, const TopoDS_Shape& shape) {
        fixer.Init(shape);
    });
    mod.method("Perform", [](ShapeFix_Shape& fixer) {
        return bool(fixer.Perform());
    });
    mod.method("Shape", [](ShapeFix_Shape& fixer) {
        return fixer.Shape();
    });
    mod.method("SetPrecision", [](ShapeFix_Shape& fixer, double prec) {
        fixer.SetPrecision(prec);
    });
    mod.method("SetMinTolerance", [](ShapeFix_Shape& fixer, double tol) {
        fixer.SetMinTolerance(tol);
    });
    mod.method("SetMaxTolerance", [](ShapeFix_Shape& fixer, double tol) {
        fixer.SetMaxTolerance(tol);
    });

    // BRep_Tool free functions
    mod.method("BRep_Tool_ToleranceEdge",   [](const TopoDS_Edge& e)   { return BRep_Tool::Tolerance(e); });
    mod.method("BRep_Tool_ToleranceFace",   [](const TopoDS_Face& f)   { return BRep_Tool::Tolerance(f); });
    mod.method("BRep_Tool_ToleranceVertex", [](const TopoDS_Vertex& v) { return BRep_Tool::Tolerance(v); });

    mod.method("BRep_Tool_IsClosed", [](const TopoDS_Shape& s) {
        return bool(BRep_Tool::IsClosed(s));
    });
    mod.method("BRep_Tool_IsClosedEdgeFace", [](const TopoDS_Edge& e, const TopoDS_Face& f) {
        return bool(BRep_Tool::IsClosed(e, f));
    });

    mod.method("BRep_Tool_Degenerated",   [](const TopoDS_Edge& e) { return bool(BRep_Tool::Degenerated(e)); });
    mod.method("BRep_Tool_SameParameter", [](const TopoDS_Edge& e) { return bool(BRep_Tool::SameParameter(e)); });

    mod.method("BRep_Tool_FirstParameter", [](const TopoDS_Edge& e) {
        double first, last;
        BRep_Tool::Range(e, first, last);
        return first;
    });
    mod.method("BRep_Tool_LastParameter", [](const TopoDS_Edge& e) {
        double first, last;
        BRep_Tool::Range(e, first, last);
        return last;
    });

    // Exact 2D edge representation in one face's parametric domain. The edge
    // argument retains its TopoDS orientation, which is significant for a seam:
    // the two oriented uses of one global edge select the two distinct p-curves.
    mod.method("BRep_Tool_CurveOnSurface", [](const TopoDS_Edge& e, const TopoDS_Face& f) -> Handle(Geom2d_Curve) {
        return occ_guard([&]() -> Handle(Geom2d_Curve) {
            double first, last;
            bool stored = false;
            Handle(Geom2d_Curve) curve = BRep_Tool::CurveOnSurface(e, f, first, last, &stored);
            if (curve.IsNull()) {
                throw std::runtime_error(
                    "BRep_Tool_CurveOnSurface: edge has no p-curve on the supplied face");
            }
            return curve;
        });
    });
    mod.method("BRep_Tool_FirstParameterOnSurface", [](const TopoDS_Edge& e, const TopoDS_Face& f) {
        return occ_guard([&]() {
            double first, last;
            BRep_Tool::Range(e, f, first, last);
            return first;
        });
    });
    mod.method("BRep_Tool_LastParameterOnSurface", [](const TopoDS_Edge& e, const TopoDS_Face& f) {
        return occ_guard([&]() {
            double first, last;
            BRep_Tool::Range(e, f, first, last);
            return last;
        });
    });
    mod.method("BRep_Tool_CurveOnSurfaceStored", [](const TopoDS_Edge& e, const TopoDS_Face& f) {
        return occ_guard([&]() {
            double first, last;
            bool stored = false;
            Handle(Geom2d_Curve) curve = BRep_Tool::CurveOnSurface(e, f, first, last, &stored);
            if (curve.IsNull()) {
                throw std::runtime_error(
                    "BRep_Tool_CurveOnSurfaceStored: edge has no p-curve on the supplied face");
            }
            return stored;
        });
    });
    mod.method("BRep_Tool_ParameterVertexEdgeFace", [](
        const TopoDS_Vertex& v,
        const TopoDS_Edge& e,
        const TopoDS_Face& f) {
        return occ_guard([&]() { return BRep_Tool::Parameter(v, e, f); });
    });

    mod.method("BRep_Tool_Curve", [](const TopoDS_Edge& e) -> Handle(Geom_Curve) {
        return occ_guard([&]() -> Handle(Geom_Curve) {
            double first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(e, first, last);
            if (curve.IsNull()) {
                throw std::runtime_error(
                    "BRep_Tool_Curve: edge has no attached 3D Geom_Curve; "
                    "operations requiring a Geom_BSplineCurve cannot use it");
            }
            return curve;
        });
    });
    mod.method("BRep_Tool_Surface", [](const TopoDS_Face& f) -> Handle(Geom_Surface) {
        return BRep_Tool::Surface(f);
    });
}