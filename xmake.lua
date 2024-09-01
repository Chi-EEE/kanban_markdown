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
else 
    add_requires("cosmocc")
end

add_repositories("local-repo xmake/repo")

add_requires("asap-fork", "fmt", "md4c", "pugixml", "tl_expected", "robin-map", "ordered_map")
add_requires("picosha2", "tobiaslocker_base64", "gzip-hpp", "yyjson", "yaml-cpp")
add_requires("cpp-dump")

add_requires("argparse")

add_requires("re2")

target("kanban_markdown", function()
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
end)

if is_plat("windows", "linux", "macosx") then
    target("kanban_markdown-server", function()
        set_kind("binary")
        set_languages("cxx17")

        add_requires("cosmocc")
        set_toolchains("@cosmocc")
        
        add_packages("re2")

        add_headerfiles("server/(**.hpp)")
        add_files("server/*.cpp")

        set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/server")

        add_deps("kanban_markdown", {public = true})
    end)
end

if is_plat("wasm") then 
    target("kanban_markdown-wasm", function()
        set_kind("binary")
        set_languages("cxx17")

        add_packages("emscripten")
        set_toolchains("emcc@emscripten")

        add_headerfiles("wasm/(**.hpp)")
        add_files("wasm/*.cpp")
    end)
end

for _, test_file in ipairs(os.files("tests/test_*.cpp")) do
    target(path.basename(test_file, ".cpp"), function()
        set_kind("binary")
        set_languages("cxx17")
        set_default(false)

        add_tests("default", {pass_outputs = {"Success: .*"}})

        add_files(test_file)

        add_configfiles("(tests/data/**)", {onlycopy = true})
        set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

        set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/tests")

        add_deps("kanban_markdown", {public = true})
    end)
end
