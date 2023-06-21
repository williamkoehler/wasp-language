add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_cxxflags("-fPIC")
add_cxxflags("-Wall", "-Wextra", "-Wpedantic")

if is_mode("debug") then
    add_cxxflags("-fstandalone-debug", { force = true })
    set_optimize("none")
    add_defines("DEBUG")
else
    add_defines("NDEBUG")
end

set_languages("cxx20", "clatest")

includes("wasp-lang")
includes("sandbox")