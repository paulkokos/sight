/************************************************************************
 *
 * Copyright (C) 2017-2019 IRCAD France
 * Copyright (C) 2017-2019 IHU Strasbourg
 *
 * This file is part of Sight.
 *
 * Sight is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Sight. If not, see <https://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#pragma once

#include "opItkRegistration/config.hpp"

#include <fwServices/IOperator.hpp>

namespace opItkRegistration
{

/**
 * @brief Service computing the center of a 3D image, it outputs the translation going from origin to the center.
 *
 * @section Signals Signals
 * - \b computed() : Signal sent after computing center.
 *
 * @section XML XML Configuration
 * @code{.xml}
   <service type="::opItkRegistration::SImageCenter">
       <in key="image" uid="..." />
       <inout key="transform" uid="..." />
   </service>
   @endcode
 *
 * @subsection Input Input
 * - \b image [::fwData::Image]: Input image on which the center will be computed.
 *
 * @subsection In-Out In-Out
 * - \b transform [::fwData::TransformationMatrix3D]: Will be first reset to identity and then will contain translation
 * from world origin(0,0,0) to the image center (in mm).
 *
 */
class OPITKREGISTRATION_CLASS_API SImageCenter : public ::fwServices::IOperator
{
public:

    fwCoreServiceMacro(SImageCenter, ::fwServices::IOperator);

    /// Constructor, does nothing.
    OPITKREGISTRATION_API SImageCenter();

    /// Destructor, does nothing.
    OPITKREGISTRATION_API ~SImageCenter();

protected:

    /// Configure registration parameters.
    OPITKREGISTRATION_API virtual void configuring() override;

    /// Does nothing.
    OPITKREGISTRATION_API virtual void starting() override;

    /// Do the registration.
    OPITKREGISTRATION_API virtual void updating() override;

    /// Does nothing.
    OPITKREGISTRATION_API virtual void stopping() override;

    /**
     * @brief Auto connections
     *
     * - Update service when one of the two images is modified.
     * - Update service when the transform matrix is modified.
     */
    OPITKREGISTRATION_API virtual KeyConnectionsMap getAutoConnections() const override;

};

} // namespace opItkRegistration
