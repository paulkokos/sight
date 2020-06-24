/************************************************************************
 *
 * Copyright (C) 2014-2020 IRCAD France
 * Copyright (C) 2014-2020 IHU Strasbourg
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
#include "visuOgreAdaptor/SMaterial.hpp"
#include "visuOgreAdaptor/STransform.hpp"

#include <fwData/Material.hpp>
#include <fwData/PointList.hpp>

#include <fwRenderOgre/IAdaptor.hpp>
#include <fwRenderOgre/ITransformable.hpp>
#include <fwRenderOgre/Mesh.hpp>
#include <fwRenderOgre/Text.hpp>

#include <OGRE/OgreEntity.h>

namespace fwData
{
class Material;
}
namespace fwData
{
class Mesh;
}

namespace visuOgreAdaptor
{

/**
 * @brief This adaptor shows a point lists using billboards generated by a geometry shader.
 *
 * This class handles the display of a ::fwData::PointList or a ::fwData::Mesh. Both are exclusive you can only specify
 * one of the two.
 *
 * @section Slots Slots
 * - \b updateVisibility(bool): Sets whether the points are visible or not.
 * - \b toggleVisibility(): Toggle whether the points are visible or not.
 *
 * @section XML XML Configuration
 * @code{.xml}
    <service uid="..." type="::visuOgreAdaptor::SPointList" >
        <in key="pointList" uid="..." />
        <config layer="..." transform="..." textureName="..." radius="1.0" fontSource="DejaVuSans.ttf" fontSize="16"
               labelColor="#0xFFFFFF" visible="true" fixedSize="false" queryFlags="0x40000000" displayLabel="false"/>
    </service>
   @endcode
 *
 * @subsection In-Out In-Out
 * - \b pointList [::fwData::PointList] (optional): point list to display.
 * - \b mesh [::fwData::Mesh] (optional): point based mesh to display. If the mesh contains any topology, it will be
 * ignored and only raw vertices will be displayed.
 * or add some fields.
 *
 * @subsection Configuration Configuration:
 * - \b layer (mandatory, string) : defines the mesh's layer.
 * - \b autoresetcamera (optional, yes/no, default=yes): reset the camera when this mesh is modified, "yes" or "no".
 * - \b transform (optional, string, default="") : the name of the Ogre transform node where to attach the mesh, as it
 * was specified
 * in the STransform adaptor.
 * Either of the following (whether a material is configured in the XML scene or not) :
 * - \b materialName (optional, string, default="") : name of the Ogre material, as defined in the
 *::visuOgreAdaptor::SMaterial you want
 * to be bound to.
 * Only if there is no material configured in the XML scene (in this case, it has to retrieve the material
 * template, the texture adaptor and the shading mode) :
 * - \b materialTemplate (optional, string, default='Billboard_Default') : the name of the base Ogre material for the
 * internally
 * created SMaterial.
 * - \b textureName (optional, string, default="") : the name of the Ogre texture that the mesh will use.
 * - \b radius (optional, float, default=1.f) : billboard radius.
 * - \b displayLabel (optional, bool, default=false) : display the label points (default = false).
 * - \b labelColor (optional, hexadecimal, default=0xFFFFFF): color of the label in hexadecimal.
 * - \b color (optional, hexadecimal, default=#FFFFFFFF): color of the texture in hexadecimal.
 * - \b fixedSize (optional, bool, default=false) : if true, the billboard will have a fixed size in screen space.
 * - \b queryFlags (optional, uint32, default=0x40000000): Picking flags. Points can be picked by pickers with a
 * matching mask.
 * - \b visible (optional, bool, default=true): If pointlist should be visible or not at start.
 * - \b fontSource (optional, string, default=DejaVuSans.ttf): TrueType font (*.ttf) source file.
 * - \b fontSize (optional, unsigned int, default=16): font size in points.
 */
class VISUOGREADAPTOR_CLASS_API SPointList final :
    public ::fwRenderOgre::IAdaptor,
    public ::fwRenderOgre::ITransformable
{

public:

    fwCoreServiceMacro(SPointList, ::fwRenderOgre::IAdaptor)

    /// Creates the adaptor, sets default parameters and initializes necessary members.
    VISUOGREADAPTOR_API SPointList() noexcept;

    /// Destoyes Ogre resource.
    VISUOGREADAPTOR_API virtual ~SPointList() noexcept;

private:

    /// Configures the adaptor.
    virtual void configuring() override;

    /// Creates a mesh in the default Ogre resource group.
    virtual void starting() override;

    /**
     * @brief Proposals to connect service slots to associated object signals.
     * @return A map of each proposed connection.
     *
     * Connect ::fwData::PointList::s_POINT_ADDED_SIG of s_POINTLIST_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::PointList::s_POINT_REMOVED_SIG of s_POINTLIST_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::PointList::s_MODIFIED_SIG of s_POINTLIST_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::Mesh::s_VERTEX_MODIFIED_SIG of s_MESH_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::Mesh::s_MODIFIED_SIG of s_MESH_INPUT to s_UPDATE_SLOT
     */
    virtual ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Updates the generated mesh.
    virtual void updating() override;

    /// Deletes the mesh after unregistering the service, and shutting connections.
    virtual void stopping() override;

    /**
     * @brief Get the point list visibility.
     * @return True if the point list is visible.
     */
    bool getVisibility() const;

    /**
     * @brief Sets whether the point list is to be seen or not.
     * @param _isVisible set to true to show the point list.
     */
    void updateVisibility(bool _isVisible);

    /// Toggle the visibility of the point list.
    void toggleVisibility();

    /**
     * @brief Updates the point list from a point list, checks if color, number of vertices have changed, and updates
     * them.
     * @param _pointList point list used for the update.
     */
    void updateMesh(const fwData::PointList::csptr& _pointList);

    /**
     * @brief Updates the point list from a mesh, checks if color, number of vertices have changed, and updates them.
     * @param _mesh mesh used for the update.
     */
    void updateMesh(const fwData::Mesh::csptr& _mesh);

    /**
     * @brief Instantiates a new material adaptor.
     * @param _materialSuffix suffix use for the material name.
     */
    ::visuOgreAdaptor::SMaterial::sptr createMaterialService(const std::string& _materialSuffix = "");

    /// Associates a new SMaterial to the managed SPointList.
    /// With this method, SPointList is responsible for creating a SMaterial.
    void updateMaterialAdaptor();

    /**
     * @brief Attachs a node in the scene graph.
     * @param _node node to attach.
     */
    void attachNode(::Ogre::MovableObject* _node);

    /// Detachs and destroy @ref m_entity from the scene graph.
    void detachAndDestroyEntity();

    /**
     * @brief Creates all the labels and attach them to the sceneNode vector.
     * @param _pointList point list used to retreive each point informations.
     */
    void createLabel(const ::fwData::PointList::csptr& _pointList);

    /// Destroys all the labels and delete them from the sceneNode vector.
    void destroyLabel();

    /// Defines whether the camera must be auto reset when a mesh is updated or not.
    bool m_autoResetCamera { true };

    /// Defines whether the material was set by the user or not.
    bool m_customMaterial { false };

    /// Contains the node in the scene graph.
    ::Ogre::Entity* m_entity { nullptr };

    /// Contains the material attached to the mesh.
    ::visuOgreAdaptor::SMaterial::sptr m_materialAdaptor { nullptr };

    /// Contains the Ogre material related to the mesh.
    ::fwData::Material::sptr m_material { nullptr };

    /// Defines the attached material's name.
    std::string m_materialTemplateName { "Billboard_Default" };

    /// Definees the attached texture adaptor UID.
    std::string m_textureName {""};

    /// Defines if the entity visible or not.
    bool m_isVisible {true};

    /// Contains the mesh support used to render the pointlist.
    ::fwRenderOgre::Mesh::sptr m_meshGeometry {nullptr};

    /// Defines the billboards scale.
    float m_radius {1.f};

    /// Defines if label numbers are displayed.
    bool m_displayLabel {false};

    /// Contains the RGB color for the label point color.
    ::fwData::Color::sptr m_labelColor {nullptr};

    /// Defines the mask for picking requests.
    std::uint32_t m_queryFlags {::Ogre::SceneManager::ENTITY_TYPE_MASK};

    /// Stores label of each point.
    std::vector< ::fwRenderOgre::Text* > m_labels;

    /// Stores label points nodes.
    std::vector< ::Ogre::SceneNode* > m_nodes;

    /// Contains the scene node where all of our manual objects are attached.
    ::Ogre::SceneNode* m_sceneNode { nullptr };

    /// Defines the TrueType font source file.
    std::string m_fontSource {"DejaVuSans.ttf"};

    /// Defines the font size in points.
    size_t m_fontSize { 16 };

};

//------------------------------------------------------------------------------

inline bool SPointList::getVisibility() const
{
    return m_entity ? m_entity->getVisible() : m_isVisible;
}

} // namespace visuOgreAdaptor.