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

#include "fwRenderOgre/interactor/TrackballInteractor.hpp"
#include "fwRenderOgre/picker/IPicker.hpp"
#include "fwRenderOgre/widget/ClippingBox.hpp"

#include <fwCom/Signal.hpp>
#include <fwCom/Signals.hpp>

#include <OGRE/OgreMovableObject.h>

namespace fwRenderOgre
{

class Layer;

namespace interactor
{

/**
 * @brief Picks VR widgets and updates clipping cube.
 *
 * @deprecated removed in sight 21.0
 */
class FWRENDEROGRE_CLASS_API VRWidgetsInteractor : public ::fwRenderOgre::interactor::TrackballInteractor
{
public:

    /// Constructor.
    [[deprecated("will be removed in sight 21.0")]]
    FWRENDEROGRE_API VRWidgetsInteractor(SPTR(Layer)_layer = nullptr) noexcept;

    /// Destructor.
    FWRENDEROGRE_API virtual ~VRWidgetsInteractor() noexcept;

    /// Interacts with the widget if it was previously picked, behaves like a trackball otherwise.
    FWRENDEROGRE_API virtual void mouseMoveEvent(MouseButton, Modifier, int, int, int, int) override;

    /// Ends all interactions with the widget.
    FWRENDEROGRE_API virtual void buttonReleaseEvent(MouseButton, Modifier, int, int) override;

    /// Picks the object at the (x,y) position on a left click, scales or translates the widget otherwise.
    FWRENDEROGRE_API virtual void buttonPressEvent(MouseButton, Modifier, int, int) override;

    /// Sets the widget handled by this interactor.
    FWRENDEROGRE_API void setWidget(widget::ClippingBox::sptr widget);

    /// Initializes the picker.
    FWRENDEROGRE_API void initPicker();

private:

    /// Currently selected widget.
    ::Ogre::MovableObject* m_pickedObject { nullptr };

    /// The widget with whom we interact.
    widget::ClippingBox::wptr m_widget;

    /// The picker used by this interactor.
    fwRenderOgre::picker::IPicker m_picker;

    /// Picks the object at screen coordinates (x, y)
    Ogre::MovableObject* pickObject(int x, int y);

};
} // namespace interactor
} // namespace fwRenderOgre
