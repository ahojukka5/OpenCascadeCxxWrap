#!/usr/bin/env bash
set -euo pipefail

: "${MONGE_COMMIT:?MONGE_COMMIT must be set}"
: "${RUNNER_TEMP:?RUNNER_TEMP must be set}"
: "${GITHUB_WORKSPACE:?GITHUB_WORKSPACE must be set}"

export JULIA_DEPOT_PATH="${JULIA_DEPOT_PATH:-$RUNNER_TEMP/occt-wrapper-release-depot}"
monge_dir="$GITHUB_WORKSPACE/Monge.jl"
wrapper_dir="$GITHUB_WORKSPACE/native/OpenCascadeCxxWrap"

for command in cmake git gzip julia sha256sum tar; do
  command -v "$command" >/dev/null
done

test -f "$monge_dir/gen/build_local.jl"
test -f "$wrapper_dir/bundled/CMakeLists.txt"

rm -rf "$JULIA_DEPOT_PATH"
mkdir -p "$JULIA_DEPOT_PATH"
test ! -f "$JULIA_DEPOT_PATH/artifacts/Overrides.toml"

cd "$monge_dir"
export JULIA_PKG_PRECOMPILE_AUTO=0
julia --startup-file=no --project=. -e '
  using Pkg
  Pkg.instantiate(verbose=true)
  Pkg.status(; mode=Pkg.PKGMODE_MANIFEST)
' 2>&1 | tee instantiate.log

export MONGE_OPENCASCADE_CXXWRAP_SOURCE="$wrapper_dir"
export MONGE_ALLOW_UNPINNED_CXXWRAP=1
julia --startup-file=no --project=. gen/build_local.jl \
  2>&1 | tee native-build.log

julia --startup-file=no --project=. -e '
  using Pkg
  Pkg.precompile()
  using Monge
  required = (
      :BRep_Tool_FirstParameterOnSurface,
      :BRep_Tool_LastParameterOnSurface,
      :BRep_Tool_CurveOnSurfaceStored,
      :BRep_Tool_IsClosedEdgeFace,
      :BRep_Tool_SameParameter,
      :BRep_Tool_SameRange,
      :HelixBRep_BuilderHelix,
      :GeomCurveArray,
      :GeomFill_Gordon,
      :GeomFill_Gordon_Init,
  )
  missing = filter(name -> !isdefined(Monge.OCC, name), required)
  isempty(missing) || error("candidate wrapper misses bindings: $missing")
  @assert volume(box(1, 1, 1)) ≈ 1
'

julia --startup-file=no --project=. test/runtests.jl \
  2>&1 | tee test.log

julia --startup-file=no --project=. -e '
  using Monge
  import Monge: intersect, union
  include("examples/hydraulic_manifold_block/hydraulic_manifold_block.jl")
  include("examples/hydraulic_manifold_block/isolation_checks.jl")
  generate_manifold_artifacts()
'
julia --startup-file=no --project=. -e '
  using Monge
  path = "examples/hydraulic_manifold_block/artifacts/hydraulic_manifold_hero.png"
  fraction = png_ink_fraction(path)
  println("hero ink fraction = ", fraction)
  fraction > 0.08 || error("hydraulic manifold hero is blank or sparse")
'

built_hash="$(julia --startup-file=no -e '
  using TOML
  path = joinpath(first(DEPOT_PATH), "artifacts", "Overrides.toml")
  overrides = TOML.parsefile(path)
  length(overrides) == 1 || error("expected one artifact override")
  print(only(values(overrides)))
')"
artifact_dir="$JULIA_DEPOT_PATH/artifacts/$built_hash"
library="$artifact_dir/lib/libopencascade_cxxwrap.so"
test -f "$library"

tarball="$RUNNER_TEMP/libopencascade_cxxwrap.tar"
archive="$RUNNER_TEMP/libopencascade_cxxwrap.tar.gz"
tar --sort=name --mtime='@0' --owner=0 --group=0 --numeric-owner \
  -C "$artifact_dir" -cf "$tarball" .
julia --startup-file=no -e '
  using Tar
  expected, tarball = ARGS
  actual = string(Tar.tree_hash(tarball))
  actual == expected || error(
      "archive tree hash $actual does not match artifact $expected",
  )
' "$built_hash" "$tarball"
gzip -n -c "$tarball" > "$archive"
archive_sha256="$(sha256sum "$archive" | awk '{print $1}')"

wrapper_commit="$(git -C "$wrapper_dir" rev-parse HEAD)"
release_tag="libopencascade_cxxwrap-${built_hash:0:8}"
rm -rf release-candidate
mkdir -p release-candidate
cp "$archive" release-candidate/libopencascade_cxxwrap.tar.gz
cat > release-candidate/release-metadata.txt <<EOF
artifact=libopencascade_cxxwrap
tree_hash=$built_hash
archive_sha256=$archive_sha256
release_tag=$release_tag
wrapper_commit=$wrapper_commit
monge_commit=$MONGE_COMMIT
julia_version=$(julia --version)
platform=x86_64-linux-gnu
EOF
cat release-candidate/release-metadata.txt

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "tree_hash=$built_hash"
    echo "archive_sha256=$archive_sha256"
    echo "release_tag=$release_tag"
    echo "wrapper_commit=$wrapper_commit"
  } >> "$GITHUB_OUTPUT"
fi
