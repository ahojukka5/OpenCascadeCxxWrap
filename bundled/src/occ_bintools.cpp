// occ_bintools.cpp — 1:1 CxxWrap bindings for BinTools, OCCT's binary shape
// serialization -- a smaller/faster native round-trip format than the
// already-bound ASCII BRepTools::Read/Write (occ_breptools.cpp), distinct
// from STEP (interchange with other CAD tools) and ASCII BREP (debuggable
// text). Uses BinTools' own file-path overloads directly, no stream
// plumbing needed.
#include <jlcxx/jlcxx.hpp>

#include <BinTools.hxx>
#include <TopoDS_Shape.hxx>
#include <Message_ProgressRange.hxx>

void register_occ_bintools(jlcxx::Module& mod) {
  mod.method("BinTools_Write", [](const TopoDS_Shape& shape, const std::string& file) -> bool {
    Message_ProgressRange progress;
    return bool(BinTools::Write(shape, file.c_str(), progress));
  });
  mod.method("BinTools_Read", [](const std::string& file) -> TopoDS_Shape {
    TopoDS_Shape shape;
    Message_ProgressRange progress;
    BinTools::Read(shape, file.c_str(), progress);
    return shape;
  });
}
