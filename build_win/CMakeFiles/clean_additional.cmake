# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\facecontrol_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\facecontrol_autogen.dir\\ParseCache.txt"
  "facecontrol_autogen"
  )
endif()
