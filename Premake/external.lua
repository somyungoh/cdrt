-- external.lua

function includeExternal()
	includedirs { _SRC_DIR.."/External" }
end

-- GLFW
function linkGLFW()
    libdirs { _SRC_DIR.."/External/GLFW/lib" }
    links { "glfw3" }
end