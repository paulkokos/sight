/************************************************************************
 *
 * Copyright (C) 2020 IRCAD France
 * Copyright (C) 2020 IHU Strasbourg
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

#include "visuOgreAdaptor/config.hpp"

#include <fwRenderOgre/IAdaptor.hpp>
#include <fwRenderOgre/Layer.hpp>

namespace visuOgreAdaptor
{

/**
 * @brief This adaptor resizes and moves viewports.
 *
 * @section Slots Slots
 * - \b resize(): resizes the viewport to the new size.
 * - \b revert(): revert back to the initial viewport size.
 *
 * @section XML XML Configuration
 * @code{.xml}
    <service type="::visuOgreAdaptor::SResizeViewport" >
        <config layer="default" hOffset="0" vOffset="0" width="1" height="1" hAlign="left" vAlign="top"/>
    </service>
   @endcode
 *
 * @subsection Configuration Configuration:
 * - \b layer (mandatory, string): layer whose viewport is resized
 * - \b hAlign (optional, left|center|right, default=left): defines the horizontal origin of the new viewport.
 * - \b vAlign (optional, top|center|bottom, default=top): defines the vertical origin of the new viewport.
 * - \b hOffset (optional, float, default=0.f): horizontal offset from the origin relatively to the window.
 * - \b vOffset (optional, float, default=0.f): vertical offset from the origin relatively to the window.
 * - \b width (optional, float, default=1.f): new viewport width relatively to the window.
 * - \b height (optional, float, default=1.f): new viewport height relatively to the window.
 */
class VISUOGREADAPTOR_CLASS_API SResizeViewport final : public ::fwRenderOgre::IAdaptor
{

public:

    /// Generates default methods as New, dynamicCast, ...
    fwCoreServiceMacro(SResizeViewport, ::fwRenderOgre::IAdaptor)

    /// Initializes slots.
    VISUOGREADAPTOR_API SResizeViewport() noexcept;

    /// Does nothing.
    VISUOGREADAPTOR_API ~SResizeViewport() noexcept override;

protected:

    /// Configures the new viewport's dimensions.
    VISUOGREADAPTOR_API void configuring() override;

    /// Starts the adaptor, records the initial viewport dimensions.
    VISUOGREADAPTOR_API void starting() override;

    /// Updates the adaptor. Unused here.
    VISUOGREADAPTOR_API void updating() noexcept override;

    /// Stops the adaptor. Unused here.
    VISUOGREADAPTOR_API void stopping() noexcept override;

private:

    /// Switches to the initial/new viewport depending on the input.
    void resizeViewport(bool _resize);

    /// Defines the configured viewports dimensions on the layer.
    void resize();

    /// Defines the original viewports dimensions on the layer.
    void revert();

    /// Defines the initial viewport dimensions.
    ::fwRenderOgre::Layer::ViewportConfigType m_previousViewportDimensions { 0.f, 0.f, 1.f, 1.f };

    /// Defines the configured viewport dimensions.
    ::fwRenderOgre::Layer::ViewportConfigType m_newViewportDimensions { 0.f, 0.f, 1.f, 1.f };

};

} // namespace visuOgreAdaptor.
