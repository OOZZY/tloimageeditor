qt5_wrap_ui(uiheaders imageeditorview.ui)
add_library(tloimageeditorcore STATIC imageeditormodel.cpp ${uiheaders} imageeditorview.cpp)
# linking to Qt5::<module> causes cmake to use the appropriate include
# directories and compile definitions needed to use Qt <module> when building
# the target
target_link_libraries(tloimageeditorcore Qt5::Widgets)
