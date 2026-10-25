# Current-source OCCT 8.0.1 qualification

This is an experimental source-build qualification. The compatibility code
was already merged in PR #20; this follow-up hardens the standalone smoke's
process isolation and records fresh local evidence. No native release is
replaced. The production recipe still selects OCCT_jll 7.9.3.

## Inputs

- Wrapper tested: `40f451aae93a98a2d141887067314455f932357a`, clean,
  based on master `2b2dd1d95e6bf7b4b70deb3c11bab7d33ee4063f`.
- Artifact catalogue: OodiArtifacts `851f8fd76a1307458be67978cd9164b96009a305`.
- OCCT source: `b8f597c677811d1f9f4d8a97f5ae2825c0353a42` (8.0.1).
- macOS native tree: `04d2d2f805e3e6931e6f02670cc1309f180e5d55`;
  archive SHA-256
  `17f2ddb18db38dc94c1e0cafcdee1567b8c837d405c4118265e0aaca666037c0`.
- CxxWrap 0.17.5; libcxxwrap_julia_jll 0.14.12+0.
  Their source trees are `f7a997d3959648a818c45dda059a45844300b94d` and
  `2eecfd1759a176424159c1993b66f012a230319f`, respectively.
- macOS 27.0.1 (26A434), ARM64, Apple Clang 21.0.0
  (clang-2100.3.34.2), CMake 4.4.2, deployment target 11.0.
  Installed JlCxx targets require C++20; CMake consequently compiles the
  wrapper with `-std=gnu++20` even though this project's minimum is C++17.

The existing CMake compatibility logic selects glTF from the configured
prefix's installed header. Standard_Failure uses `what()` on OCCT 8 and
`GetMessageString()` on 7.9.3. No further C++ API adaptation was needed.

## Fresh macOS ARM64 results

Pkg.Artifacts selected the published `occt_8_0_1` tree. Julia 1.12.6 also
ran `verify_artifact`, returning true. The other ABIs resolved the same
installed, verified tree; these are shared-depot runs, not four independent
clean downloads. Each ABI configured and compiled the complete existing
wrapper against the installed OCCT CMake package and its selected Julia.

| Julia | libcxxwrap artifact tree | OCCT 8.0.1 linked toolkits |
| --- | --- | --- |
| 1.10.11 | `5cea6d99ca1f751ca33837a3db1dd5461ad24d09` | 48 |
| 1.11.9 | `25d16a2b98f5702c16fa6d93a1e10ebad02d86f3` | 48 |
| 1.12.6 | `81bab7fd7c55828c9a992019a5d74800e898fdb3` | 48 |
| 1.13.1 | `c4ed4fc6220915cf9a4f708918ecc41db6dcd496` | 48 |

Each standalone fresh-process `scripts/occt8_smoke.jl` passed:

1. `@wrapmodule` and `@initcxx`, reporting header version 8.0.1.
2. Box volume 6; Boolean cut volume 4500; both shapes valid.
3. STEP export/import with volume 4500 and a valid imported shape.
4. All 48 actual toolkit paths from the selected OCCT artifact, one wrapper,
   and libcxxwrap from `CxxWrap.prefix_path()`, with no 7.9 libraries.
5. Explicitly absent RWGltf_CafWriter binding.

`otool -L` and `otool -l` were retained for each wrapper: 48 OCCT 8.0.1
sonames, the selected Julia minor and libcxxwrap prefix, and the OCCT
artifact library directory in LC_RPATH. No OCCT 7.9 soname was present.

On Julia 1.12.6, preloading OCCT_jll 7.9.3 caused the smoke to refuse before
any OCCT 8 library was opened. The new preflight also rejects a toolkit
loaded from another prefix even when OCCT_jll itself is not imported.

A separate Julia 1.12.6 rebuild of this exact wrapper source against
OCCT_jll 7.9.3 found RWGltf_CafWriter.hxx and kept that binding enabled.
The capability probe returned true, and all 936 consumer binding tests plus
9 geometry/runtime tests passed with that rebuilt library. Its native
linkage listed only 7.9 OCCT sonames. A process-local artifact-directory
selection kept it separate from every published archive and depot override.

## Reproduction

Use a throwaway Julia project with CxxWrap 0.17.5. Keep the shared depot,
build output and logs outside development checkouts. For each Julia minor,
use a separate environment and wrapper build directory. Set `TMPDIR` to a
cache directory before invoking Julia; no artifact override is required.

```sh
CASE="$HOME/.cache/oodi/OpenCascadeCxxWrap/occt8-qualification"
mkdir -p "$CASE/env" "$CASE/tmp" "$CASE/build"
export TMPDIR="$CASE/tmp" JULIA_PKG_PRECOMPILE_AUTO=0
julia +1.12.6 --startup-file=no --project="$CASE/env" -e '
using Pkg
Pkg.add(PackageSpec(name="CxxWrap", version="0.17.5"))'

# Pass the catalogue and wrapper checkout explicitly. Their revisions must
# match Inputs; do not assume a neighbouring checkout or search-path OCCT.
julia +1.12.6 --startup-file=no --project="$CASE/env" -e '
using Pkg.Artifacts, CxxWrap
prefix = ensure_artifact_installed("occt_8_0_1", ARGS[1])
hash = artifact_hash("occt_8_0_1", ARGS[1])
@assert verify_artifact(hash)
jlcxx = CxxWrap.prefix_path()
prefix_path = string(prefix, ";", jlcxx)
julia = joinpath(Sys.BINDIR, Base.julia_exename())
source, build = ARGS[2:3]
run(`cmake -S $source/bundled -B $build -DCMAKE_BUILD_TYPE=Release -DJlCxx_DIR=$jlcxx/lib/cmake/JlCxx -DJulia_EXECUTABLE=$julia -DOpenCASCADE_DIR=$prefix/lib/cmake/opencascade -DCMAKE_PREFIX_PATH=$prefix_path -DOPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH=ON`)
run(`cmake --build $build --parallel 4`)
println("OCCT_PREFIX=", prefix)
' /path/to/OodiArtifacts/Artifacts.toml /path/to/OpenCascadeCxxWrap "$CASE/build"

# Use the printed prefix, not an OCCT 7.9.3 JLL path.
julia +1.12.6 --startup-file=no --compiled-modules=no --project="$CASE/env" \
  /path/to/OpenCascadeCxxWrap/scripts/occt8_smoke.jl \
  /printed/OCCT_PREFIX "$CASE/build/libopencascade_cxxwrap.dylib"
otool -L "$CASE/build/libopencascade_cxxwrap.dylib"
otool -l "$CASE/build/libopencascade_cxxwrap.dylib"
```

On Linux the library suffix is `.so`; use `readelf -d` for linkage and
RUNPATH. The versioned toolkit names must be `.so.8.0.1`. The actual
libcxxwrap artifact differs by platform and Julia ABI.

## Limits and retained evidence

The published OCCT 8 archive disables RapidJSON/glTF. No fallback exporter
or optional rendering capability was qualified. OCCT 7.9 and 8 must never
share a CAD process. The preflight protects this script; it cannot prevent
unrelated user code from importing another OCCT later.

Raw commands, build/runtime records, native linkage and full `dllist()` paths
are retained under
`~/.cache/oodi/Monge.jl/occt8-20261025-132832/`. Each runtime record identifies
the exact wrapper commit, Julia, native artifact and CxxWrap prefix. The
existing [Linux qualification](occt-8.0.1.md#native-linux-x86_64glibc-lumi-uan01)
remains historical evidence from its cited source, separate from this run.
