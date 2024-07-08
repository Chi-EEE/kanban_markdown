add_rules("mode.debug", "mode.release")
if is_kind("shared") then
    add_requireconfs("*", {configs = {shared = true}})
    set_runtimes("MD")
else
    add_requireconfs("*", {configs = {shared = false}})
    set_runtimes("MT")
end

add_requires("fmt", "re2", "md4c", "tl_expected", "ordered_map")
add_requires("argparse")

target("kanban-markdown")
    set_kind("$(kind)")
    set_languages("cxx17")

    add_packages("fmt", "re2", "md4c", "tl_expected", "ordered_map", {public = true})

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
