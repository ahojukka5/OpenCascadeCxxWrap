// occ_dimtol.cpp — 1:1 CxxWrap bindings for a read-only GD&T/PMI slice:
// dimensions, geometric tolerances, and datums already present in an
// imported STEP/IGES XCAF document (XCAFDimTolObjects_Tool and its three
// data-object types). Write-side authoring is out of scope for this round.
//
// Needs two small prerequisite bindings: TCollection_HAsciiString (every
// name/description string field in this object graph) and
// NCollection_HArray1<double> (DimensionObject::GetValues()'s raw value
// array) -- neither previously bound anywhere in this codebase, both
// trivial (same Handle-factory idiom as ShapeCustom_RestrictionParameters
// in occ_shapecustom.cpp).
//
// Sequence-of-Handle results (GetDimensions, GetGeomTolerances, ...) are
// exposed via the same "recompute a local sequence inside the lambda, expose
// Count/Get(index)" idiom already used throughout occ_xcaf.cpp for
// TDF_LabelSequence-returning methods.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <TCollection_HAsciiString.hxx>
#include <NCollection_HArray1.hxx>
#include <NCollection_Sequence.hxx>
#include <NCollection_DataMap.hxx>
#include <XCAFDimTolObjects_Tool.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

using HArray1OfReal = NCollection_HArray1<double>;

void register_occ_dimtol(jlcxx::Module& mod) {
  // TCollection_HAsciiString itself is add_type'd + given its NewHAsciiString/ToCString
  // factory in occ_xcaf.cpp now (Round 21 needed it there too, and occ_xcaf.cpp registers
  // earlier than this file -- registration order matters for Handle(T) factories).

  // ---- NCollection_HArray1<double>: DimensionObject::GetValues()'s raw array ----
  mod.add_type<HArray1OfReal>("HArray1OfReal");
  mod.method("Length", [](const Handle(HArray1OfReal)& a) -> int {
    return a.IsNull() ? 0 : a->Length();
  });
  mod.method("Value", [](const Handle(HArray1OfReal)& a, int i) -> double {
    return a->Value(i);
  });

  // ---- XCAFDimTolObjects_Tool ----
  // The three data-object types are only ever received as Handle(T) results from
  // XCAFDimTolObjects_Tool's sequence accessors, never constructed from Julia -- but
  // jlcxx still needs each add_type'd before any Handle(T) of that type can be
  // wrapped/returned (same "No appropriate factory" pitfall as XCAFDoc_VisMaterial).
  mod.add_type<XCAFDimTolObjects_DimensionObject>("XCAFDimTolObjects_DimensionObject");
  mod.add_type<XCAFDimTolObjects_GeomToleranceObject>("XCAFDimTolObjects_GeomToleranceObject");
  mod.add_type<XCAFDimTolObjects_DatumObject>("XCAFDimTolObjects_DatumObject");

  // Count/Get accessors above return a null handle for "not found" (e.g. RefDatum on a
  // shape with no datum, or an out-of-range index) -- callers need IsNull to detect it.
  mod.method("IsNull", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> bool { return d.IsNull(); });
  mod.method("IsNull", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g) -> bool { return g.IsNull(); });
  mod.method("IsNull", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> bool { return d.IsNull(); });

  mod.add_type<XCAFDimTolObjects_Tool>("XCAFDimTolObjects_Tool")
     .constructor<const Handle(TDocStd_Document)&>();

  mod.method("XCAFDimTolObjects_Tool_NbDimensions", [](const XCAFDimTolObjects_Tool& t) -> int {
    NCollection_Sequence<Handle(XCAFDimTolObjects_DimensionObject)> seq;
    t.GetDimensions(seq);
    return seq.Length();
  });
  mod.method("XCAFDimTolObjects_Tool_Dimension",
             [](const XCAFDimTolObjects_Tool& t, int i) -> Handle(XCAFDimTolObjects_DimensionObject) {
    NCollection_Sequence<Handle(XCAFDimTolObjects_DimensionObject)> seq;
    t.GetDimensions(seq);
    return (i < 1 || i > seq.Length()) ? Handle(XCAFDimTolObjects_DimensionObject)() : seq.Value(i);
  });

  mod.method("XCAFDimTolObjects_Tool_NbRefDimensions",
             [](const XCAFDimTolObjects_Tool& t, const TopoDS_Shape& s) -> int {
    NCollection_Sequence<Handle(XCAFDimTolObjects_DimensionObject)> seq;
    t.GetRefDimensions(s, seq);
    return seq.Length();
  });
  mod.method("XCAFDimTolObjects_Tool_RefDimension",
             [](const XCAFDimTolObjects_Tool& t, const TopoDS_Shape& s, int i) -> Handle(XCAFDimTolObjects_DimensionObject) {
    NCollection_Sequence<Handle(XCAFDimTolObjects_DimensionObject)> seq;
    t.GetRefDimensions(s, seq);
    return (i < 1 || i > seq.Length()) ? Handle(XCAFDimTolObjects_DimensionObject)() : seq.Value(i);
  });

  mod.method("XCAFDimTolObjects_Tool_NbGeomTolerances", [](const XCAFDimTolObjects_Tool& t) -> int {
    NCollection_Sequence<Handle(XCAFDimTolObjects_GeomToleranceObject)> tolSeq;
    NCollection_Sequence<Handle(XCAFDimTolObjects_DatumObject)> datumSeq;
    NCollection_DataMap<Handle(XCAFDimTolObjects_GeomToleranceObject), Handle(XCAFDimTolObjects_DatumObject)> map;
    t.GetGeomTolerances(tolSeq, datumSeq, map);
    return tolSeq.Length();
  });
  mod.method("XCAFDimTolObjects_Tool_GeomTolerance",
             [](const XCAFDimTolObjects_Tool& t, int i) -> Handle(XCAFDimTolObjects_GeomToleranceObject) {
    NCollection_Sequence<Handle(XCAFDimTolObjects_GeomToleranceObject)> tolSeq;
    NCollection_Sequence<Handle(XCAFDimTolObjects_DatumObject)> datumSeq;
    NCollection_DataMap<Handle(XCAFDimTolObjects_GeomToleranceObject), Handle(XCAFDimTolObjects_DatumObject)> map;
    t.GetGeomTolerances(tolSeq, datumSeq, map);
    return (i < 1 || i > tolSeq.Length()) ? Handle(XCAFDimTolObjects_GeomToleranceObject)() : tolSeq.Value(i);
  });
  mod.method("XCAFDimTolObjects_Tool_NbDatums", [](const XCAFDimTolObjects_Tool& t) -> int {
    NCollection_Sequence<Handle(XCAFDimTolObjects_GeomToleranceObject)> tolSeq;
    NCollection_Sequence<Handle(XCAFDimTolObjects_DatumObject)> datumSeq;
    NCollection_DataMap<Handle(XCAFDimTolObjects_GeomToleranceObject), Handle(XCAFDimTolObjects_DatumObject)> map;
    t.GetGeomTolerances(tolSeq, datumSeq, map);
    return datumSeq.Length();
  });
  mod.method("XCAFDimTolObjects_Tool_Datum",
             [](const XCAFDimTolObjects_Tool& t, int i) -> Handle(XCAFDimTolObjects_DatumObject) {
    NCollection_Sequence<Handle(XCAFDimTolObjects_GeomToleranceObject)> tolSeq;
    NCollection_Sequence<Handle(XCAFDimTolObjects_DatumObject)> datumSeq;
    NCollection_DataMap<Handle(XCAFDimTolObjects_GeomToleranceObject), Handle(XCAFDimTolObjects_DatumObject)> map;
    t.GetGeomTolerances(tolSeq, datumSeq, map);
    return (i < 1 || i > datumSeq.Length()) ? Handle(XCAFDimTolObjects_DatumObject)() : datumSeq.Value(i);
  });

  mod.method("XCAFDimTolObjects_Tool_RefDatum",
             [](const XCAFDimTolObjects_Tool& t, const TopoDS_Shape& s) -> Handle(XCAFDimTolObjects_DatumObject) {
    Handle(XCAFDimTolObjects_DatumObject) datum;
    t.GetRefDatum(s, datum);
    return datum;
  });

  // ---- XCAFDimTolObjects_DimensionObject (read-only slice) ----
  mod.method("GetSemanticName", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> Handle(TCollection_HAsciiString) {
    return d->GetSemanticName();
  });
  mod.method("GetType", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> int {
    return int(d->GetType());
  });
  mod.method("GetValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> double {
    return d->GetValue();
  });
  mod.method("GetValues", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> Handle(HArray1OfReal) {
    return d->GetValues();
  });
  mod.method("IsDimWithRange", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> bool {
    return bool(d->IsDimWithRange());
  });
  mod.method("GetUpperBound", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> double {
    return d->GetUpperBound();
  });
  mod.method("GetLowerBound", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> double {
    return d->GetLowerBound();
  });
  mod.method("IsDimWithPlusMinusTolerance", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> bool {
    return bool(d->IsDimWithPlusMinusTolerance());
  });
  mod.method("GetUpperTolValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> double {
    return d->GetUpperTolValue();
  });
  mod.method("GetLowerTolValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> double {
    return d->GetLowerTolValue();
  });
  mod.method("GetPath", [](const Handle(XCAFDimTolObjects_DimensionObject)& d) -> TopoDS_Edge {
    return d->GetPath();
  });

  // ---- XCAFDimTolObjects_GeomToleranceObject (read-only slice) ----
  mod.method("GetSemanticName", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g) -> Handle(TCollection_HAsciiString) {
    return g->GetSemanticName();
  });
  mod.method("GetType", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g) -> int {
    return int(g->GetType());
  });
  mod.method("GetTypeOfValue", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g) -> int {
    return int(g->GetTypeOfValue());
  });
  mod.method("GetValue", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g) -> double {
    return g->GetValue();
  });

  // ---- XCAFDimTolObjects_DatumObject (read-only slice) ----
  mod.method("GetSemanticName", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> Handle(TCollection_HAsciiString) {
    return d->GetSemanticName();
  });
  mod.method("GetName", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> Handle(TCollection_HAsciiString) {
    return d->GetName();
  });
  mod.method("GetDatumTarget", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> TopoDS_Shape {
    return d->GetDatumTarget();
  });
  mod.method("GetPosition", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> int {
    return d->GetPosition();
  });
  mod.method("IsDatumTarget", [](const Handle(XCAFDimTolObjects_DatumObject)& d) -> bool {
    return bool(d->IsDatumTarget());
  });

  // ---- XCAFDimTolObjects_DimensionType (32 values) ----
  mod.method("XCAFDimTolObjects_DimensionType_Location_None", []() { return int(XCAFDimTolObjects_DimensionType_Location_None); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_CurvedDistance", []() { return int(XCAFDimTolObjects_DimensionType_Location_CurvedDistance); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToOuter", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToOuter); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToInner", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToInner); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToCenter", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToCenter); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToOuter", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToOuter); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToInner", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToInner); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToCenter", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToCenter); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToOuter", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToOuter); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToInner", []() { return int(XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToInner); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_Angular", []() { return int(XCAFDimTolObjects_DimensionType_Location_Angular); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_Oriented", []() { return int(XCAFDimTolObjects_DimensionType_Location_Oriented); });
  mod.method("XCAFDimTolObjects_DimensionType_Location_WithPath", []() { return int(XCAFDimTolObjects_DimensionType_Location_WithPath); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_CurveLength", []() { return int(XCAFDimTolObjects_DimensionType_Size_CurveLength); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_Diameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_Diameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_SphericalDiameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_SphericalDiameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_Radius", []() { return int(XCAFDimTolObjects_DimensionType_Size_Radius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_SphericalRadius", []() { return int(XCAFDimTolObjects_DimensionType_Size_SphericalRadius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius", []() { return int(XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_Thickness", []() { return int(XCAFDimTolObjects_DimensionType_Size_Thickness); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_Angular", []() { return int(XCAFDimTolObjects_DimensionType_Size_Angular); });
  mod.method("XCAFDimTolObjects_DimensionType_Size_WithPath", []() { return int(XCAFDimTolObjects_DimensionType_Size_WithPath); });
  mod.method("XCAFDimTolObjects_DimensionType_CommonLabel", []() { return int(XCAFDimTolObjects_DimensionType_CommonLabel); });
  mod.method("XCAFDimTolObjects_DimensionType_DimensionPresentation", []() { return int(XCAFDimTolObjects_DimensionType_DimensionPresentation); });

  // ---- XCAFDimTolObjects_GeomToleranceType (16 values) ----
  mod.method("XCAFDimTolObjects_GeomToleranceType_None", []() { return int(XCAFDimTolObjects_GeomToleranceType_None); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Angularity", []() { return int(XCAFDimTolObjects_GeomToleranceType_Angularity); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_CircularRunout", []() { return int(XCAFDimTolObjects_GeomToleranceType_CircularRunout); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness", []() { return int(XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Coaxiality", []() { return int(XCAFDimTolObjects_GeomToleranceType_Coaxiality); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Concentricity", []() { return int(XCAFDimTolObjects_GeomToleranceType_Concentricity); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Cylindricity", []() { return int(XCAFDimTolObjects_GeomToleranceType_Cylindricity); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Flatness", []() { return int(XCAFDimTolObjects_GeomToleranceType_Flatness); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Parallelism", []() { return int(XCAFDimTolObjects_GeomToleranceType_Parallelism); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Perpendicularity", []() { return int(XCAFDimTolObjects_GeomToleranceType_Perpendicularity); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Position", []() { return int(XCAFDimTolObjects_GeomToleranceType_Position); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_ProfileOfLine", []() { return int(XCAFDimTolObjects_GeomToleranceType_ProfileOfLine); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface", []() { return int(XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Straightness", []() { return int(XCAFDimTolObjects_GeomToleranceType_Straightness); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_Symmetry", []() { return int(XCAFDimTolObjects_GeomToleranceType_Symmetry); });
  mod.method("XCAFDimTolObjects_GeomToleranceType_TotalRunout", []() { return int(XCAFDimTolObjects_GeomToleranceType_TotalRunout); });

  // ---- XCAFDimTolObjects_GeomToleranceTypeValue (3 values) ----
  mod.method("XCAFDimTolObjects_GeomToleranceTypeValue_None", []() { return int(XCAFDimTolObjects_GeomToleranceTypeValue_None); });
  mod.method("XCAFDimTolObjects_GeomToleranceTypeValue_Diameter", []() { return int(XCAFDimTolObjects_GeomToleranceTypeValue_Diameter); });
  mod.method("XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter", []() { return int(XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter); });
}
