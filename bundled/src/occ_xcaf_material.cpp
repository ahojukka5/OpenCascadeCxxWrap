// occ_xcaf_material.cpp — 1:1 CxxWrap bindings for XCAF PBR (metallic-
// roughness) visual materials: XCAFDoc_VisMaterialPBR (the plain data
// struct), XCAFDoc_VisMaterial (the label-attached attribute wrapping it),
// and XCAFDoc_VisMaterialTool (assigns materials to shapes). Directly
// complements write_gltf, since RWGltf_CafWriter picks up PBR materials
// assigned this way. Texture fields (BaseColorTexture etc.) are not bound
// -- only the scalar/color parameters most callers need.
#include <jlcxx/jlcxx.hpp>

#include "occ_handle_traits.hpp"

#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialPBR.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TopoDS_Shape.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <TCollection_AsciiString.hxx>
#include <string>

void register_occ_xcaf_material(jlcxx::Module& mod) {
  mod.add_type<XCAFDoc_VisMaterialPBR>("XCAFDoc_VisMaterialPBR").constructor<>();
  mod.method("SetPbrBaseColor", [](XCAFDoc_VisMaterialPBR& m, double r, double g, double b, double a) {
    m.BaseColor = Quantity_ColorRGBA(float(r), float(g), float(b), float(a));
  });
  mod.method("SetPbrMetallic", [](XCAFDoc_VisMaterialPBR& m, double v) { m.Metallic = float(v); });
  mod.method("SetPbrRoughness", [](XCAFDoc_VisMaterialPBR& m, double v) { m.Roughness = float(v); });
  mod.method("PbrMetallic", [](const XCAFDoc_VisMaterialPBR& m) -> double { return double(m.Metallic); });
  mod.method("PbrRoughness", [](const XCAFDoc_VisMaterialPBR& m) -> double { return double(m.Roughness); });

  mod.add_type<XCAFDoc_VisMaterial>("XCAFDoc_VisMaterial");
  mod.method("NewVisMaterial", []() -> Handle(XCAFDoc_VisMaterial) { return new XCAFDoc_VisMaterial(); });
  mod.method("SetPbrMaterial", [](const Handle(XCAFDoc_VisMaterial)& m, const XCAFDoc_VisMaterialPBR& pbr) {
    m->SetPbrMaterial(pbr);
  });
  mod.method("VisMaterialBaseColorR", [](const Handle(XCAFDoc_VisMaterial)& m) -> double {
    return m->BaseColor().GetRGB().Red();
  });
  mod.method("VisMaterialBaseColorG", [](const Handle(XCAFDoc_VisMaterial)& m) -> double {
    return m->BaseColor().GetRGB().Green();
  });
  mod.method("VisMaterialBaseColorB", [](const Handle(XCAFDoc_VisMaterial)& m) -> double {
    return m->BaseColor().GetRGB().Blue();
  });
  mod.method("VisMaterialBaseColorA", [](const Handle(XCAFDoc_VisMaterial)& m) -> double {
    return double(m->BaseColor().Alpha());
  });

  mod.add_type<XCAFDoc_VisMaterialTool>("XCAFDoc_VisMaterialTool");
  mod.method("XCAFDoc_VisMaterialTool", [](const Handle(TDocStd_Document)& doc) -> Handle(XCAFDoc_VisMaterialTool) {
    return XCAFDoc_DocumentTool::VisMaterialTool(doc->Main());
  });
  mod.method("AddMaterial", [](const Handle(XCAFDoc_VisMaterialTool)& t, const Handle(XCAFDoc_VisMaterial)& mat,
                                const std::string& name) -> TDF_Label {
    return t->AddMaterial(mat, TCollection_AsciiString(name.c_str()));
  });
  mod.method("SetShapeMaterial", [](const Handle(XCAFDoc_VisMaterialTool)& t, const TopoDS_Shape& shape,
                                     const TDF_Label& materialLabel) -> bool {
    return t->SetShapeMaterial(shape, materialLabel);
  });
  mod.method("GetShapeMaterial", [](const Handle(XCAFDoc_VisMaterialTool)& t, const TopoDS_Shape& shape)
                                     -> Handle(XCAFDoc_VisMaterial) {
    return t->GetShapeMaterial(shape);
  });
  mod.method("IsSetShapeMaterial", [](const Handle(XCAFDoc_VisMaterialTool)& t, const TopoDS_Shape& shape) -> bool {
    return t->IsSetShapeMaterial(shape);
  });
}
