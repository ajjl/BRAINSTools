##- project(ImageRegionPlotter)

## Build all the programs
##
set(ALL_PROGS_LIST
  ImageRegionPlotter
  )
foreach( prog ${ALL_PROGS_LIST} )
  StandardBRAINSBuildMacro( NAME ${prog} TARGET_LIBRARIES BRAINSCommonLib ${BRAINSCut_ITK_LIBRARIES} )
endforeach()
