
set( NAME videoPCL )
set( VERSION 0.1 )
set( TYPE MODULE )
set( DEPENDENCIES
        fwCore
        fwData
        fwRuntime
        fwServices
        fwTools
        arData
        fwCom
        fwGui
        arServices
        arPreferences
)
set( REQUIREMENTS  )
set( CONAN_DEPS
    ${CONAN_BOOST}
    ${CONAN_PCL}
)