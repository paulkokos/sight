/************************************************************************
 *
 * Copyright (C) 2018-2020 IRCAD France
 * Copyright (C) 2018-2020 IHU Strasbourg
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
#include "visuOgreAdaptor/SFrustum.hpp"

#include <fwRenderOgre/IAdaptor.hpp>
#include <fwRenderOgre/ITransformable.hpp>

#include <boost/circular_buffer.hpp>

namespace visuOgreAdaptor
{

/**
 * @brief This adaptor displays a new Frustum each times the transform is updated.
 * The number of Frustum is fixed, if the maximum number of Frustum is reached the oldest one will be replaced.
 *
 * @section Slots Slots
 * - \b updateVisibility(bool): sets whether frustums are shown or not.
 * - \b toggleVisibility(): toggles whether frustums are shown or not.
 * - \b show(): shows frustums.
 * - \b hide(): hides frustums.
 * - \b addFrustum(bool): adds a frustum in the list and displays it.
 * - \b clear(): clears frustum list.
 *
 * @section XML XML Configuration
 *
 * @code{.xml}
        <service type="::visuOgreAdaptor::SFrustumList">
            <in key="camera" uid="..." />
            <in key="transform" uid="..." />
            <config layer="default" near="0.1" far="300" color="#f8e119" transform="..." />
       </service>
   @endcode
 *
 * @subsection Input Input:
 * - \b camera [::arData::Camera]: ::arData::Camera that handles calibration parameters
 * - \b transform [::fwData::TransformationMatrix3D]: each time this transform is modified, a frustum is created.
 *
 * @subsection Configuration Configuration:
 * - \b layer (mandatory, string): defines the frustum's layer
 * - \b near (optional, float, default=1.0): near clipping distance of the ::Ogre::Camera
 * - \b far (optional, float, default=20.0): far clipping distance of the ::Ogre::Camera
 * - \b color (optional, hexadecimal, default=0x0000FF): frustum's color
 * - \b transform (optional, string, default=""): transform applied to the frustumList's scene node
 * - \b visible (optional, bool, default=true): the visibility of the adaptor.
 */
class VISUOGREADAPTOR_CLASS_API SFrustumList final :
    public ::fwRenderOgre::IAdaptor,
    public ::fwRenderOgre::ITransformable
{

public:

    /// Generates default methods as New, dynamicCast, ...
    fwCoreServiceMacro(SFrustumList, ::fwRenderOgre::IAdaptor)

    /// Creates slots.
    VISUOGREADAPTOR_API SFrustumList() noexcept;

    /// Does nothing.
    VISUOGREADAPTOR_API ~SFrustumList() noexcept override;

protected:

    /// Configures.
    VISUOGREADAPTOR_API void configuring() override;

    /// Initializes the material.
    VISUOGREADAPTOR_API void starting() override;

    /**
     * @brief Proposals to connect service slots to associated object signals.
     * @return A map of each proposed connection.
     *
     * Connect ::fwData::TransformationMatrix3D::s_MODIFIED_SIG of s_TRANSFORM_INPUT to s_ADD_FRUSTUM_SLOT
     */
    VISUOGREADAPTOR_API ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Updates the adaptor by attaching new cameras to scene nodes (called after addFrustum slot).
    VISUOGREADAPTOR_API void updating() override;

    /// Clears data.
    VISUOGREADAPTOR_API void stopping() override;

    /**
     * @brief Sets the frustum list visibility.
     * @param _visible the visibility status of the frustum list.
     */
    VISUOGREADAPTOR_API void setVisible(bool _visible) override;

private:

    /// SLOT: clears frustum list.
    void clear();

    /// SLOT: adds a frustum in the list and displays it.
    void addFrustum();

    /// Defines the near clipping distance.
    float m_near { 1.f };

    /// Defines the far clipping distance.
    float m_far { 20.f };

    /// Defines the color of frustum.
    std::string m_color { "#0000FF" };

    /// Defines the maximum capacity of frustum list.
    unsigned int m_capacity {50};

    /// Stores a circular list of frustum adaptors.
    ::boost::circular_buffer< ::Ogre::Camera* > m_frustumList {};

    /// Uses to generate unique ID for each ::Ogre::Camera.
    size_t m_currentCamIndex {0};

    /// Contains the Ogre material adaptor.
    ::visuOgreAdaptor::SMaterial::sptr m_materialAdaptor {nullptr};

    /// Contains the material data.
    ::fwData::Material::sptr m_material {nullptr};

    /// Contains the scene node where all frustums are attached.
    ::Ogre::SceneNode* m_sceneNode { nullptr };

};

} // namespace visuOgreAdaptor.
