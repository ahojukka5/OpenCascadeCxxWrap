// occ_xcafview.cpp — 1:1 CxxWrap bindings for XCAFDoc_ViewTool/XCAFDoc_View/
// XCAFView_Object and XCAFDoc_ClippingPlaneTool.
//
// STEPCAFControl_Reader::ReadViews (confirmed via its own .cxx) populates
// both tables from STEP AP242 view entities today, by default -- but until
// now Monge had no way to read that data back out. Confirmed the reverse is
// NOT true: grepping STEPCAFControl_Writer.cxx for "View" finds zero
// matches, so there is no write-side round trip to test or expose here --
// this round is read/in-memory-author only, same honest scoping as prior
// read-only XCAF rounds before their write-side companion (if any) existed.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <XCAFDoc_ViewTool.hxx>
#include <XCAFDoc_View.hxx>
#include <XCAFView_Object.hxx>
#include <XCAFView_ProjectionType.hxx>
#include <XCAFDoc_ClippingPlaneTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_AsciiString.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

void register_occ_xcafview(jlcxx::Module& mod) {
  // ---- XCAFDoc_ViewTool ----
  mod.add_type<XCAFDoc_ViewTool>("XCAFDoc_ViewTool");
  mod.method("XCAFDoc_ViewTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_ViewTool) {
    return XCAFDoc_DocumentTool::ViewTool(doc->Main());
  });
  mod.method("IsView", [](const Handle(XCAFDoc_ViewTool)& t, const TDF_Label& label) -> bool {
    return bool(t->IsView(label));
  });
  mod.method("XCAFDoc_ViewTool_ViewLabelCount", [](const Handle(XCAFDoc_ViewTool)& t) -> int {
    TDF_LabelSequence seq;
    t->GetViewLabels(seq);
    return seq.Length();
  });
  mod.method("XCAFDoc_ViewTool_ViewLabel", [](const Handle(XCAFDoc_ViewTool)& t, int index) -> TDF_Label {
    TDF_LabelSequence seq;
    t->GetViewLabels(seq);
    if (index < 1 || index > seq.Length()) return TDF_Label();
    return seq.Value(index);
  });
  mod.method("AddView", [](const Handle(XCAFDoc_ViewTool)& t) -> TDF_Label { return t->AddView(); });
  mod.method("RemoveView", [](const Handle(XCAFDoc_ViewTool)& t, const TDF_Label& viewL) {
    t->RemoveView(viewL);
  });
  mod.method("XCAFDoc_ViewTool_SetView",
             [](const Handle(XCAFDoc_ViewTool)& t, const TDF_LabelSequence& shapes,
                const TDF_LabelSequence& gdts, const TDF_LabelSequence& clippingPlanes,
                const TDF_Label& viewL) {
    t->SetView(shapes, gdts, clippingPlanes, viewL);
  });
  // Count/Get idiom for each of the 3 reference-label sequences a view can
  // carry -- same pattern as XCAFDoc_MaterialTool_NbMaterials/MaterialLabel.
  mod.method("XCAFDoc_ViewTool_RefShapeLabelCount", [](const Handle(XCAFDoc_ViewTool)& t,
                                                        const TDF_Label& viewL) -> int {
    TDF_LabelSequence seq;
    if (!t->GetRefShapeLabel(viewL, seq)) return 0;
    return seq.Length();
  });
  mod.method("XCAFDoc_ViewTool_RefShapeLabel", [](const Handle(XCAFDoc_ViewTool)& t,
                                                   const TDF_Label& viewL, int index) -> TDF_Label {
    TDF_LabelSequence seq;
    if (!t->GetRefShapeLabel(viewL, seq)) return TDF_Label();
    if (index < 1 || index > seq.Length()) return TDF_Label();
    return seq.Value(index);
  });
  mod.method("XCAFDoc_ViewTool_RefGDTLabelCount", [](const Handle(XCAFDoc_ViewTool)& t,
                                                      const TDF_Label& viewL) -> int {
    TDF_LabelSequence seq;
    if (!t->GetRefGDTLabel(viewL, seq)) return 0;
    return seq.Length();
  });
  mod.method("XCAFDoc_ViewTool_RefGDTLabel", [](const Handle(XCAFDoc_ViewTool)& t,
                                                 const TDF_Label& viewL, int index) -> TDF_Label {
    TDF_LabelSequence seq;
    if (!t->GetRefGDTLabel(viewL, seq)) return TDF_Label();
    if (index < 1 || index > seq.Length()) return TDF_Label();
    return seq.Value(index);
  });
  mod.method("XCAFDoc_ViewTool_RefClippingPlaneLabelCount",
             [](const Handle(XCAFDoc_ViewTool)& t, const TDF_Label& viewL) -> int {
    TDF_LabelSequence seq;
    if (!t->GetRefClippingPlaneLabel(viewL, seq)) return 0;
    return seq.Length();
  });
  mod.method("XCAFDoc_ViewTool_RefClippingPlaneLabel",
             [](const Handle(XCAFDoc_ViewTool)& t, const TDF_Label& viewL, int index) -> TDF_Label {
    TDF_LabelSequence seq;
    if (!t->GetRefClippingPlaneLabel(viewL, seq)) return TDF_Label();
    if (index < 1 || index > seq.Length()) return TDF_Label();
    return seq.Value(index);
  });

  // ---- XCAFView_Object: plain data object (camera/window parameters) ----
  // add_type only, no .constructor<>() -- Standard_Transient-derived, so it
  // needs a Handle(T)-returning factory instead (same "No appropriate
  // factory for type" pitfall as XCAFDimTolObjects_DimensionObject).
  // Must be registered before XCAFDoc_View below, whose SetObject/GetObject
  // reference Handle(XCAFView_Object) -- jlcxx registration order matters.
  mod.add_type<XCAFView_Object>("XCAFView_Object");
  mod.method("NewViewObject", []() -> Handle(XCAFView_Object) { return new XCAFView_Object(); });
  mod.method("IsNull", [](const Handle(XCAFView_Object)& o) -> bool { return o.IsNull(); });
  mod.method("SetName", [](const Handle(XCAFView_Object)& o, const Handle(TCollection_HAsciiString)& n) {
    o->SetName(n);
  });
  mod.method("Name", [](const Handle(XCAFView_Object)& o) -> Handle(TCollection_HAsciiString) {
    return o->Name();
  });
  mod.method("SetType", [](const Handle(XCAFView_Object)& o, int t) { o->SetType(XCAFView_ProjectionType(t)); });
  mod.method("GetType", [](const Handle(XCAFView_Object)& o) -> int { return int(o->Type()); });
  mod.method("SetProjectionPoint", [](const Handle(XCAFView_Object)& o, const gp_Pnt& p) {
    o->SetProjectionPoint(p);
  });
  mod.method("ProjectionPoint", [](const Handle(XCAFView_Object)& o) -> gp_Pnt { return o->ProjectionPoint(); });
  mod.method("SetViewDirection", [](const Handle(XCAFView_Object)& o, const gp_Dir& d) {
    o->SetViewDirection(d);
  });
  mod.method("ViewDirection", [](const Handle(XCAFView_Object)& o) -> gp_Dir { return o->ViewDirection(); });
  mod.method("SetUpDirection", [](const Handle(XCAFView_Object)& o, const gp_Dir& d) { o->SetUpDirection(d); });
  mod.method("UpDirection", [](const Handle(XCAFView_Object)& o) -> gp_Dir { return o->UpDirection(); });
  mod.method("SetZoomFactor", [](const Handle(XCAFView_Object)& o, double z) { o->SetZoomFactor(z); });
  mod.method("ZoomFactor", [](const Handle(XCAFView_Object)& o) -> double { return o->ZoomFactor(); });
  mod.method("SetWindowHorizontalSize", [](const Handle(XCAFView_Object)& o, double s) {
    o->SetWindowHorizontalSize(s);
  });
  mod.method("WindowHorizontalSize", [](const Handle(XCAFView_Object)& o) -> double {
    return o->WindowHorizontalSize();
  });
  mod.method("SetWindowVerticalSize", [](const Handle(XCAFView_Object)& o, double s) {
    o->SetWindowVerticalSize(s);
  });
  mod.method("WindowVerticalSize", [](const Handle(XCAFView_Object)& o) -> double {
    return o->WindowVerticalSize();
  });

  mod.method("XCAFView_ProjectionType_NoCamera", []() { return int(XCAFView_ProjectionType_NoCamera); });
  mod.method("XCAFView_ProjectionType_Parallel", []() { return int(XCAFView_ProjectionType_Parallel); });
  mod.method("XCAFView_ProjectionType_Central",  []() { return int(XCAFView_ProjectionType_Central); });

  // ---- XCAFDoc_View: the OCAF attribute carrying one XCAFView_Object per label ----
  mod.add_type<XCAFDoc_View>("XCAFDoc_View");
  mod.method("XCAFDoc_View_Set", [](const TDF_Label& label) -> Handle(XCAFDoc_View) {
    return XCAFDoc_View::Set(label);
  });
  mod.method("SetObject", [](const Handle(XCAFDoc_View)& attr, const Handle(XCAFView_Object)& obj) {
    attr->SetObject(obj);
  });
  mod.method("GetObject", [](const Handle(XCAFDoc_View)& attr) -> Handle(XCAFView_Object) {
    return attr->GetObject();
  });

  // ---- XCAFDoc_ClippingPlaneTool: self-contained plane+name+capping table ----
  mod.add_type<XCAFDoc_ClippingPlaneTool>("XCAFDoc_ClippingPlaneTool");
  mod.method("XCAFDoc_ClippingPlaneTool",
             [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_ClippingPlaneTool) {
    return XCAFDoc_DocumentTool::ClippingPlaneTool(doc->Main());
  });
  mod.method("IsClippingPlane", [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const TDF_Label& label) -> bool {
    return bool(t->IsClippingPlane(label));
  });
  mod.method("AddClippingPlane",
             [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const gp_Pln& plane,
                const std::string& name, bool capping) -> TDF_Label {
    return t->AddClippingPlane(plane, TCollection_ExtendedString(name.c_str()), capping);
  });
  mod.method("RemoveClippingPlane", [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const TDF_Label& label) -> bool {
    return bool(t->RemoveClippingPlane(label));
  });
  mod.method("XCAFDoc_ClippingPlaneTool_ClippingPlaneLabelCount",
             [](const Handle(XCAFDoc_ClippingPlaneTool)& t) -> int {
    TDF_LabelSequence seq;
    t->GetClippingPlanes(seq);
    return seq.Length();
  });
  mod.method("XCAFDoc_ClippingPlaneTool_ClippingPlaneLabel",
             [](const Handle(XCAFDoc_ClippingPlaneTool)& t, int index) -> TDF_Label {
    TDF_LabelSequence seq;
    t->GetClippingPlanes(seq);
    if (index < 1 || index > seq.Length()) return TDF_Label();
    return seq.Value(index);
  });
  // GetClippingPlane's 3 out-params bridged via std::make_tuple, same
  // established idiom as GeomConvert_CurveToAnaCurve::ConvertToAnalytical.
  mod.method("GetClippingPlane",
             [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const TDF_Label& label) {
    gp_Pln plane;
    TCollection_ExtendedString name;
    bool capping = false;
    bool ok = t->GetClippingPlane(label, plane, name, capping);
    return std::make_tuple(ok, plane, std::string(TCollection_AsciiString(name).ToCString()), capping);
  });
  mod.method("SetCapping", [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const TDF_Label& label, bool capping) {
    t->SetCapping(label, capping);
  });
  mod.method("GetCapping", [](const Handle(XCAFDoc_ClippingPlaneTool)& t, const TDF_Label& label) -> bool {
    return bool(t->GetCapping(label));
  });
}
