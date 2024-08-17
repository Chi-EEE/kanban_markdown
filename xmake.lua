add_rules("mode.debug", "mode.release")
if is_kind("shared") then
    add_requireconfs("*", {configs = {shared = true}})
    set_runtimes("MD")
else
    add_requireconfs("*", {configs = {shared = false}})
    set_runtimes("MT")
end

if is_plat("wasm") then
    add_requires("emscripten")
    set_toolchains("emcc@emscripten")
else 
    add_requires("cosmocc")
end

add_repositories("local-repo xmake/repo")

add_requires("asap-fork", "fmt", "md4c", "pugixml", "tl_expected", "robin-map", "ordered_map")
add_requires("picosha2", "tobiaslocker_base64", "gzip-hpp", "yyjson", "yaml-cpp")
add_requires("cpp-dump")

add_requires("argparse")

add_requires("re2")

target("kanban-markdown")
    set_kind("$(kind)")
    set_languages("cxx17")

    if is_plat("windows") then
        add_cxxflags("/utf-8", {public = true})
    end

    add_packages("asap-fork", "fmt", "md4c", "pugixml", "tl_expected", "robin-map", "ordered_map", {public = true})
    add_packages("picosha2", "tobiaslocker_base64", "gzip-hpp", "yyjson", "yaml-cpp", {public = true})
    add_packages("cpp-dump", {public = true})

    add_headerfiles("include/(**.hpp)")
    add_includedirs("include", {public = true})

    add_defines("VC_EXTRALEAN", "WIN32_LEAN_AND_MEAN")
target_end()

target("kanban-markdown_cli")
    set_kind("binary")
    set_languages("cxx17")
    add_packages("argparse")

    add_files("cli/*.cpp")

    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/cli")

    add_deps("kanban-markdown", {public = true})
target_end()

target("kanban-markdown_server")
    set_kind("binary")
    set_languages("cxx17")
    add_packages("re2")
    if not is_plat("wasm") then
        set_toolchains("@cosmocc")
    end

    add_headerfiles("server/(**.hpp)")
    add_files("server/*.cpp")

    after_build(function (target) 
        local target_extension_path = path.join("$(scriptdir)", "extension", "server", "kanban-markdown_server.exe")
        os.cp(path.join(target:targetdir(), "kanban-markdown_server.exe"), target_extension_path)
        local verifiedHash = import("xmake.hash").sha256(io.readfile(target_extension_path, {encoding = "binary"}))
        io.writefile(target_extension_path .. ".sha256", verifiedHash)
    end)

    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/server")

    add_deps("kanban-markdown", {public = true})
target_end()

for _, test_file in ipairs(os.files("tests/test_*.cpp")) do
    target(path.basename(test_file, ".cpp"))
        set_kind("binary")
        set_languages("cxx17")
        set_default(false)

        add_tests("default", {pass_outputs = {"Success: .*"}})

        add_files(test_file)

        add_configfiles("(tests/data/**)", {onlycopy = true})
        set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

        set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/tests")

        add_deps("kanban-markdown", {public = true})
    target_end()
end
