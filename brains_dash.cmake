# This file is mostly copied from vxl/dash_example.cmake from the dashboard branch of vxl

# BRAINSTools Example Dashboard Script
#
# Copy this example script and edit as necessary for your client.
# See brainstools_common.cmake for more instructions.

# Client maintainer: someone@users.sourceforge.net
set(CTEST_SITE "medusa.psychiatry.uiowa.edu") #the default is the hostname
set(CTEST_BUILD_NAME "Linux-gcc")
set(CTEST_BUILD_FLAGS "-j12") # parallel build for makefiles
set(CTEST_BUILD_CONFIGURATION "Debug")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(CTEST_GIT_COMMAND /path/to/git)


set(dashboard_project_name BRAINSTools)
#set(CTEST_BUILD_FLAGS "-j2") # parallel build for makefiles
#set(CTEST_BUILD_CONFIGURATION Release)
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(dashboard_multiple_git_branches 1)
#set(dashboard_git_branch dashboard)
set(dashboard_model Experimental)
#set(dashboard_model Continuous)
set(dashboard_git_url http://github.com/BRAINSIa/BRAINSTools.git)
set(dashboard_do_memcheck 1)
set(dashboard_do_coverage 1)

set(secondary_dashboard 1)
set(secondary_drop_method "https")
set(secondary_site "code-testing.iibi.uiowa.edu")
set(secondary_location "/submit.php?project=ALTest")

set(dashboard_no_clean 1)
# brainstools modules
set(dashboard_cache "
    USE_ANTS=OFF
    USE_AutoWorkup=OFF
    USE_BRAINSABC=OFF
    USE_BRAINSConstellationDetector=ON
    USE_BRAINSContinuousClass=OFF
    USE_BRAINSCreateLabelMapFromProbabilityMaps=OFF
    USE_BRAINSCut=OFF
    USE_BRAINSDWICleanup=OFF
    USE_BRAINSDemonWarp=OFF
    USE_BRAINSFit=OFF
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

include(${CTEST_SCRIPT_DIRECTORY}/common.cmake)



