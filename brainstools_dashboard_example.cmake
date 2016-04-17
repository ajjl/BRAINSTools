# This file is mostly copied from vxl/dash_example.cmake from the dashboard branch of vxl

# BRAINSTools Example Dashboard Script
#
# Copy this example script and edit as necessary for your client.
# See brainstools_common.cmake for more instructions.

# Client maintainer: someone@users.sourceforge.net
#set(CTEST_SITE "machine.site") the default is the hostname
set(CTEST_BUILD_NAME "Linux-gcc")
set(CTEST_BUILD_FLAGS "-j12") # parallel build for makefiles
set(CTEST_BUILD_CONFIGURATION "Debug")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(CTEST_GIT_COMMAND /path/to/git)

# brainstools modules
set(dashboard_cache "
    USE_ANTS=OFF
    USE_AutoWorkup=OFF
    USE_BRAINSABC=OFF
    USE_BRAINSConstellationDetector=OFF
    USE_BRAINSContinuousClass=OFF
    USE_BRAINSCreateLabelMapFromProbabilityMaps=OFF
    USE_BRAINSCut=OFF
    USE_BRAINSDWICleanup=OFF
    USE_BRAINSDemonWarp=OFF
    USE_BRAINSFit=ON
    USE_BRAINSInitializedControlPoints=OFF
    USE_BRAINSLabelStats=OFF
    USE_BRAINSLandmarkInitializer=OFF
    USE_BRAINSMultiModeSegment=OFF
    USE_BRAINSMultiSTAPLE=OFF
    USE_BRAINSMush=OFF
    USE_BRAINSPosteriorToContinuousClass=OFF
    USE_BRAINSROIAuto=OFF
    USE_BRAINSResample=OFF
    USE_BRAINSSnapShotWriter=OFF
    USE_BRAINSStripRotation=OFF
    USE_BRAINSSurfaceTools=OFF
    USE_BRAINSTalairach=OFF
    USE_BRAINSTransformConvert=OFF
    USE_ConvertBetweenFileFormats=OFF
    USE_DWIConvert=OFF
    USE_DebugImageViewer=OFF
    USE_GTRACT=OFF
    USE_ICCDEF=OFF
    USE_ImageCalculator=OFF
    USE_ReferenceAtlas=OFF
")

set(dashboard_model Experimental)
#set(dashboard_model Continuous)

#set(dashboard_do_memcheck 1)
#set(dashboard_do_coverage 1)

#set(dashboard_cache "
#BUILD_SHARED_LIBS:BOOL=ON
#")

include(${CTEST_SCRIPT_DIRECTORY}/brainstools_common.cmake)
