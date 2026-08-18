include(FetchContent)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        27b68329de32cf9c601ca8eb6c588fd639960c40
    GIT_PROGRESS TRUE
) 
message("pugixml")
FetchContent_MakeAvailable(pugixml)
set_property(TARGET pugixml PROPERTY CXX_STANDARD 23)
