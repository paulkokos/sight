
set( NAME arPatchMedicalDataTest )
set( VERSION  )
set( TYPE TEST )
set( DEPENDENCIES
    fwTest
    fwCore
    fwData
    fwServices
    fwRuntime
    fwMedData
    fwThread
)
set( REQUIREMENTS dataReg servicesReg ioAtoms patchMedicalData arPatchMedicalData)

set(CPPUNITTEST_OPTIONS BUNDLE arPatchMedicalData WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
