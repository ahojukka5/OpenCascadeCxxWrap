// occ_xcaf.cpp — minimal XCAF document + STEPCAF I/O for names/colors/assemblies.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_Editor.hxx>
#include <TCollection_HAsciiString.hxx>
#include <NCollection_Sequence.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <TDataStd_Name.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_TypeOfColor.hxx>
#include <XCAFDoc_ColorType.hxx>
#include <STEPControl_StepModelType.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TCollection_AsciiString.hxx>
#include <Standard_Type.hxx>

void register_occ_xcaf(jlcxx::Module& mod) {
  mod.add_type<TDF_Label>("TDF_Label");
  mod.method("TDF_Label_IsNull", [](const TDF_Label& label) -> bool { return label.IsNull(); });
  mod.add_type<XCAFApp_Application>("XCAFApp_Application");
  mod.add_type<TDocStd_Document>("TDocStd_Document");
  mod.add_type<XCAFDoc_ShapeTool>("XCAFDoc_ShapeTool");
  mod.add_type<XCAFDoc_ColorTool>("XCAFDoc_ColorTool");
  mod.add_type<XCAFDoc_LayerTool>("XCAFDoc_LayerTool");
  mod.add_type<XCAFDoc_MaterialTool>("XCAFDoc_MaterialTool");

  // TCollection_HAsciiString: minimal Handle-string bridge, needed by both this file's
  // material-name/description fields and occ_dimtol.cpp's GD&T name fields -- registered
  // here since occ_xcaf.cpp runs earlier than occ_dimtol.cpp (registration order matters
  // for Handle(T) factories).
  mod.add_type<TCollection_HAsciiString>("TCollection_HAsciiString");
  mod.method("NewHAsciiString", [](const std::string& s) -> Handle(TCollection_HAsciiString) {
    return new TCollection_HAsciiString(s.c_str());
  });
  mod.method("ToCString", [](const Handle(TCollection_HAsciiString)& s) -> std::string {
    return s.IsNull() ? std::string() : std::string(s->ToCString());
  });

  mod.method("XCAFApp_GetApplication", []() -> Handle(XCAFApp_Application) {
    return XCAFApp_Application::GetApplication();
  });

  mod.method("XCAFApp_NewDocument", [](Handle(XCAFApp_Application)& app) -> Handle(TDocStd_Document) {
    Handle(TDocStd_Document) doc;
    app->NewDocument("MDTV-XCAF", doc);
    return doc;
  });

  mod.method("XCAFDoc_ShapeTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_ShapeTool) {
    return XCAFDoc_DocumentTool::ShapeTool(doc->Main());
  });

  // The document's own root label -- needed as XCAFDoc_Editor::Expand's
  // theDoc argument (distinct from any specific shape's label).
  mod.method("XCAFDoc_Document_Main", [](const Handle(TDocStd_Document)& doc) -> TDF_Label {
    return doc->Main();
  });

  mod.method("XCAFDoc_ColorTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_ColorTool) {
    return XCAFDoc_DocumentTool::ColorTool(doc->Main());
  });

  mod.method("XCAFDoc_LayerTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_LayerTool) {
    return XCAFDoc_DocumentTool::LayerTool(doc->Main());
  });

  // ---- XCAFDoc_MaterialTool: physical/manufacturing material read-back ----
  // STEPCAFControl_Reader already populates this from STEP AP242 material-designation
  // entities when SetMatMode(reader, true) is set (io.jl's read_step_xcaf already does
  // this) -- but until now nothing could read the result back out.
  mod.method("XCAFDoc_MaterialTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_MaterialTool) {
    return XCAFDoc_DocumentTool::MaterialTool(doc->Main());
  });
  mod.method("IsMaterial", [](const Handle(XCAFDoc_MaterialTool)& t, const TDF_Label& lab) -> bool {
    return bool(t->IsMaterial(lab));
  });
  mod.method("XCAFDoc_MaterialTool_NbMaterials", [](const Handle(XCAFDoc_MaterialTool)& t) -> int {
    NCollection_Sequence<TDF_Label> seq;
    t->GetMaterialLabels(seq);
    return seq.Length();
  });
  mod.method("XCAFDoc_MaterialTool_MaterialLabel",
             [](const Handle(XCAFDoc_MaterialTool)& t, int i) -> TDF_Label {
    NCollection_Sequence<TDF_Label> seq;
    t->GetMaterialLabels(seq);
    return (i < 1 || i > seq.Length()) ? TDF_Label() : seq.Value(i);
  });
  // GetMaterial is a single static call producing 5 out-params atomically; mirrors
  // XCAFDimTolObjects_DimensionObject's per-field-accessor style (occ_dimtol.cpp) rather
  // than returning a tuple -- each accessor re-invokes the call and keeps one field.
  mod.method("XCAFDoc_MaterialTool_MaterialName",
             [](const TDF_Label& matLabel) -> Handle(TCollection_HAsciiString) {
    Handle(TCollection_HAsciiString) name, desc, densName, densValType;
    double density = 0.0;
    XCAFDoc_MaterialTool::GetMaterial(matLabel, name, desc, density, densName, densValType);
    return name;
  });
  mod.method("XCAFDoc_MaterialTool_MaterialDescription",
             [](const TDF_Label& matLabel) -> Handle(TCollection_HAsciiString) {
    Handle(TCollection_HAsciiString) name, desc, densName, densValType;
    double density = 0.0;
    XCAFDoc_MaterialTool::GetMaterial(matLabel, name, desc, density, densName, densValType);
    return desc;
  });
  mod.method("XCAFDoc_MaterialTool_MaterialDensity", [](const TDF_Label& matLabel) -> double {
    Handle(TCollection_HAsciiString) name, desc, densName, densValType;
    double density = 0.0;
    XCAFDoc_MaterialTool::GetMaterial(matLabel, name, desc, density, densName, densValType);
    return density;
  });
  mod.method("XCAFDoc_MaterialTool_MaterialDensityName",
             [](const TDF_Label& matLabel) -> Handle(TCollection_HAsciiString) {
    Handle(TCollection_HAsciiString) name, desc, densName, densValType;
    double density = 0.0;
    XCAFDoc_MaterialTool::GetMaterial(matLabel, name, desc, density, densName, densValType);
    return densName;
  });
  mod.method("XCAFDoc_MaterialTool_MaterialDensityValType",
             [](const TDF_Label& matLabel) -> Handle(TCollection_HAsciiString) {
    Handle(TCollection_HAsciiString) name, desc, densName, densValType;
    double density = 0.0;
    XCAFDoc_MaterialTool::GetMaterial(matLabel, name, desc, density, densName, densValType);
    return densValType;
  });
  mod.method("XCAFDoc_MaterialTool_GetDensityForShape", [](const TDF_Label& shapeLabel) -> double {
    return XCAFDoc_MaterialTool::GetDensityForShape(shapeLabel);
  });
  // Minimal write-side (AddMaterial + link), added alongside the read-only slice above
  // specifically so this binding can be verified with a real round trip rather than only
  // an empty-document check -- full write-side GD&T-style authoring is still out of scope.
  mod.method("XCAFDoc_MaterialTool_AddMaterial",
             [](const Handle(XCAFDoc_MaterialTool)& t, const Handle(TCollection_HAsciiString)& name,
                const Handle(TCollection_HAsciiString)& desc, double density,
                const Handle(TCollection_HAsciiString)& densName,
                const Handle(TCollection_HAsciiString)& densValType) -> TDF_Label {
    return t->AddMaterial(name, desc, density, densName, densValType);
  });
  mod.method("XCAFDoc_MaterialTool_SetMaterial",
             [](const Handle(XCAFDoc_MaterialTool)& t, const TDF_Label& shapeLabel,
                const TDF_Label& matLabel) {
    t->SetMaterial(shapeLabel, matLabel);
  });

  // The document's own declared length unit (meters per model unit) -- STEPCAFControl_Reader
  // already sets this automatically from a STEP file's own units (confirmed in
  // STEPCAFControl_Reader.cxx's prepareUnits()), and RWGltf_CafWriter/RWObj_CafWriter's own
  // Perform() already reads it automatically to scale mesh export output. A document created
  // directly via XCAFApp_NewDocument (not from a STEP read) never gets this set, so mesh
  // export silently applies no unit scaling at all -- this is the fix, not a new capability.
  mod.method("XCAFDoc_DocumentTool_SetLengthUnit",
             [](const Handle(TDocStd_Document)& doc, double unitInMeters) {
    XCAFDoc_DocumentTool::SetLengthUnit(doc, unitInMeters);
  });
  mod.method("XCAFDoc_DocumentTool_GetLengthUnit",
             [](const Handle(TDocStd_Document)& doc) -> double {
    double v = -1.0;
    XCAFDoc_DocumentTool::GetLengthUnit(doc, v);
    return v;  // -1.0 sentinel = "not set", matching RWMesh_CoordinateSystemConverter's own convention
  });

  mod.method("XCAFDoc_LayerTool_AddLayer", [](Handle(XCAFDoc_LayerTool)& tool,
                                               const std::string& name) -> TDF_Label {
    return tool->AddLayer(TCollection_ExtendedString(name.c_str()));
  });

  mod.method("XCAFDoc_LayerTool_SetLayer", [](Handle(XCAFDoc_LayerTool)& tool,
                                               const TDF_Label& shapeLabel, const std::string& layerName) {
    tool->SetLayer(shapeLabel, TCollection_ExtendedString(layerName.c_str()), false);
  });

  // Layer names as one ';'-joined string rather than a std::vector<std::string>
  // return (no existing binding in this codebase returns a string collection,
  // and this avoids introducing that risk for a single, low-traffic feature).
  mod.method("XCAFDoc_LayerTool_GetLayerNames", [](Handle(XCAFDoc_LayerTool)& tool,
                                                    const TDF_Label& shapeLabel) -> std::string {
    TDF_LabelSequence labelSeq;
    tool->GetLayers(shapeLabel, labelSeq);
    std::string result;
    for (int i = 1; i <= labelSeq.Length(); ++i) {
      TCollection_ExtendedString name;
      if (tool->GetLayer(labelSeq.Value(i), name)) {
        if (!result.empty()) result += ";";
        result += TCollection_AsciiString(name).ToCString();
      }
    }
    return result;
  });

  mod.method("XCAFDoc_LayerTool_GetAllLayerNames", [](Handle(XCAFDoc_LayerTool)& tool) -> std::string {
    TDF_LabelSequence labelSeq;
    tool->GetLayerLabels(labelSeq);
    std::string result;
    for (int i = 1; i <= labelSeq.Length(); ++i) {
      TCollection_ExtendedString name;
      if (tool->GetLayer(labelSeq.Value(i), name)) {
        if (!result.empty()) result += ";";
        result += TCollection_AsciiString(name).ToCString();
      }
    }
    return result;
  });

  mod.method("XCAFDoc_ShapeTool_AddShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                               const TopoDS_Shape& s) -> TDF_Label {
    return tool->AddShape(s);
  });
  // 3-arg overload exposing AddShape's theExpand flag -- the 2-arg call
  // above always defaults it to true (auto-expanding compounds on add),
  // which makes XCAFDoc_Editor_Expand a no-op for anything added that way;
  // pass expand=false to add a compound as a single un-expanded shape.
  mod.method("XCAFDoc_ShapeTool_AddShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                               const TopoDS_Shape& s, bool expand) -> TDF_Label {
    return tool->AddShape(s, expand);
  });

  mod.method("XCAFDoc_GetOneFreeShape", [](Handle(XCAFDoc_ShapeTool)& tool) -> TopoDS_Shape {
    TDF_LabelSequence labels;
    tool->GetFreeShapes(labels);
    if (labels.Length() < 1) return TopoDS_Shape();
    return tool->GetShape(labels.Value(1));
  });

  mod.method("XCAFDoc_FreeShapeCount", [](Handle(XCAFDoc_ShapeTool)& tool) -> int {
    TDF_LabelSequence labels;
    tool->GetFreeShapes(labels);
    return int(labels.Length());
  });

  mod.method("XCAFDoc_FreeShapeLabel", [](Handle(XCAFDoc_ShapeTool)& tool, int index) -> TDF_Label {
    TDF_LabelSequence labels;
    tool->GetFreeShapes(labels);
    if (index < 1 || index > labels.Length()) return TDF_Label();
    return labels.Value(index);
  });

  mod.method("XCAFDoc_ShapeTool_GetShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                               const TDF_Label& label) -> TopoDS_Shape {
    return tool->GetShape(label);
  });

  mod.method("XCAFDoc_ShapeTool_FindShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                                const TopoDS_Shape& shape) -> TDF_Label {
    TDF_Label label;
    tool->FindShape(shape, label);
    return label;
  });

  mod.method("XCAFDoc_ShapeTool_AddSubShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                                  const TDF_Label& parent,
                                                  const TopoDS_Shape& sub) -> TDF_Label {
    return tool->AddSubShape(parent, sub);
  });

  mod.method("XCAFDoc_ShapeTool_FindSubShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                                   const TDF_Label& parent,
                                                   const TopoDS_Shape& sub) -> TDF_Label {
    TDF_Label label;
    tool->FindSubShape(parent, sub, label);
    return label;
  });

  mod.method("XCAFDoc_SubShapeCount", [](Handle(XCAFDoc_ShapeTool)& tool,
                                          const TDF_Label& parent) -> int {
    TDF_LabelSequence labels;
    tool->GetSubShapes(parent, labels);
    return int(labels.Length());
  });

  mod.method("XCAFDoc_SubShapeLabel", [](Handle(XCAFDoc_ShapeTool)& tool,
                                          const TDF_Label& parent,
                                          int index) -> TDF_Label {
    TDF_LabelSequence labels;
    tool->GetSubShapes(parent, labels);
    if (index < 1 || index > labels.Length()) return TDF_Label();
    return labels.Value(index);
  });

  mod.method("XCAFDoc_ComponentCount", [](Handle(XCAFDoc_ShapeTool)& tool,
                                           const TDF_Label& label) -> int {
    TDF_LabelSequence components;
    if (!tool->GetComponents(label, components)) return 0;
    return int(components.Length());
  });

  mod.method("XCAFDoc_ComponentLabel", [](Handle(XCAFDoc_ShapeTool)& tool,
                                           const TDF_Label& label,
                                           int index) -> TDF_Label {
    TDF_LabelSequence components;
    if (!tool->GetComponents(label, components)) return TDF_Label();
    if (index < 1 || index > components.Length()) return TDF_Label();
    return components.Value(index);
  });

  // Component-tree construction (native XCAF assemblies: a prototype shape
  // label + one component label per placed instance, replacing Assembly's
  // previous flat baked-transform-only representation).
  mod.method("XCAFDoc_ShapeTool_NewShape", [](Handle(XCAFDoc_ShapeTool)& tool) -> TDF_Label {
    return tool->NewShape();
  });

  mod.method("XCAFDoc_ShapeTool_AddComponent", [](Handle(XCAFDoc_ShapeTool)& tool,
                                                   const TDF_Label& assembly,
                                                   const TDF_Label& comp,
                                                   const TopLoc_Location& loc) -> TDF_Label {
    return tool->AddComponent(assembly, comp, loc);
  });

  mod.method("XCAFDoc_ShapeTool_UpdateAssemblies", [](Handle(XCAFDoc_ShapeTool)& tool) {
    tool->UpdateAssemblies();
  });

  mod.method("XCAFDoc_ShapeTool_IsAssembly", [](const TDF_Label& label) -> bool {
    return bool(XCAFDoc_ShapeTool::IsAssembly(label));
  });

  mod.method("XCAFDoc_ShapeTool_IsComponent", [](const TDF_Label& label) -> bool {
    return bool(XCAFDoc_ShapeTool::IsComponent(label));
  });

  mod.method("XCAFDoc_ShapeTool_GetLocation", [](const TDF_Label& label) -> TopLoc_Location {
    return XCAFDoc_ShapeTool::GetLocation(label);
  });

  // Read-side counterpart to AddComponent above: given a component label,
  // find the label of the prototype shape it refers to (so its actual
  // geometry can be fetched via XCAFDoc_ShapeTool_GetShape on the result) --
  // needed to reconstruct a Monge Assembly from a nested XCAF component
  // tree read back from STEP, not just to build one for writing.
  mod.method("XCAFDoc_ShapeTool_GetReferredShape", [](const TDF_Label& label) -> TDF_Label {
    TDF_Label referred;
    XCAFDoc_ShapeTool::GetReferredShape(label, referred);
    return referred;
  });

  mod.method("TDataStd_Name_Set", [](const TDF_Label& label, const std::string& name) {
    TCollection_ExtendedString ext(name.c_str());
    TDataStd_Name::Set(label, ext);
  });

  mod.method("TDataStd_Name_Get", [](const TDF_Label& label) -> std::string {
    Handle(TDataStd_Name) attr;
    if (!label.FindAttribute(TDataStd_Name::GetID(), attr)) return std::string();
    TCollection_AsciiString asc(attr->Get());
    return std::string(asc.ToCString());
  });

  mod.method("XCAFDoc_ColorTool_GetColor", [](Handle(XCAFDoc_ColorTool)& tool,
                                                 const TDF_Label& label) {
    Quantity_Color col;
    const bool ok = tool->GetColor(label, XCAFDoc_ColorGen, col);
    return std::make_tuple(ok, col.Red(), col.Green(), col.Blue());
  });

  mod.method("XCAFDoc_ColorTool_SetColor", [](Handle(XCAFDoc_ColorTool)& tool,
                                                 const TDF_Label& label,
                                                 double r, double g, double b) {
    Quantity_Color col(r, g, b, Quantity_TOC_RGB);
    tool->SetColor(label, col, XCAFDoc_ColorGen);
  });

  mod.add_type<STEPCAFControl_Reader>("STEPCAFControl_Reader").constructor<>();
  mod.method("ReadFile", [](STEPCAFControl_Reader& r, const std::string& f) {
    return int(r.ReadFile(f.c_str()));
  });
  mod.method("Transfer", [](STEPCAFControl_Reader& r, Handle(TDocStd_Document)& doc) {
    return bool(r.Transfer(doc));
  });
  // Explicit transfer-mode control -- previously always ran with OCCT's
  // compiled-in defaults (confirmed empirically to already transfer
  // colors/names/layers correctly); these guarantee that behavior rather
  // than leaving it to an undocumented default a future OCCT version could
  // silently change, and let a caller opt out (e.g. skip color transfer).
  mod.method("SetColorMode", [](STEPCAFControl_Reader& r, bool value) { r.SetColorMode(value); });
  mod.method("SetNameMode",  [](STEPCAFControl_Reader& r, bool value) { r.SetNameMode(value); });
  mod.method("SetLayerMode", [](STEPCAFControl_Reader& r, bool value) { r.SetLayerMode(value); });
  mod.method("SetMatMode",   [](STEPCAFControl_Reader& r, bool value) { r.SetMatMode(value); });

  mod.add_type<STEPCAFControl_Writer>("STEPCAFControl_Writer").constructor<>();
  mod.method("Transfer", [](STEPCAFControl_Writer& w, Handle(TDocStd_Document)& doc, int mode) {
    return bool(w.Transfer(doc, STEPControl_StepModelType(mode)));
  });
  mod.method("Write", [](STEPCAFControl_Writer& w, const std::string& f) {
    return int(w.Write(f.c_str()));
  });
  mod.method("SetColorMode",    [](STEPCAFControl_Writer& w, bool value) { w.SetColorMode(value); });
  mod.method("SetNameMode",     [](STEPCAFControl_Writer& w, bool value) { w.SetNameMode(value); });
  mod.method("SetLayerMode",    [](STEPCAFControl_Writer& w, bool value) { w.SetLayerMode(value); });
  mod.method("SetMaterialMode", [](STEPCAFControl_Writer& w, bool value) { w.SetMaterialMode(value); });

  mod.method("Quantity_TOC_RGB", []() { return int(Quantity_TOC_RGB); });
  mod.method("XCAFDoc_ColorGen", []() { return int(XCAFDoc_ColorGen); });
  mod.method("XCAFDoc_ColorSurf", []() { return int(XCAFDoc_ColorSurf); });
  mod.method("XCAFDoc_ColorCurv", []() { return int(XCAFDoc_ColorCurv); });

  // ---- BOM instance count: how many places a prototype part is used
  // across the assembly tree (not reachable before -- only
  // ComponentCount/ComponentLabel/IsAssembly, which walk the tree the other
  // direction: assembly -> its components, not component -> its users).
  // Same NCollection_Sequence<TDF_Label> Count/Get marshaling as
  // XCAFDoc_ComponentCount/ComponentLabel above.
  mod.method("XCAFDoc_UserCount", [](const TDF_Label& label, bool getsubchilds) -> int {
    TDF_LabelSequence users;
    return XCAFDoc_ShapeTool::GetUsers(label, users, getsubchilds);
  });
  mod.method("XCAFDoc_UserLabel", [](const TDF_Label& label, int index, bool getsubchilds) -> TDF_Label {
    TDF_LabelSequence users;
    XCAFDoc_ShapeTool::GetUsers(label, users, getsubchilds);
    if (index < 1 || index > users.Length()) return TDF_Label();
    return users.Value(index);
  });

  // ---- XCAFDoc_Editor: static structural-editing utilities on a document's
  // label tree. All plain static methods -- no add_type/Handle needed.
  mod.method("XCAFDoc_Editor_RescaleGeometry",
             [](const TDF_Label& label, double scaleFactor, bool forceIfNotRoot) -> bool {
    return bool(XCAFDoc_Editor::RescaleGeometry(label, scaleFactor, forceIfNotRoot));
  });
  mod.method("XCAFDoc_Editor_Expand", [](const TDF_Label& doc, bool recursively) -> bool {
    return bool(XCAFDoc_Editor::Expand(doc, recursively));
  });
  mod.method("XCAFDoc_Editor_ExpandShape",
             [](const TDF_Label& doc, const TDF_Label& shape, bool recursively) -> bool {
    return bool(XCAFDoc_Editor::Expand(doc, shape, recursively));
  });
}
