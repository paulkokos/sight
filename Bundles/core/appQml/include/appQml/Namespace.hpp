/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2018.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

/**
 * @ingroup apprequirement
 * @brief  The bundle appXml allows to parse xml configuration of an application QML.
 * See @ref AppConfig for more details.
 *
 * This bundle is automatically started at the launch of the application if it is present in the REQUIREMENTS of the
 * application's Properties.cmake.
 *
 * Example
 * @code{cmake}
    set( NAME Tuto01Basic )
    set( VERSION 0.1 )
    set( TYPE APP )
    set( DEPENDENCIES  )
    set( REQUIREMENTS
        dataReg
        servicesReg
        guiQt # it will be automatically started when the application launches
        fwlauncher
        appXml # it will be automatically started when the application launches
    )
    bundleParam(appXml PARAM_LIST config PARAM_VALUES tutoBasicConfig)
   @endcode
 */
#pragma once

namespace appQml
{

}
