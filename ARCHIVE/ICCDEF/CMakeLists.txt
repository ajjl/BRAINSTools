
##- project(iccdefRegistrationNew)

#-----------------------------------------------------------------------------
# Dependencies.
#

#
# ITK
#

FindITKUtil( ICCDEF_ITK
  ITKImageCompare
  ITKFFT
  ITKConnectedComponents
  ITKMathematicalMorphology
  ITKBinaryMathematicalMorphology
  ITKRegionGrowing
  ITKPDEDeformableRegistration
  ITKRegionGrowing
  ITKSmoothing
  ITKRegistrationCommon
  ITKThresholding
  ITKImageFeature
  ITKSmoothing
  ITKLevelSets
  ITKReview
)

#-------------------------------------------------------------------------------
# OUTPUT DIRECTORIES
#
##- include_directories(${COMMON_BRAINSCOMMONLIB_SOURCE_DIR})
##- link_directories(${COMMON_BRAINSCOMMONLIB_BINARY_DIR})

set(MODULE_FOLDER "Modules-ICCDEF")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/opt/iccdefRegistration_New" CACHE PATH "Install path prefix,
     prepended onto install directories." FORCE)
endif()

set(ALL_PROGS_LIST
  AverageBrainGenerator
  CreateMask
  iccdefRegistration_New
  )

set(ICCDEFLibraries BRAINSCommonLib ${ICCDEF_ITK_LIBRARIES})
DebugImageViewerLibAdditions(ICCDEFLibraries)

foreach(prog ${ALL_PROGS_LIST})
  StandardBRAINSBuildMacro(NAME ${prog} TARGET_LIBRARIES ${ICCDEFLibraries})
endforeach()

# Testing is depended on obsolete ITKv3 testing framework.
# Tests will have to be re-written to build with ITKv4
#
# if(BUILD_TESTING AND NOT BRAINSTools_DISABLE_TESTING)
#   add_subdirectory(TestSuite)
# endif()
