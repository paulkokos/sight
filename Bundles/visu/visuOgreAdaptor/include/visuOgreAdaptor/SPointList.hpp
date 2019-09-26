/************************************************************************
 *
 * Copyright (C) 2014-2019 IRCAD France
 * Copyright (C) 2014-2019 IHU Strasbourg
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
 * @brief This adaptor shows point lists using billboards generated by a geometry shader.
 *
 * This class handles the display of a ::fwData::PointList or a ::fwData::Mesh. Both are exclusive you can only specify
 * one of the two.
 *
 * @section Slots Slots
 * - \b updateVisibility(bool): Sets whether the points has to be seen or not.
 * - \b update(): Called when the point list is modified.
 *
 * @section XML XML Configuration
 * @code{.xml}
    <service uid="..." type="::visuOgreAdaptor::SPointList" >
        <in key="pointList" uid="..." />
        <config layer="..." transform="..."
                textureName="..." radius="2.4" displayLabel="true" charHeight="0.02" labelColor="#0000ff"
                visible="true"/>
    </service>
   @endcode
 * @subsection In-Out In-Out
 * - \b pointList [::fwData::PointList] (optional): point list to display.
 * - \b mesh [::fwData::Mesh] (optional): point based mesh to display. If the mesh contains any topology, it will be
 * ignored and only raw vertices will be displayed.
 * or add some fields.
 * @subsection Configuration Configuration:
 * - \b layer (mandatory) : defines the mesh's layer.
 * - \b autoresetcamera (optional, default='yes'): reset the camera when this mesh is modified, "yes" or "no".
 * - \b transform (optional) : the name of the Ogre transform node where to attach the mesh, as it was specified
 * in the STransform adaptor.
 * Either of the following (whether a material is configured in the XML scene or not) :
 * - \b materialName (optional) : name of the Ogre material, as defined in the ::visuOgreAdaptor::SMaterial you want
 * to be bound to.
 * Only if there is no material configured in the XML scene (in this case, it has to retrieve the material
 * template, the texture adaptor and the shading mode) :
 * - \b materialTemplate (optional, default='Billboard_Default') : the name of the base Ogre material for the internally
 * created SMaterial.
 * - \b textureName (optional) : the name of the Ogre texture that the mesh will use.
 * - \b radius (optional, default=1.f) : billboard radius.
 * - \b displayLabel (optional, default=false) : display the label points (default = false).
 * - \b charHeight (optional): size of the character label (default = 0.03).
 * - \b labelColor (optional, default=0xFFFFFF): color of the label in hexadecimal (default = white).
 * - \b fixedSize (optional, default="false") : if true, the billboard will have a fixed size in screen space.
 * - \b queryFlags (optional, default=0x40000000) : Used for picking. Picked only by pickers whose mask that match the
 * flag.
 * - \b visible (optional, default=true): If pointlist should be visible or not at start.
 */
class VISUOGREADAPTOR_CLASS_API SPointList : public ::fwRenderOgre::IAdaptor,
                                             public ::fwRenderOgre::ITransformable
{
public:
    fwCoreServiceMacro(SPointList, ::fwRenderOgre::IAdaptor);

    /// Sets default parameters and initializes necessary members.
    VISUOGREADAPTOR_API SPointList() noexcept;

    /// If an entity exists in the Ogre Scene, asks Ogre to destroy it.
    VISUOGREADAPTOR_API virtual ~SPointList() noexcept;

    /**
     * @brief getMaterial Get the associated material.
     * @return The material.
     */
    VISUOGREADAPTOR_API ::fwData::Material::sptr getMaterial() const;

    /**
     * @brief Sets the current material.
     * @param _material The new material.
     */
    VISUOGREADAPTOR_API void setMaterial(::fwData::Material::sptr _material);

    /**
     * @brief Sets the material template Name.
     * @param _materialName The material name.
     */
    VISUOGREADAPTOR_API void setMaterialTemplateName(const std::string& _materialName);

    /**
     * @brief Active/Inactive automatic reset on camera.
     * @param _autoResetCamera Use true to activate it.
     */
    VISUOGREADAPTOR_API void setAutoResetCamera(bool _autoResetCamera);

    /**
     * @brief Get the associated entity.
     * @return The entity.
     */
    VISUOGREADAPTOR_API ::Ogre::Entity* getEntity() const;

    /**
     * @brief Get the point list visibility.
     * @return True if the point list is visible.
     */
    VISUOGREADAPTOR_API bool getVisibility() const;

    /**
     * @brief Sets whether the point list is to be seen or not.
     * @param _isVisible Set to true to show the point list.
     */
    VISUOGREADAPTOR_API void updateVisibility(bool _isVisible);

    /**
     * @brief Returns proposals to connect service slots to associated object signals.
     * @return The connection map proposals.
     */
    VISUOGREADAPTOR_API ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Ask the render service (SRender) to update - we also flag the r2vb objects as dirty.
    VISUOGREADAPTOR_API virtual void requestRender() override;

private:

    /// Configures the adaptor.
    void configuring() override;

    /// Manually creates a Mesh in the Default Ogre Ressource group.
    void starting() override;

    /// Deletes the mesh after unregistering the service, and shutting connections.
    void stopping() override;

    /// Called when the mesh is modified
    void updating() override;

    /**
     * @brief Updates the point list from a point list, checks if color, number of vertices have changed, and updates
     * them.
     * @param _pointList The point list used for the update.
     */
    void updateMesh(const fwData::PointList::csptr& _pointList);

    /**
     * @brief Updates the point list from a mesh, checks if color, number of vertices have changed, and updates them.
     * @param _mesh The mesh used for the update.
     */
    void updateMesh(const fwData::Mesh::csptr& _mesh);

    /**
     * @brief Instantiates a new material adaptor
     * @param _materialSuffix Suffix use for the material name.
     */
    ::visuOgreAdaptor::SMaterial::sptr createMaterialService(const std::string& _materialSuffix = "");

    /// Associates a new SMaterial to the managed SPointList.
    /// With this method, SPointList is responsible for creating a SMaterial.
    void updateMaterialAdaptor();

    /**
     * @brief Attach a node in the scene graph.
     * @param _node The node to attach.
     */
    void attachNode(::Ogre::MovableObject* _node);

    /// Detach and destroy m_entity in the scene graph.
    void detachAndDestroyEntity();

    /**
     * @brief Create all the labels and attach them to the sceneNode vector
     * @param _pointList The point list used to retreive each point informations.
     */
    void createLabel(const ::fwData::PointList::csptr& _pointList);

    /// Destroy all the labels and delete them from the sceneNode vector
    void destroyLabel();

    /// Sets whether the camera must be auto reset when a mesh is updated or not.
    bool m_autoResetCamera;

    /// Whether the material was set by the user or not.
    bool m_customMaterial {false};

    /// Node in the scene graph
    ::Ogre::Entity* m_entity {nullptr};

    /// SMaterial attached to the mesh
    ::visuOgreAdaptor::SMaterial::sptr m_materialAdaptor {nullptr};

    /// Ogre Material related to the mesh
    ::fwData::Material::sptr m_material {nullptr};

    /// Attached Material's name
    std::string m_materialTemplateName {"Billboard_Default"};

    /// Attached texture adaptor UID
    std::string m_textureName {""};

    /// Is the entity visible or not ? We need to store it in the adaptor because the information may be received
    /// before the entity is created.
    bool m_isVisible {true};

    ::fwRenderOgre::Mesh::sptr m_meshGeometry {nullptr};

    /// Allows to scale the billboards
    float m_radius {1.f};

    /// Display the labelNumber
    bool m_displayLabel {false};

    /// Size of the character label
    float m_charHeight {0.03f};

    /// RGB Color for the labelPoint color
    ::fwData::Color::sptr m_labelColor {nullptr};

    /// Mask for picking requests
    std::uint32_t m_queryFlags {::Ogre::SceneManager::ENTITY_TYPE_MASK};

    /// Used to store label of each point.
    std::vector< ::fwRenderOgre::Text* > m_labels;

    /// Used to store label points nodes.
    std::vector< ::Ogre::SceneNode* > m_nodes;

    /// Scene node where all of our manual objects are attached
    ::Ogre::SceneNode* m_sceneNode {nullptr};
};

//------------------------------------------------------------------------------
// Inline functions

inline ::fwData::Material::sptr SPointList::getMaterial() const
{
    return m_material;
}

//------------------------------------------------------------------------------

inline void SPointList::setMaterial(::fwData::Material::sptr _material)
{
    m_material = _material;
}

//------------------------------------------------------------------------------

inline void SPointList::setMaterialTemplateName(const std::string& _materialName)
{
    m_materialTemplateName = _materialName;
}

//------------------------------------------------------------------------------

inline void SPointList::setAutoResetCamera(bool _autoResetCamera)
{
    m_autoResetCamera = _autoResetCamera;
}

//------------------------------------------------------------------------------

inline ::Ogre::Entity* SPointList::getEntity() const
{
    return m_entity;
}

//------------------------------------------------------------------------------

inline bool SPointList::getVisibility() const
{
    return m_entity ? m_entity->getVisible() : m_isVisible;
}

//------------------------------------------------------------------------------

} //namespace visuOgreAdaptor
