
set( NAME openvslamTracker )
set( VERSION 0.1 )
set( TYPE MODULE )
set( DEPENDENCIES
        fwCore
        fwCom
        fwData
        fwDataTools
        fwRuntime
        fwServices
        fwThread
        arServices
        fwGui
        arData
        cvIO
        openvslamIO
)
set( REQUIREMENTS )

set( CONAN_DEPS
    ${CONAN_BOOST}
    ${CONAN_OPENCV}
    ${CONAN_EIGEN}
    ${CONAN_OPENVSLAM})
