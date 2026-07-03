# Build libopencascade_cxxwrap — strict 1:1 CxxWrap bindings for the OCCT
# modeling kernel (no Netgen / nglib).
cmake_minimum_required(VERSION 3.16)
project(OpenCascadeCxxWrap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JlCxx CONFIG REQUIRED)
find_package(OpenCASCADE REQUIRED)

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
)
target_compile_features(opencascade_cxxwrap PRIVATE cxx_std_17)

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

install(TARGETS opencascade_cxxwrap
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib)
