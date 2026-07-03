// occ_vrml.cpp — 1:1 CxxWrap bindings for VrmlAPI_Writer (shape/XCAF-document
// VRML export) and VrmlAPI_CafReader (the first CAF-document *reader* bound
// in this codebase -- the existing RWGltf_CafWriter/RWObj_CafWriter are
// write-only). VrmlAPI_CafReader's own public surface is just an internal
// performMesh() override; the actual entry points (SetDocument/Perform/
// SingleShape) come from its base RWMesh_CafReader, mirrored here directly
// since jlcxx has no notion of inherited method binding.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <VrmlAPI_Writer.hxx>
#include <VrmlAPI_RepresentationOfShape.hxx>
#include <VrmlAPI_CafReader.hxx>
#include <TopoDS_Shape.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_AsciiString.hxx>
#include <Message_ProgressRange.hxx>

void register_occ_vrml(jlcxx::Module& mod) {
  mod.add_type<VrmlAPI_Writer>("VrmlAPI_Writer")
     .constructor<>();
  mod.method("SetDeflection", [](VrmlAPI_Writer& w, double d) { w.SetDeflection(d); });
  mod.method("SetRepresentation", [](VrmlAPI_Writer& w, int rep) {
    w.SetRepresentation(VrmlAPI_RepresentationOfShape(rep));
  });
  mod.method("Write", [](const VrmlAPI_Writer& w, const TopoDS_Shape& shape,
                          const std::string& file, int version) -> bool {
    return bool(w.Write(shape, file.c_str(), version));
  });
  mod.method("WriteDoc", [](const VrmlAPI_Writer& w, const Handle(TDocStd_Document)& doc,
                            const std::string& file, double scale) -> bool {
    return bool(w.WriteDoc(doc, file.c_str(), scale));
  });

  mod.method("VrmlAPI_ShadedRepresentation",    []() { return int(VrmlAPI_ShadedRepresentation); });
  mod.method("VrmlAPI_WireFrameRepresentation", []() { return int(VrmlAPI_WireFrameRepresentation); });
  mod.method("VrmlAPI_BothRepresentation",      []() { return int(VrmlAPI_BothRepresentation); });

  mod.add_type<VrmlAPI_CafReader>("VrmlAPI_CafReader")
     .constructor<>();
  mod.method("SetDocument", [](VrmlAPI_CafReader& r, const Handle(TDocStd_Document)& doc) {
    r.SetDocument(doc);
  });
  mod.method("Perform", [](VrmlAPI_CafReader& r, const std::string& file) -> bool {
    Message_ProgressRange progress;
    return bool(r.Perform(TCollection_AsciiString(file.c_str()), progress));
  });
  mod.method("SingleShape", [](const VrmlAPI_CafReader& r) -> TopoDS_Shape { return r.SingleShape(); });
}
