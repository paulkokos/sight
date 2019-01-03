/************************************************************************
 *
 * Copyright (C) 2014-2018 IRCAD France
 * Copyright (C) 2014-2018 IHU Strasbourg
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
        <config renderer="rendererId" transform="transformUID" materialAdaptor="materialName" shadingMode="gouraud"
                textureName="texAdaptorUID" radius="2.4" displayLabel="true" charHeight="0.02" labelColor="#0000ff" />
    </service>
   @endcode
 * @subsection In-Out In-Out
 * - \b pointList [::fwData::PointList] (optional): point list to display.
 * - \b mesh [::fwData::Mesh] (optional): point based mesh to display. If the mesh contains any topology, it will be
 * ignored and only raw vertices will be displayed.
 * or add some fields.
 * @subsection Configuration Configuration:
 *  - \b renderer (mandatory) : defines the mesh's layer
 *  - \b autoresetcamera (optional, default="yes"): reset the camera when this mesh is modified, "yes" or "no".
 *  - \b transform (optional) : the name of the Ogre transform node where to attach the mesh, as it was specified
 * in the STransform adaptor.
 * Either of the following (whether a material is configured in the XML scene or not) :
 *  - \b materialName (optional) : name of the Ogre material, as defined in the ::visuOgreAdaptor::SMaterial you want
 * to be bound to.
 * Only if there is no material configured in the XML scene (in this case, it has to retrieve the material
 * template, the texture adaptor and the shading mode) :
 *  - \b materialTemplate (optional) : the name of the base Ogre material for the internally created SMaterial.
 *  - \b textureName (optional) : the name of the Ogre texture that the mesh will use.
 *  - \b radius (optional) : billboard radius.
 *  - \b displayLabel (optional) : display the label points (default = false)
 *  - \b charHeight (optional): size of the character label (default = 0.03)
 *  - \b labelColor (optional): color of the label in hexadecimal (default = white)
 *  - \b fixedSize (optional, default="false") : if true, the billboard will have a fixed size in screen space.
 *  - \b queryFlags (optional) : Used for picking. Picked only by pickers with the same flag.
 */
class VISUOGREADAPTOR_CLASS_API SPointList : public ::fwRenderOgre::IAdaptor,
                                             public ::fwRenderOgre::ITransformable
{
public:
    fwCoreServiceClassDefinitionsMacro((SPointList)(::fwRenderOgre::IAdaptor))

    /// Constructor: Sets default parameters and initializes necessary members.
    VISUOGREADAPTOR_API SPointList() noexcept;
    /// Destructor: if an entity exists in the Ogre Scene, asks Ogre to destroy it.
    VISUOGREADAPTOR_API virtual ~SPointList() noexcept;

    /// Returns the material associated to this.
    VISUOGREADAPTOR_API SPTR(::fwData::Material) getMaterial() const;
    /// Sets the current material.
    VISUOGREADAPTOR_API void setMaterial(SPTR(::fwData::Material) material);
    /// Sets the material template Name.
    VISUOGREADAPTOR_API void setMaterialTemplateName(const std::string& materialName);

    /// Active/Inactive automatic reset on camera. By default =true.
    VISUOGREADAPTOR_API void setAutoResetCamera(bool autoResetCamera);

    /// Returns associated entity
    VISUOGREADAPTOR_API ::Ogre::Entity* getEntity() const;

    /// Returns if the SPointList is visible in the scene or not.
    VISUOGREADAPTOR_API bool getVisibility() const;
    /// Sets whether the mesh is to be seen or not.
    VISUOGREADAPTOR_API void updateVisibility(bool isVisible);

    /// Returns proposals to connect service slots to associated object signals
    VISUOGREADAPTOR_API ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Ask the render service (SRender) to update - we also flag the r2vb objects as dirty
    VISUOGREADAPTOR_API virtual void requestRender() override;

private:

    /// Configures the adaptor
    void configuring() override;
    /// Manually creates a Mesh in the Default Ogre Ressource group
    void starting() override;
    /// Deletes the mesh after unregistering the service, and shutting connections.
    void stopping() override;
    /// Called when the mesh is modified
    void updating() override;

    /// Updates the mesh from a points list, checks if color, number of vertices have changed, and updates them.
    void updateMesh(const fwData::PointList::csptr& _pointList);

    /// Updates the mesh from a data mesh, checks if color, number of vertices have changed, and updates them.
    void updateMesh(const fwData::Mesh::csptr& _mesh);

    /// Instantiates a new material adaptor
    ::visuOgreAdaptor::SMaterial::sptr createMaterialService(const std::string& _materialSuffix = "");
    /// Associates a new SMaterial to the managed SPointList.
    /// With this method, SPointList is responsible for creating a SMaterial
    void updateMaterialAdaptor();

    /// Attach a node in the scene graph
    void attachNode(::Ogre::MovableObject* _node);

    /// Detach and destroy m_entity in the scene graph
    void detachAndDestroyEntity();

    /// Create all the labels and attach them to the sceneNode vector
    void createLabel(const ::fwData::PointList::csptr& _pointList);

    /// Destroy all the labels and delete them from the sceneNode vector
    void destroyLabel();

    /// Sets whether the camera must be auto reset when a mesh is updated or not.
    bool m_autoResetCamera;

    /// Node in the scene graph
    ::Ogre::Entity* m_entity;

    /// SMaterial attached to the mesh
    ::visuOgreAdaptor::SMaterial::sptr m_materialAdaptor;
    /// Ogre Material related to the mesh
    ::fwData::Material::sptr m_material;
    /// Attached Material's name
    std::string m_materialTemplateName;

    /// Attached texture adaptor UID
    std::string m_textureName;

    /// Is the entity visible or not ? We need to store it in the adaptor because the information may be received
    /// before the entity is created.
    bool m_isVisible;

    ::fwRenderOgre::Mesh::sptr m_meshGeometry;

    /// Allows to scale the billboards
    float m_radius { 1.f };

    /// Display the labelNumber
    bool m_displayLabel {false};

    /// Size of the character label
    float m_charHeight {0.03f};

    /// Remove last label when removing a point
    bool m_removeLastLabel {false};

    /// RGB Color for the labelPoint color
    ::fwData::Color::sptr m_labelColor;

    /// Mask for picking requests
    std::uint32_t m_queryFlags {0};

    /// Used to store label of each point.
    std::vector< ::fwRenderOgre::Text* > m_labels;

    /// Used to store label points nodes.
    std::vector< ::Ogre::SceneNode* > m_nodes;

    /// Scene node where all of our manual objects are attached
    ::Ogre::SceneNode* m_sceneNode { nullptr };
};

//------------------------------------------------------------------------------
// Inline functions

inline SPTR(::fwData::Material) SPointList::getMaterial() const
{
    return m_material;
}

//------------------------------------------------------------------------------

inline void SPointList::setMaterial(::fwData::Material::sptr material)
{
    m_material = material;
}

//------------------------------------------------------------------------------

inline void SPointList::setMaterialTemplateName(const std::string& materialName)
{
    m_materialTemplateName = materialName;
}

//------------------------------------------------------------------------------

inline void SPointList::setAutoResetCamera(bool autoResetCamera)
{
    m_autoResetCamera = autoResetCamera;
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
