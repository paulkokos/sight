fwLoadProperties()

find_package(Qt5 QUIET COMPONENTS Core Gui Widgets REQUIRED)
target_link_libraries(${FWPROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)

set_target_properties(${FWPROJECT_NAME} PROPERTIES AUTOMOC TRUE)
