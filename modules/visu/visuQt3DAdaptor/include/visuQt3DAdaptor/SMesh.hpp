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

#include "visuQt3DAdaptor/config.hpp"

#include <fwRenderQt3D/data/Mesh.hpp>
#include <fwRenderQt3D/IAdaptor.hpp>

namespace visuQt3DAdaptor
{

/**
 * @brief This adaptor shows individual meshes.
 *
 * This class handles the conversion of ::fwData::Mesh to Qt3D. It can handle triangles.
 *
 * @section Slot Slot
 * - \b updateVisibility(bool): shows or hides the mesh.
 * - \b toggleVisibility(): Toggle whether the adaptor is shown or not.
 * - \b modifyVertices(): called when the vertices are modified.
 *
 * @section XML XML Configuration
 * @code{.xml}
    <service uid="..." type="::visuQt3DAdaptor::SMesh" >
        <in key="mesh" uid="..." />
        <config autoresetcamera="yes" visible="yes" />
    </service>
   @endcode
 *
 * @subsection Input Input
 * - \b mesh [::fwData::Mesh]: adapted mesh.
 *
 * @subsection Configuration Configuration:
 *  - \b autoresetcamera (optional, bool, default=false): reset the camera when this mesh is modified.
 *  - \b visible (optional, bool, default=true): set the initial visibility of the mesh.
 */
class VISUQT3DADAPTOR_CLASS_API SMesh : public ::fwRenderQt3D::IAdaptor
{

public:

    /// Generates default methods as New, dynamicCast, ...
    fwCoreServiceMacro(SMesh, ::fwRenderQt3D::IAdaptor)

    /// Sets default parameters and initializes necessary members.
    VISUQT3DADAPTOR_API SMesh() noexcept;

    /// Destroys the adaptor.
    VISUQT3DADAPTOR_API ~SMesh() noexcept override;

private:

    /// Configures the adaptor.
    void configuring() override;

    ///
    void starting() override;

    /**
     * @brief Proposals to connect service slots to associated object signals.
     * @return A map of each proposed connection.
     *
     * Connect ::fwData::Image::s_MODIFIED_SIG of s_MESH_INOUT to s_UPDATE_SLOT.
     * Connect ::fwData::Image::s_VERTEX_MODIFIED_SIG of s_MESH_INOUT to s_MODIFY_VERTICES_SLOT.
     */
    ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Updates the mesh.
    void updating() override;

    /// Does nothing.
    void stopping() override;

    /**
     * @brief Sets whether the mesh is to be seen or not.
     * @param _visibility the visibility status of the volume.
     */
    void updateVisibility(bool _visibility) override;

    /// Updates mesh vertices.
    void modifyVertices();

    /// Contains a Qt3D mesh.
    QPointer< ::fwRenderQt3D::data::Mesh > m_mesh;

    /// Defines whether the camera must be auto reset when a mesh is updated or not.
    bool m_autoResetCamera { false };

};

} // namespace visuQt3DAdaptor.
