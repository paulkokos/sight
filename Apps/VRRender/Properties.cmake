
set( NAME VRRender )
set( VERSION 0.9 )
set( TYPE APP )
set( DEPENDENCIES  )
set( REQUIREMENTS
    servicesReg
    dataReg

    gui
    guiQt

    activities
    appXml
    memory
    fwlauncher

    ioAtoms
    ioITK
    ioVTK
    ioVtkGdcm
    ioData

    uiIO
    uiGenericQt
    uiMedDataQt
    uiPreferences
    uiActivitiesQt

    2DVisualizationActivity
    3DVisualizationActivity
    volumeRenderingActivity
    blendActivity
    ioActivity

    dicomFilteringActivity
    dicomPacsReaderActivity
    dicomPacsWriterActivity

    DicomWebReaderActivity
    DicomWebWriterActivity

    media
    style

    patchMedicalData

    filterUnknownSeries

    preferences
)

bundleParam(guiQt
    PARAM_LIST
        resource
        stylesheet
    PARAM_VALUES
        style-0.1/darkstyle.rcc
        style-0.1/darkstyle.qss
)
bundleParam(appXml PARAM_LIST config parameters PARAM_VALUES VRRenderBase VRRenderAppBase)
