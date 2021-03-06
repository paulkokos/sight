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

#include <fwData/PointList.hpp>

#include <fwRenderOgre/IAdaptor.hpp>
#include <fwRenderOgre/TransferFunction.hpp>

#include <OGRE/OgreTexture.h>

namespace visuOgreAdaptor
{
/**
 * @brief This adaptor renders a video frame from a 2D-image.
 *
 * @section Slots Slots
 * - \b updateVisibility(bool): sets whether the video is visible or not.
 * - \b toggleVisibility(): toggle whether the video is visible or not.
 * - \b show(): shows the video.
 * - \b hide(): hides the video.
 * - \b updateTF(): Updates the displayed transfer function.
 * - \b updatePL(): Updates the displayed point list.
 *
 * @section XML XML Configuration
 * @code{.xml}
    <service type="::visuOgreAdaptor::SVideo" autoConnect="yes" >
        <in key="image" uid="..." />
        <in key="tf" uid="..." optional="yes" />
        <config layer="default" />
    </service>
   @endcode
 *
 * @subsection Input Input:
 * - \b image [::fwData::Image]: frame displayed.
 * - \b tf [::fwData::TransferFunction] (optional): a transfer function that can be applied to the video.
 * - \b pointList [::fwData::PointList] (optional): point list to display (coordinates must be in the image space).
 *
 * @subsection Configuration Configuration:
 * - \b layer (mandatory, string): defines the video's layer
 * - \b visible (optional, bool, default=true): the visibility of the adaptor.
 * - \b materialTemplate (optional, string, default='Billboard_Default'): the name of the base Ogre material for the
 *      internally created SMaterial.
 * - \b textureName (optional, string, default=""): the name of the Ogre texture the mesh will use.
 * - \b radius (optional, float, default=1.f): billboard radius.
 * - \b displayLabel (optional, bool, default=false): display the label points (default = false).
 * - \b labelColor (optional, hexadecimal, default=0xFFFFFF): color of the label in hexadecimal.
 * - \b color (optional, hexadecimal, default=#FFFFFFFF): color of the texture in hexadecimal.
 * - \b fixedSize (optional, bool, default=false): if true, the billboard will have a fixed size in screen space.
 * - \b queryFlags (optional, uint32, default=0x40000000): Picking flags. Points can be picked by pickers with a
 *      matching mask.
 * - \b fontSource (optional, string, default=DejaVuSans.ttf): TrueType font (*.ttf) source file.
 * - \b fontSize (optional, unsigned int, default=16): font size in points.
 */
class VISUOGREADAPTOR_CLASS_API SVideo final : public ::fwRenderOgre::IAdaptor
{

public:

    /// Generates default methods as New, dynamicCast, ...
    fwCoreServiceMacro(SVideo, ::fwRenderOgre::IAdaptor)

    /// Creates the adaptor an initialize slots.
    VISUOGREADAPTOR_API SVideo() noexcept;

    /// Does nothing.
    VISUOGREADAPTOR_API ~SVideo() noexcept override;

protected:

    /// Configures the adaptor.
    VISUOGREADAPTOR_API void configuring() override;

    /// Creates the Ogre texture and mapper used to show the video frame.
    VISUOGREADAPTOR_API void starting() override;

    /**
     * @brief Proposals to connect service slots to associated object signals.
     * @return A map of each proposed connection.
     *
     * Connect ::fwData::Image::s_MODIFIED_SIG of s_IMAGE_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::Image::s_BUFFER_MODIFIED_SIG of s_IMAGE_INPUT to s_UPDATE_SLOT
     * Connect ::fwData::TransferFunction::s_MODIFIED_SIG of s_TF_INPUT to :s_UPDATE_TF_SLOT
     * Connect ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG of s_TF_INPUT to s_UPDATE_TF_SLOT
     * Connect ::fwData::TransferFunction::s_WINDOWING_MODIFIED_SIG of s_TF_INPUT to s_UPDATE_TF_SLOT
     */
    VISUOGREADAPTOR_API ::fwServices::IService::KeyConnectionsMap getAutoConnections() const override;

    /// Updates the frame from the current Image.
    VISUOGREADAPTOR_API void updating() override;

    /// Removes the actor from the renderer
    VISUOGREADAPTOR_API void stopping() override;

    /**
     * @brief Sets the video visibility.
     * @param _visible the visibility status of the video.
     */
    VISUOGREADAPTOR_API void setVisible(bool _visible) override;

private:

    /// SLOTS: updates the displayed transfer function.
    void updateTF();

    /// SLOTS: updates 2D coordinates of the pointlist from image space (bottom left) to video space (center center).
    void updatePL();

    /// Cleans scene entities and associated mesh.
    void clearEntity();

    /// Contains the Ogre texture used to store the image.
    ::Ogre::TexturePtr m_texture;

    /// Contains the Ogre material used to display the video plane.
    ::Ogre::MaterialPtr m_material;

    /// Contains the Ogre mesh used to display the video plane.
    ::Ogre::MeshPtr m_mesh;

    /// Contains the node where the plane is attached.
    ::Ogre::SceneNode* m_sceneNode { nullptr };

    /// Contains the Ogre entity used to display the video plane.
    ::Ogre::Entity* m_entity { nullptr };

    /// Contains and manages the Ogre textures used to store the transfer function (GPU point of view).
    std::unique_ptr< ::fwRenderOgre::TransferFunction> m_gpuTF;

    /// Defines if the texture is initialized.
    bool m_isTextureInit { false };

    /// Defines the previous image size.
    size_t m_previousWidth { 0 };

    /// Defines the previous image spacing.
    size_t m_previousHeight { 0 };

    /// Defines the previous image type.
    ::fwTools::Type m_previousType;

    /// Contains the pointlist in the video space coordinates.
    ::fwData::PointList::sptr m_pointList { nullptr };

    /// Contains the adaptor used to display the 2D pointlist.
    ::fwRenderOgre::IAdaptor::sptr m_pointListAdaptor { nullptr };

    /// Defines material's name used by the point list.
    std::string m_materialTemplateName { "" };

    /// Defines the attached texture adaptor UID used by the point list.
    std::string m_textureName { "" };

    /// Defines the billboards radius.
    std::string m_radius { "" };

    /// Defines if label numbers are displayed.
    std::string m_displayLabel { "" };

    /// Defines the RGB color for the label point color.
    std::string m_labelColor { "" };

    /// Defines the color of points.
    std::string m_color { "" };

    /// Defines if billboard will have a fixed size in screen space.
    std::string m_fixedSize { "" };

    /// Defines the mask for picking requests in the point list.
    std::string m_queryFlags { "" };

    /// Defines the TrueType font source file.
    std::string m_fontSource { "" };

    /// Defines the font size in points.
    std::string m_fontSize { "" };

};

} // namespace visuOgreAdaptor.
