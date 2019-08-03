cmake_minimum_required(VERSION 3.5)

project(CleverSeg)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SliceGrowth")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Jonathan Ramos (University of Sao Paulo)")
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
