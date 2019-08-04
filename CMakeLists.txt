cmake_minimum_required(VERSION 3.5)

set(MODULE_NAME CleverSeg)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/CleverSeg")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_STATUS "Beta")
set(EXTENSION_CONTRIBUTORS "Jonathan Ramos (ICMC, USP)")
set(EXTENSION_DESCRIPTION "This is the implementation of the CleverSeg method that supports multi-label segmentations")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/CleverSeg.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/CleverSeg/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(CleverSeg)
add_subdirectory(CleverSegEffect)
## NEXT_MODULE


#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
