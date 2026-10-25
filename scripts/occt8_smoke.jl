# Load libopencascade_cxxwrap built against the published OCCT 8.0.1
# artifact and run a box, a volume, a Boolean, a validity check, and a
# STEP round trip. A successful dlopen is not the result.
#
#   julia --compiled-modules=no --project=<env with CxxWrap 0.17> \
#     scripts/occt8_smoke.jl <occt-prefix> <libopencascade_cxxwrap>

using CxxWrap
using Libdl

length(ARGS) == 2 || error("usage: occt8_smoke.jl <occt-prefix> <wrapper-library>")
const prefix = abspath(ARGS[1])
const wrapper = abspath(ARGS[2])
isdir(prefix) || error("OCCT prefix does not exist: $prefix")
isfile(wrapper) || error("wrapper library does not exist: $wrapper")

const version_header = joinpath(prefix, "include", "opencascade", "Standard_Version.hxx")
isfile(version_header) || error("missing OCCT version header: $version_header")
occursin("#define OCC_VERSION_COMPLETE \"8.0.1\"", read(version_header, String)) ||
    error("prefix is not OCCT 8.0.1: $prefix")

const resources = joinpath(prefix, "share", "opencascade", "resources")
isdir(resources) || error("missing OCCT resources: $resources")
previous_resources = get(ENV, "CSF_OCCTResourcePath", nothing)
if previous_resources !== nothing && abspath(previous_resources) != abspath(resources)
    error("CSF_OCCTResourcePath already points outside this OCCT 8.0.1 prefix: $previous_resources")
end
ENV["CSF_OCCTResourcePath"] = resources

const libdir = joinpath(prefix, "lib")
const toolkit_pattern = Sys.isapple() ? r"^libTK.+\.8\.0\.1\.dylib$" : r"^libTK.+\.so\.8\.0\.1$"
const toolkits = sort!(filter(name -> occursin(toolkit_pattern, name), readdir(libdir)))
length(toolkits) == 48 || error("expected 48 OCCT 8.0.1 toolkits, found $(length(toolkits)) in $libdir")

const flags = Libdl.RTLD_NOW | Libdl.RTLD_GLOBAL
for name in toolkits
    Libdl.dlopen(joinpath(libdir, name), flags)
end

module OCCTSmoke
using CxxWrap
using Libdl

const library = abspath(ARGS[2])
@wrapmodule(() -> library, :define_julia_module, Libdl.RTLD_LAZY | Libdl.RTLD_GLOBAL)

function __init__()
    @initcxx
end
end

using .OCCTSmoke

OCCTSmoke.OCCT_VersionComplete() == "8.0.1" ||
    error("wrapper reports OCCT $(OCCTSmoke.OCCT_VersionComplete()), not 8.0.1")
OCCTSmoke.OCCT_RWGltfCafWriterBound() == false ||
    error("OCCT 8.0.1 archive was built without RWGltf_CafWriter, but the binding is live")

function volume_of(shape)
    properties = OCCTSmoke.GProp_GProps()
    OCCTSmoke.BRepGProp_VolumeProperties(shape, properties)
    return OCCTSmoke.Mass(properties)
end

function require_valid(shape, label)
    OCCTSmoke.IsValid(OCCTSmoke.BRepCheck_Analyzer(shape)) ||
        error("$label is not a valid shape")
    return shape
end

box = require_valid(OCCTSmoke.Shape(OCCTSmoke.BRepPrimAPI_MakeBox(1.0, 2.0, 3.0)), "box")
volume = volume_of(box)
isapprox(volume, 6.0; atol = 1e-9) || error("box volume $volume is not 6")

stock = OCCTSmoke.Shape(OCCTSmoke.BRepPrimAPI_MakeBox(10.0, 20.0, 30.0))
tool = OCCTSmoke.Shape(OCCTSmoke.BRepPrimAPI_MakeBox(5.0, 10.0, 30.0))
cut = OCCTSmoke.BRepAlgoAPI_Cut(stock, tool)
OCCTSmoke.IsDone(cut) || error("Boolean cut did not finish")
cut_shape = require_valid(OCCTSmoke.Shape(cut), "Boolean cut")
cut_volume = volume_of(cut_shape)
isapprox(cut_volume, 4500.0; atol = 1e-6) || error("Boolean volume $cut_volume is not 4500")

step_path = joinpath(mktempdir(), "roundtrip.step")
writer = OCCTSmoke.STEPControl_Writer()
OCCTSmoke.Transfer(writer, cut_shape, 0) == 1 || error("STEP transfer failed")
OCCTSmoke.Write(writer, step_path) == 1 || error("STEP write failed")
reader = OCCTSmoke.STEPControl_Reader()
OCCTSmoke.ReadFile(reader, step_path) == 1 || error("STEP read failed")
OCCTSmoke.TransferRoots(reader) > 0 || error("STEP transfer produced no roots")
imported = require_valid(OCCTSmoke.OneShape(reader), "imported STEP shape")
imported_volume = volume_of(imported)
isapprox(imported_volume, cut_volume; atol = 1e-5) ||
    error("STEP round trip changed volume from $cut_volume to $imported_volume")

loaded = Libdl.dllist()
occt8 = filter(path -> occursin("libTK", path) && occursin(prefix, path), loaded)
occt79 = filter(path -> occursin(r"libTK.*7\.9", path) || occursin("libTK.7.9", path), loaded)
wrapper_hits = filter(path -> occursin("libopencascade_cxxwrap", path), loaded)
isempty(occt79) || error("OCCT 7.9 libraries are loaded:\n$(join(occt79, '\n'))")
length(unique(basename.(occt8))) >= 48 ||
    error("expected the OCCT 8.0.1 toolkits in dllist, found $(length(unique(basename.(occt8))))")
length(wrapper_hits) == 1 || error("wrapper was not the only cxxwrap library: $wrapper_hits")
occursin(wrapper, first(wrapper_hits)) || error("loaded wrapper is $(first(wrapper_hits))")
cxx = filter(path -> occursin("libcxxwrap_julia", path), loaded)
isempty(cxx) && error("libcxxwrap_julia was not loaded")
length(unique(dirname.(cxx))) == 1 || error("multiple libcxxwrap_julia libraries are loaded: $cxx")

println("OCCT 8.0.1 Julia smoke PASS")
println("version ", OCCTSmoke.OCCT_VersionComplete())
println("box_volume ", volume)
println("boolean_volume ", cut_volume)
println("step_volume ", imported_volume)
println("toolkits ", length(unique(basename.(occt8))))
println("wrapper ", first(wrapper_hits))
println("gltf_bound ", OCCTSmoke.OCCT_RWGltfCafWriterBound())
