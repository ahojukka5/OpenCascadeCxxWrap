// occ_xcaf.cpp — minimal XCAF document + STEPCAF I/O for names/colors/assemblies.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <TDataStd_Name.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_TypeOfColor.hxx>
#include <XCAFDoc_ColorType.hxx>
#include <STEPControl_StepModelType.hxx>
#include <TopoDS_Shape.hxx>
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

  mod.method("XCAFDoc_ColorTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_ColorTool) {
    return XCAFDoc_DocumentTool::ColorTool(doc->Main());
  });

  mod.method("XCAFDoc_ShapeTool_AddShape", [](Handle(XCAFDoc_ShapeTool)& tool,
                                               const TopoDS_Shape& s) -> TDF_Label {
    return tool->AddShape(s);
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

  mod.add_type<STEPCAFControl_Writer>("STEPCAFControl_Writer").constructor<>();
  mod.method("Transfer", [](STEPCAFControl_Writer& w, Handle(TDocStd_Document)& doc, int mode) {
    return bool(w.Transfer(doc, STEPControl_StepModelType(mode)));
  });
  mod.method("Write", [](STEPCAFControl_Writer& w, const std::string& f) {
    return int(w.Write(f.c_str()));
  });

  mod.method("Quantity_TOC_RGB", []() { return int(Quantity_TOC_RGB); });
  mod.method("XCAFDoc_ColorGen", []() { return int(XCAFDoc_ColorGen); });
  mod.method("XCAFDoc_ColorSurf", []() { return int(XCAFDoc_ColorSurf); });
  mod.method("XCAFDoc_ColorCurv", []() { return int(XCAFDoc_ColorCurv); });
}
