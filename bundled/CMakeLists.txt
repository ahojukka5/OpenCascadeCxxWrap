# Build libopencascade_cxxwrap — strict 1:1 CxxWrap bindings for the OCCT
# modeling kernel (no Netgen / nglib).
cmake_minimum_required(VERSION 3.16)
project(OpenCascadeCxxWrap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JlCxx CONFIG REQUIRED)
find_package(OpenCASCADE REQUIRED)

# RWGltf_CafWriter is part of OCCT's optional RapidJSON/glTF toolkit.
# OCCT 7.9.3 installs the header, including builds whose runtime writer
# fails closed because RapidJSON itself is absent. The published OCCT
# 8.0.1 archives disable RapidJSON and do not install the header.
# Decide from the configured prefix only. A header found on the compiler's
# default search path must not turn the binding back on.
set(OPENCASCADE_CXXWRAP_RWGLTF_HEADER
  "${OpenCASCADE_INCLUDE_DIR}/RWGltf_CafWriter.hxx")
if(EXISTS "${OPENCASCADE_CXXWRAP_RWGLTF_HEADER}")
  set(OPENCASCADE_CXXWRAP_HAS_RWGLTF ON)
  message(STATUS "OCCT RWGltf_CafWriter: ${OPENCASCADE_CXXWRAP_RWGLTF_HEADER}")
else()
  set(OPENCASCADE_CXXWRAP_HAS_RWGLTF OFF)
  message(STATUS
    "OCCT RWGltf_CafWriter: header not installed; the binding stays unloaded")
endif()

# Off in the BinaryBuilder recipe so a 7.9.3 package build does not embed
# a host prefix. Local OCCT 8.0.1 builds turn it on and then check the
# loader path.
option(OPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH
  "Embed OpenCASCADE_LIBRARY_DIR as an RPATH on libopencascade_cxxwrap"
  OFF)

add_library(opencascade_cxxwrap SHARED
  src/opencascade.cpp
  src/occ_gp.cpp
  src/occ_topology.cpp
  src/occ_builders.cpp
  src/occ_io.cpp
  src/occ_props.cpp
  src/occ_fillet.cpp
  src/occ_topo2.cpp
  src/occ_adaptor.cpp
  src/occ_analysis.cpp
  src/occ_sweep.cpp
  src/occ_wire.cpp
  src/occ_sewing.cpp
  src/occ_extrema.cpp
  src/occ_builder2.cpp
  src/occ_intersect.cpp
  src/occ_shape_analysis.cpp
  src/occ_breptools.cpp
  src/occ_mesh.cpp
  src/occ_shapefix2.cpp
  src/occ_shapeupgrade.cpp
  src/occ_toptools_list.cpp
  src/occ_geom.cpp
  src/occ_geom2d.cpp
  src/occ_feat.cpp
  src/occ_thicksolid.cpp
  src/occ_draft.cpp
  src/occ_breplib.cpp
  src/occ_advanced.cpp
  src/occ_xcaf.cpp
  src/occ_history.cpp
  src/occ_bopalgo_analyzer.cpp
  src/occ_bopalgo_builder.cpp
  src/occ_feat_hole.cpp
  src/occ_helix.cpp
  src/occ_geom2d_int.cpp
  src/occ_xcaf_material.cpp
  src/occ_igescaf.cpp
  src/occ_localanalysis.cpp
  src/occ_gce.cpp
  src/occ_units.cpp
  src/occ_rwstl.cpp
  src/occ_triangulation.cpp
  src/occ_interface_static.cpp
  src/occ_gltf.cpp
  src/occ_fill.cpp
  src/occ_hlr.cpp
  src/occ_geomfill.cpp
  src/occ_robj.cpp
  src/occ_normalproj.cpp
  src/occ_chfi2d.cpp
  src/occ_geomplate.cpp
  src/occ_geomconvert.cpp
  src/occ_vrml.cpp
  src/occ_findplane.cpp
  src/occ_bintools.cpp
  src/occ_shapecustom.cpp
  src/occ_dimtol.cpp
  src/occ_extrema2.cpp
  src/occ_fillet2d.cpp
  src/occ_bopalgo_connected.cpp
  src/occ_xcafview.cpp
  src/occ_ply.cpp
)
target_compile_features(opencascade_cxxwrap PRIVATE cxx_std_17)

if(OPENCASCADE_CXXWRAP_HAS_RWGLTF)
  target_compile_definitions(opencascade_cxxwrap PRIVATE
    OPENCASCADE_CXXWRAP_HAS_RWGLTF=1)
endif()

target_include_directories(opencascade_cxxwrap PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(opencascade_cxxwrap PRIVATE
  JlCxx::cxxwrap_julia
  ${OpenCASCADE_LIBRARIES}
)

set_target_properties(opencascade_cxxwrap PROPERTIES
  CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN OFF)

if(OPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH)
  if(NOT OpenCASCADE_LIBRARY_DIR)
    message(FATAL_ERROR
      "OPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH requires OpenCASCADE_LIBRARY_DIR")
  endif()
  set_target_properties(opencascade_cxxwrap PROPERTIES
    BUILD_RPATH "${OpenCASCADE_LIBRARY_DIR}"
    INSTALL_RPATH "${OpenCASCADE_LIBRARY_DIR}")
endif()

install(TARGETS opencascade_cxxwrap
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib)
