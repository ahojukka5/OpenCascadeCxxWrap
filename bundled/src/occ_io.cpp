// occ_io.cpp — 1:1 CxxWrap bindings for OCC I/O (BRepTools, STEP, IGES).
#include <sstream>
#include <stdexcept>
#include <string>

#include <jlcxx/jlcxx.hpp>

#include <TopoDS_Shape.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopTools_FormatVersion.hxx>

#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>

void register_occ_io(jlcxx::Module& mod) {
  mod.method("BRepTools_Write", [](const TopoDS_Shape& s, const std::string& f) {
    return bool(BRepTools::Write(s, f.c_str()));
  });
  mod.method("BRepTools_Read", [](TopoDS_Shape& s, const std::string& f) {
    BRep_Builder b;
    return bool(BRepTools::Read(s, f.c_str(), b));
  });

  mod.method("BRepTools_WriteString", [](const TopoDS_Shape& s) -> std::string {
    std::ostringstream out;
#if OCC_VERSION_HEX < 0x070600
    BRepTools::Write(s, out);
#else
    BRepTools::Write(s, out, false, false, TopTools_FormatVersion_VERSION_1);
#endif
    return out.str();
  });
  mod.method("BRepTools_ReadString", [](const std::string& brep) -> TopoDS_Shape {
    TopoDS_Shape shape;
    BRep_Builder builder;
    std::istringstream in(brep);
    BRepTools::Read(shape, in, builder);
    if (shape.IsNull())
      throw std::runtime_error("BRepTools_ReadString: failed to parse BREP data");
    return shape;
  });

  mod.add_type<STEPControl_Reader>("STEPControl_Reader").constructor<>();
  mod.method("ReadFile", [](STEPControl_Reader& r, const std::string& f) {
    return int(r.ReadFile(f.c_str()));
  });
  mod.method("TransferRoots", [](STEPControl_Reader& r) { return r.TransferRoots(); });
  mod.method("NbShapes",      [](STEPControl_Reader& r) { return r.NbShapes(); });
  mod.method("OneShape",      [](STEPControl_Reader& r) -> TopoDS_Shape { return r.OneShape(); });
  mod.method("SetSystemLengthUnit", [](STEPControl_Reader& r, double u) { r.SetSystemLengthUnit(u); });
  mod.method("SystemLengthUnit",    [](const STEPControl_Reader& r) -> double { return r.SystemLengthUnit(); });
  // Introspection only: what length unit does the file itself declare?
  // (OCCT's own transfer already converts correctly to the system unit --
  // confirmed empirically -- this is for a caller who wants to know/display
  // the file's declared unit, not to fix a conversion bug.)
  mod.method("StepFileLengthUnit", [](STEPControl_Reader& r) -> std::string {
    TColStd_SequenceOfAsciiString lengthUnits, angleUnits, solidAngleUnits;
    r.FileUnits(lengthUnits, angleUnits, solidAngleUnits);
    return lengthUnits.Length() >= 1 ? std::string(lengthUnits.Value(1).ToCString()) : std::string();
  });

  mod.add_type<STEPControl_Writer>("STEPControl_Writer").constructor<>();
  mod.method("Transfer", [](STEPControl_Writer& w, const TopoDS_Shape& s, int mode) {
    return int(w.Transfer(s, STEPControl_StepModelType(mode)));
  });
  mod.method("Write", [](STEPControl_Writer& w, const std::string& f) {
    return int(w.Write(f.c_str()));
  });

  mod.add_type<IGESControl_Reader>("IGESControl_Reader").constructor<>();
  mod.method("ReadFile", [](IGESControl_Reader& r, const std::string& f) {
    return int(r.ReadFile(f.c_str()));
  });
  mod.method("TransferRoots", [](IGESControl_Reader& r) { return r.TransferRoots(); });
  mod.method("OneShape",      [](IGESControl_Reader& r) -> TopoDS_Shape { return r.OneShape(); });

  mod.add_type<IGESControl_Writer>("IGESControl_Writer").constructor<>();
  mod.method("AddShape", [](IGESControl_Writer& w, const TopoDS_Shape& s) {
    return bool(w.AddShape(s));
  });
  mod.method("Write", [](IGESControl_Writer& w, const std::string& f) {
    return bool(w.Write(f.c_str()));
  });
}
