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
    set_toolchains("@cosmocc")
end

add_repositories("local-repo xmake/repo")

add_requires("asap-fork", "md4c", "pugixml", "tl_expected", "robin-map", "ordered_map")
add_requires("fmt")
add_requires("picosha2", "tobiaslocker_base64", "gzip-hpp", "yyjson", "yaml-cpp")
add_requires("cpp-dump")

if is_plat("windows") then
    add_cxxflags("/utf-8", {public = true})
end

add_requires("argparse")

add_requires("re2")

target("kanban_markdown", function()
    set_kind("$(kind)")
    set_languages("cxx17")

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
        add_ldflags("--bind")

        add_headerfiles("wasm/(**.hpp)")
        add_files("wasm/*.cpp")
    
        add_deps("kanban_markdown", {public = true})
    end)
end
