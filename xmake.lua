add_rules("mode.debug", "mode.release")
if is_kind("shared") then
    add_requireconfs("*", {configs = {shared = true}})
    set_runtimes("MD")
else
    add_requireconfs("*", {configs = {shared = false}})
    set_runtimes("MT")
end

add_requires("asap", "fmt", "md4c", "tl_expected", "ordered_map", "access_private")
add_requires("picosha2", "tobiaslocker_base64", "yyjson", "yaml-cpp")

add_requires("cpp-dump")

add_requires("argparse")

add_requires("re2", "cosmocc")

target("kanban-markdown")
    set_kind("$(kind)")
    set_languages("cxx17")

    if is_plat("windows") then
        add_cxxflags("/utf-8", {public = true})
    end

    add_packages("asap", "fmt", "md4c", "tl_expected", "ordered_map", "access_private", {public = true})
    add_packages("picosha2", "tobiaslocker_base64", "yyjson", "yaml-cpp", {public = true})

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
    set_toolchains("@cosmocc")
    add_packages("re2")

    add_headerfiles("server/(**.hpp)")
    add_files("server/*.cpp")

    after_build(function (target) 
        os.cp(path.join(target:targetdir(), "kanban-markdown_server.exe"), path.join("$(scriptdir)", "extension", "server", "kanban-markdown_server.exe"))
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
