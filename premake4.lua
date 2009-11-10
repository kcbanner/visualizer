solution "visualizer"
   configurations { "Debug", "Release" }
   
   project "visualizer"
      kind "ConsoleApp"
      language "C++"
      files { "**.h", "**.hpp", "**.cpp" }
	  links { "SDL", "SDLmain", "opengl32", "glu32", "glut32", "glew32", "fmodex_vc"}
	  flags { "ExtraWarnings" }
	  includedirs { "D:/dev/include" }
	  libdirs { "D:/dev/lib" }
	  
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }