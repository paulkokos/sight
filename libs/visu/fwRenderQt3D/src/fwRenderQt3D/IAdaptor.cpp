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

#include "fwRenderQt3D/IAdaptor.hpp"

#include "fwRenderQt3D/registry/Adaptor.hpp"

#include <fwCom/Slots.hxx>

namespace fwRenderQt3D
{

//-----------------------------------------------------------------------------

static const ::fwCom::Slots::SlotKeyType s_UPDATE_VISIBILITY_SLOT = "updateVisibility";
static const ::fwCom::Slots::SlotKeyType s_TOGGLE_VISIBILITY_SLOT = "toggleVisibility";

//------------------------------------------------------------------------------

IAdaptor::IAdaptor()
{
    newSlot(s_UPDATE_VISIBILITY_SLOT, &IAdaptor::updateVisibility, this);
    newSlot(s_TOGGLE_VISIBILITY_SLOT, &IAdaptor::toggleVisibility, this);
}

//------------------------------------------------------------------------------

IAdaptor::~IAdaptor()
{
}

//------------------------------------------------------------------------------

void IAdaptor::initialize()
{
    // Retrieve the render service attached to the adaptor.
    if(m_renderService.expired())
    {
        auto servicesVector = ::fwServices::OSR::getServices("::fwRenderQt3D::SRender");

        auto& registry       = ::fwRenderQt3D::registry::getAdaptorRegistry();
        auto renderServiceId = registry[this->getID()];

        auto result =
            std::find_if(servicesVector.begin(), servicesVector.end(),
                         [renderServiceId](const ::fwServices::IService::sptr& srv)
            {
                return srv->getID() == renderServiceId;
            });
        SLM_ASSERT("Can't find '" + renderServiceId + "' SRender service.", result != servicesVector.end());

        m_renderService = ::fwRenderQt3D::SRender::dynamicCast(*result);
    }
}

//-----------------------------------------------------------------------------

void IAdaptor::updateVisibility(bool _visibility)
{
    // Enable/disable qt3d entity according to _visibility.
    m_isVisible = _visibility;
}

//-----------------------------------------------------------------------------

void IAdaptor::toggleVisibility()
{
    m_isVisible = !m_isVisible;
    this->updateVisibility(m_isVisible);
}

//------------------------------------------------------------------------------

SRender::sptr IAdaptor::getRenderService() const
{
    return m_renderService.lock();
}

//------------------------------------------------------------------------------

} // namespace fwRenderQt3D.
