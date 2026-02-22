include("D:/Licenta/Licenta/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Licenta-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/Licenta/Licenta/build/Desktop_Qt_6_9_3_MSVC2022_64bit-Debug/Licenta.exe"
    GENERATE_QT_CONF
)
