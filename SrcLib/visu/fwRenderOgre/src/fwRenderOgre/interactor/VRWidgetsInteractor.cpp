/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2018.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "fwRenderOgre/interactor/VRWidgetsInteractor.hpp"

#include <fwRenderOgre/Layer.hpp>
#include <fwRenderOgre/registry/macros.hpp>

#include <fwCom/Signal.hxx>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreViewport.h>

fwRenderOgreRegisterInteractorMacro( ::fwRenderOgre::interactor::VRWidgetsInteractor );

namespace fwRenderOgre
{

namespace interactor
{

//------------------------------------------------------------------------------

VRWidgetsInteractor::VRWidgetsInteractor() noexcept :
    m_pickedObject(nullptr)
{
}

//------------------------------------------------------------------------------

VRWidgetsInteractor::~VRWidgetsInteractor() noexcept
{
}

//------------------------------------------------------------------------------

Ogre::MovableObject* VRWidgetsInteractor::pickObject(int x, int y)
{
    ::Ogre::Camera* camera = m_sceneManager->getCamera(::fwRenderOgre::Layer::DEFAULT_CAMERA_NAME);

    const int height = camera->getViewport()->getActualHeight();
    const int width  = camera->getViewport()->getActualWidth();

    const bool pickSuccess = m_picker.executeRaySceneQuery( x, y, width, height, 0 );

    return pickSuccess ? m_picker.getSelectedObject() : nullptr;
}

//------------------------------------------------------------------------------

void VRWidgetsInteractor::mouseMoveEvent(MouseButton button, int x, int y, int dx, int dy)
{
    if(m_widget) // If a widget is present in the scene.
    {
        if(button == LEFT)
        {
            if(m_pickedObject == nullptr) // Enable trackball if no face was picked.
            {
                TrackballInteractor::mouseMoveEvent(button, x, y, dx, dy);
            }
            else
            {
                m_widget->widgetPicked(m_pickedObject, x, y);
            }
        }
        else if(button == MIDDLE)
        {
            m_widget->moveClippingBox(x, y, -dx, -dy);
        }
        else if(button == RIGHT)
        {
            m_widget->scaleClippingBox(x, y, dy);
        }
    }
    else if(button == LEFT) // Fallback to trackball otherwise.
    {
        TrackballInteractor::mouseMoveEvent(button, x, y, dx, dy);
    }
}

//------------------------------------------------------------------------------

void VRWidgetsInteractor::buttonReleaseEvent(MouseButton /*button*/, int /*x*/, int /*y*/)
{
    if(m_widget)
    {
        m_widget->widgetReleased();
        m_pickedObject = nullptr;
    }
}

//------------------------------------------------------------------------------

void VRWidgetsInteractor::buttonPressEvent(MouseButton button, int x, int y)
{
    if(m_widget)
    {
        if(button == LEFT && m_widget->getVisibility())
        {
            m_pickedObject = pickObject(x, y);

            if(m_widget->belongsToWidget(m_pickedObject))
            {
                m_widget->widgetPicked(m_pickedObject, x, y);
            }
            else
            {
                m_pickedObject = nullptr;
            }
        }
        else if(button == MIDDLE)
        {
            m_widget->moveClippingBox(x, y, 0, 0);
        }
        else if(button == RIGHT)
        {
            m_widget->scaleClippingBox(x, y, 0);
        }
    }
}

//------------------------------------------------------------------------------

void VRWidgetsInteractor::setWidget(ui::VRWidget* widget)
{
    m_widget = widget;
}

//------------------------------------------------------------------------------

void VRWidgetsInteractor::initPicker()
{
    m_picker.setSceneManager(this->m_sceneManager);
}

} // namespace interactor
} // namespace fwRenderOgre
