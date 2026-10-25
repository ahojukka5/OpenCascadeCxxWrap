# OCCT 8.0.1 local wrapper qualification

The published consumer contract remains `OCCT_jll` 7.9.3. This note records a
local build of the existing bindings against the published OodiArtifacts
`occt_8_0_1` archives. It is not a JLL, a registry change, or a replacement
of the 7.9.3 releases.

Fresh local builds and stronger smoke-process isolation are recorded in
[current-source qualification](occt-8.0.1-local-qualification.md). The
source-pinned historical Linux evidence below remains separate.

## Revisions

| Item | Value |
| --- | --- |
| Wrapper branch | `feat/19-occt-801-bindings` at `e5ff8aa9ead64797ec4616cd7cd65c8cbb2040c1` |
| Wrapper master | `4386e15d84b975ba72f04deb6c9b77dcba757be9` |
| OCCT 8.0.1 source | `b8f597c677811d1f9f4d8a97f5ae2825c0353a42` |
| Linux x86_64/glibc tree | `9ad71dc7bc24252016d89611722806a84a9c100b` |
| Linux archive SHA-256 | `27447b3eaa9c16e568651e479c02f466db7957a32c5c3e3122b302232af63762` |
| macOS ARM64 tree | `04d2d2f805e3e6931e6f02670cc1309f180e5d55` |
| macOS archive SHA-256 | `17f2ddb18db38dc94c1e0cafcdee1567b8c837d405c4118265e0aaca666037c0` |
| CxxWrap | 0.17.5 |
| libcxxwrap_julia_jll | 0.14.12 |

## Reproducible Linux build

Use an isolated depot. Resolve the artifact through `Pkg.Artifacts`, then
configure the existing CMake project against that prefix:

```bash
export CC=gcc-14 CXX=g++-14
export JULIA_PKG_PRECOMPILE_AUTO=0
export JULIA_DEPOT_PATH="$PWD/depot:"

julia --startup-file=no --compiled-modules=no -e '
using Pkg.Artifacts
toml = "/path/to/OodiArtifacts/Artifacts.toml"
prefix = ensure_artifact_installed("occt_8_0_1", toml)
println(prefix)
'

# CxxWrap 0.17.5 in a throwaway project supplies JlCxx_DIR.
cmake -S bundled -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DJlCxx_DIR="$JLCXX/lib/cmake/JlCxx" \
  -DJulia_EXECUTABLE="$(command -v julia)" \
  -DOpenCASCADE_DIR="$PREFIX/lib/cmake/opencascade" \
  -DCMAKE_PREFIX_PATH="$PREFIX;$JLCXX" \
  -DOPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH=ON
cmake --build build --parallel 4

julia --startup-file=no --compiled-modules=no --project="$CXXENV" \
  scripts/occt8_smoke.jl "$PREFIX" build/libopencascade_cxxwrap.so
```

CMake must print `OCCT RWGltf_CafWriter: header not installed`. `readelf -d`
must list `libTK*.so.8.0.1` and must not list a 7.9 soname.

## Native Linux x86_64/glibc (LUMI uan01)

Host: Linux 6.4, glibc 2.38, CMake 3.28.3, g++ 14.3.0. Each Julia ABI used
an isolated depot. No `artifacts/Overrides.toml` was written.

| Check | 1.10.12 | 1.11.9 | 1.12.6 | 1.13.1 |
| --- | --- | --- | --- | --- |
| Artifact install (`9ad71dc7…`) | pass | pass | pass | pass |
| CMake + link, 48× `NEEDED` `libTK*.so.8.0.1`, zero 7.9 | pass | pass | pass | pass |
| `libjulia` NEEDED | `libjulia.so.1.10` | `libjulia.so.1.11` | `libjulia.so.1.12` | `libjulia.so.1.13` |
| libcxxwrap artifact | `a8f0a7b436c518450e24d3d59c4855a47b51ab92` | `6975fda2bf1f433598c43db7e6c568c3c3b47219` | `c257905bb27ce365e187bec4b4085a387f8c8dc6` | `2c8e3286c36e49cd2f2e4883844266a265341857` |
| `occt8_smoke.jl` version 8.0.1, box 6, Boolean 4500, STEP 4500, 48 toolkits, glTF unbound | pass | pass | pass | pass |

`g++-14` on this host recorded all 48 toolkit sonames as `NEEDED`. Debian
g++ with `--as-needed` (qemu Linux in PR #20) recorded 29; the runtime still
`dlopen`s the full set.

g++-14 warned that `GCE2d_MakeSegment`, `GCE2d_MakeArcOfCircle`, and
`GCE2d_MakeCircle` are deprecated aliases for `GC_Make*2d` since OCCT 8.0.0.
The 7.9.3 names were kept so the 7.9.3 path is unchanged.

## Unsupported

- `RWGltf_CafWriter` on the published OCCT 8.0.1 archives (no RapidJSON).
- Mixing OCCT 7.9.3 and 8.0.1 in one process.
- Publishing a JLL, changing OodiRegistry, or replacing 7.9.3 artifacts.
