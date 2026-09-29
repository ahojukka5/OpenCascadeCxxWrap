# BinaryBuilder recipe for libopencascade_cxxwrap — strict 1:1 OCCT modeling
# kernel bindings (no Netgen). Depends only on OCCT_jll + libcxxwrap_julia_jll.
using BinaryBuilder, Pkg

name = "OpenCascadeCxxWrap"
version = v"0.1.0"

sources = [
    DirectorySource("./bundled"),
]

script = raw"""
cd ${WORKSPACE}/srcdir
mkdir build
cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TARGET_TOOLCHAIN} \
    -DCMAKE_BUILD_TYPE=Release \
    -DJlCxx_DIR=${prefix}/lib/cmake/JlCxx \
    -DCMAKE_PREFIX_PATH=${prefix}
make -j${nproc}
make install
install_license ${WORKSPACE}/srcdir/LICENSE
"""

platforms = supported_platforms()
platforms = filter!(p -> arch(p) != "armv6l" && !Sys.iswindows(p), platforms)
platforms = expand_cxxstring_abis(platforms)

products = [
    LibraryProduct("libopencascade_cxxwrap", :libopencascade_cxxwrap),
]

dependencies = [
    Dependency("OCCT_jll"; compat="7.9.3"),
    Dependency("Zlib_jll"),
    Dependency("libcxxwrap_julia_jll"),
]

build_tarballs(ARGS, name, version, sources, script, platforms, products, dependencies;
               julia_compat="1.6", preferred_gcc_version=v"10")
