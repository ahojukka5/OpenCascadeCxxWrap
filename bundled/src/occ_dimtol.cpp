// occ_dimtol.cpp — 1:1 CxxWrap bindings for GD&T/PMI: dimensions, geometric
// tolerances, and datums, both read (already present in an imported
// STEP/IGES XCAF document, via XCAFDimTolObjects_Tool and its three
// data-object types) and write (Round 27: authoring new GD&T annotations).
//
// The write path is a *different*, older OCAF-attribute class family than
// the read-friendly XCAFDimTolObjects_Tool (which has no Set/Add methods of
// its own -- confirmed empty by inspection, genuinely read-only by design).
// Verified directly against XCAFDimTolObjects_Tool.cxx (the implementation
// backing the read side already bound below) that GetDimensions/
// GetGeomTolerances/RefDatum walk XCAFDoc_Dimension/GeomTolerance/Datum
// TDF_Attributes and call their GetObject() -- and against
// STEPCAFControl_Writer.cxx that STEP export reads GD&T out through the
// exact same attributes' GetObject(). So the write path is: allocate a
// label via XCAFDoc_DimTolTool::AddDimension()/AddGeomTolerance()/
// AddDatum(), attach data via XCAFDoc_Dimension::Set(label)->SetObject(...),
// then link it to the shape(s) it annotates via
// XCAFDoc_DimTolTool::SetDimension/SetGeomTolerance/SetDatum.
//
// Needs two small prerequisite bindings: TCollection_HAsciiString (every
// name/description string field in this object graph) and
// NCollection_HArray1<double> (DimensionObject::GetValues()'s raw value
// array) -- neither previously bound anywhere in this codebase, both
// trivial (same Handle-factory idiom as ShapeCustom_RestrictionParameters
// in occ_shapecustom.cpp).
//
// STEP round-trip verified empirically (write with a fresh STEPCAFControl_Writer,
// AP242DIS schema selected via Interface_Static_SetCVal("write.step.schema",
// "AP242DIS") -- note the writer/reader must already be constructed once
// before this static parameter exists at all, confirmed via Interface_Static_IVal
// reading 0 beforehand), then re-read with a fresh STEPCAFControl_Reader:
// authored Dimension and GeomTolerance objects round-trip exactly (type +
// value confirmed byte-identical after the round trip). A hand-authored
// Datum round-trips correctly in-memory (same document, no STEP involved)
// but its cross-reference to the GeomTolerance it defines the reference
// frame for does not survive a STEP write/read cycle even after calling the
// documented XCAFDoc_DimTolTool::SetDatumToGeomTol link -- the DATUM STEP
// entity itself is confirmed present in the written file, but the
// tolerance's own STEP entity (e.g. POSITION_TOLERANCE) is written without a
// datum-reference argument regardless. This looks like a genuine OCCT
// writer limitation/quirk in this schema corner, not a binding defect (the
// call is a direct 1:1 match to the documented signature); flagged, not
// silently claimed as fully round-tripping.
//
// Sequence-of-Handle results (GetDimensions, GetGeomTolerances, ...) are
// exposed via the same "recompute a local sequence inside the lambda, expose
// Count/Get(index)" idiom already used throughout occ_xcaf.cpp for
// TDF_LabelSequence-returning methods.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <TCollection_HAsciiString.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <NCollection_Sequence.hxx>
#include <NCollection_DataMap.hxx>
#include <XCAFDimTolObjects_Tool.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_GeomTolerance.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

using HArray1OfReal = TColStd_HArray1OfReal;

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

  // ---- Write-side authoring (Round 27) ----

  // New*: the three data-object types are already add_type'd above (read-only
  // since Round 18), but with no constructor -- add factories so Julia can
  // build a fresh object to populate and attach, same NewRestrictionParameters/
  // NewHAsciiString idiom already used elsewhere in this codebase.
  mod.method("NewDimensionObject", []() -> Handle(XCAFDimTolObjects_DimensionObject) {
    return new XCAFDimTolObjects_DimensionObject();
  });
  mod.method("NewGeomToleranceObject", []() -> Handle(XCAFDimTolObjects_GeomToleranceObject) {
    return new XCAFDimTolObjects_GeomToleranceObject();
  });
  mod.method("NewDatumObject", []() -> Handle(XCAFDimTolObjects_DatumObject) {
    return new XCAFDimTolObjects_DatumObject();
  });

  // Set* mutators, mirroring the Get* accessors already bound above.
  mod.method("SetSemanticName", [](const Handle(XCAFDimTolObjects_DimensionObject)& d,
                                    const Handle(TCollection_HAsciiString)& name) { d->SetSemanticName(name); });
  mod.method("SetType", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, int type) {
    d->SetType(XCAFDimTolObjects_DimensionType(type));
  });
  mod.method("SetValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, double v) { d->SetValue(v); });
  mod.method("SetUpperBound", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, double v) { d->SetUpperBound(v); });
  mod.method("SetLowerBound", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, double v) { d->SetLowerBound(v); });
  mod.method("SetUpperTolValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, double v) -> bool {
    return bool(d->SetUpperTolValue(v));
  });
  mod.method("SetLowerTolValue", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, double v) -> bool {
    return bool(d->SetLowerTolValue(v));
  });
  mod.method("SetPath", [](const Handle(XCAFDimTolObjects_DimensionObject)& d, const TopoDS_Edge& e) { d->SetPath(e); });

  mod.method("SetSemanticName", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g,
                                    const Handle(TCollection_HAsciiString)& name) { g->SetSemanticName(name); });
  mod.method("SetType", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g, int type) {
    g->SetType(XCAFDimTolObjects_GeomToleranceType(type));
  });
  mod.method("SetTypeOfValue", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g, int type) {
    g->SetTypeOfValue(XCAFDimTolObjects_GeomToleranceTypeValue(type));
  });
  mod.method("SetValue", [](const Handle(XCAFDimTolObjects_GeomToleranceObject)& g, double v) { g->SetValue(v); });

  mod.method("SetSemanticName", [](const Handle(XCAFDimTolObjects_DatumObject)& d,
                                    const Handle(TCollection_HAsciiString)& name) { d->SetSemanticName(name); });
  mod.method("SetName", [](const Handle(XCAFDimTolObjects_DatumObject)& d,
                            const Handle(TCollection_HAsciiString)& name) { d->SetName(name); });
  mod.method("SetDatumTarget", [](const Handle(XCAFDimTolObjects_DatumObject)& d, const TopoDS_Shape& s) {
    d->SetDatumTarget(s);
  });
  mod.method("SetPosition", [](const Handle(XCAFDimTolObjects_DatumObject)& d, int pos) { d->SetPosition(pos); });

  // XCAFDoc_Dimension/GeomTolerance/Datum: the OCAF TDF_Attribute layer that
  // actually carries the data object on a label -- STEPCAFControl_Writer.cxx
  // was confirmed (this round) to read GD&T out through exactly these
  // attributes' GetObject(), the same mechanism XCAFDimTolObjects_Tool's own
  // read-side implementation (XCAFDimTolObjects_Tool.cxx) uses.
  mod.add_type<XCAFDoc_Dimension>("XCAFDoc_Dimension");
  mod.method("XCAFDoc_Dimension_Set", [](const TDF_Label& label) -> Handle(XCAFDoc_Dimension) {
    return XCAFDoc_Dimension::Set(label);
  });
  mod.method("SetObject", [](const Handle(XCAFDoc_Dimension)& attr,
                              const Handle(XCAFDimTolObjects_DimensionObject)& obj) { attr->SetObject(obj); });

  mod.add_type<XCAFDoc_GeomTolerance>("XCAFDoc_GeomTolerance");
  mod.method("XCAFDoc_GeomTolerance_Set", [](const TDF_Label& label) -> Handle(XCAFDoc_GeomTolerance) {
    return XCAFDoc_GeomTolerance::Set(label);
  });
  mod.method("SetObject", [](const Handle(XCAFDoc_GeomTolerance)& attr,
                              const Handle(XCAFDimTolObjects_GeomToleranceObject)& obj) { attr->SetObject(obj); });

  mod.add_type<XCAFDoc_Datum>("XCAFDoc_Datum");
  mod.method("XCAFDoc_Datum_Set", [](const TDF_Label& label) -> Handle(XCAFDoc_Datum) {
    return XCAFDoc_Datum::Set(label);
  });
  mod.method("SetObject", [](const Handle(XCAFDoc_Datum)& attr,
                              const Handle(XCAFDimTolObjects_DatumObject)& obj) { attr->SetObject(obj); });

  // XCAFDoc_DimTolTool: label allocation (AddDimension/AddGeomTolerance/AddDatum)
  // and shape linkage (SetDimension/SetGeomTolerance/SetDatum) -- the same
  // XCAFDoc_DocumentTool-factory idiom already used for ShapeTool/ColorTool/
  // MaterialTool in occ_xcaf.cpp.
  mod.add_type<XCAFDoc_DimTolTool>("XCAFDoc_DimTolTool");
  mod.method("XCAFDoc_DimTolTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_DimTolTool) {
    return XCAFDoc_DocumentTool::DimTolTool(doc->Main());
  });
  mod.method("AddDimension", [](const Handle(XCAFDoc_DimTolTool)& t) -> TDF_Label { return t->AddDimension(); });
  mod.method("AddGeomTolerance", [](const Handle(XCAFDoc_DimTolTool)& t) -> TDF_Label { return t->AddGeomTolerance(); });
  mod.method("AddDatum", [](const Handle(XCAFDoc_DimTolTool)& t) -> TDF_Label { return t->AddDatum(); });
  mod.method("XCAFDoc_DimTolTool_SetDimension",
             [](const Handle(XCAFDoc_DimTolTool)& t, const TDF_Label& shapeL, const TDF_Label& dimL) {
    t->SetDimension(shapeL, dimL);
  });
  mod.method("XCAFDoc_DimTolTool_SetGeomTolerance",
             [](const Handle(XCAFDoc_DimTolTool)& t, const TDF_Label& shapeL, const TDF_Label& geomTolL) {
    t->SetGeomTolerance(shapeL, geomTolL);
  });
  mod.method("XCAFDoc_DimTolTool_SetDatum",
             [](const Handle(XCAFDoc_DimTolTool)& t, const TDF_Label& shapeL, const TDF_Label& datumL) {
    NCollection_Sequence<TDF_Label> shapeLabels;
    shapeLabels.Append(shapeL);
    t->SetDatum(shapeLabels, datumL);
  });
  // XCAFDimTolObjects_Tool's own read side (confirmed via its .cxx) retrieves
  // datums as children of a *geometric tolerance*'s datum-reference list
  // (GetDatumOfTolerLabels), not via the shape-linked SetDatum above -- a
  // datum only round-trips through STEP AP242 GD&T once it is also linked to
  // the tolerance it defines the reference frame for.
  mod.method("SetDatumToGeomTol", [](const Handle(XCAFDoc_DimTolTool)& t,
                                      const TDF_Label& datumL, const TDF_Label& geomTolL) {
    t->SetDatumToGeomTol(datumL, geomTolL);
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
