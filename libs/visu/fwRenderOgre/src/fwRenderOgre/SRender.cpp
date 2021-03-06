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

#include "fwRenderOgre/SRender.hpp"

#include "fwRenderOgre/IAdaptor.hpp"
#include "fwRenderOgre/registry/Adaptor.hpp"
#include "fwRenderOgre/Utils.hpp"

#include <fwCom/Signal.hxx>
#include <fwCom/Slots.hxx>

#define FW_PROFILING_DISABLED
#include <fwCore/Profiling.hpp>

#include <fwData/mt/ObjectWriteLock.hpp>

#include <fwRuntime/ConfigurationElementContainer.hpp>
#include <fwRuntime/utils/GenericExecutableFactoryRegistrar.hpp>

#include <fwServices/helper/Config.hpp>
#include <fwServices/macros.hpp>

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreNode.h>
#include <OGRE/Overlay/OgreOverlayContainer.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

#include <stack>

fwServicesRegisterMacro( ::fwRender::IRender, ::fwRenderOgre::SRender, ::fwData::Composite )

namespace fwRenderOgre
{

//-----------------------------------------------------------------------------

const std::string SRender::s_OGREBACKGROUNDID = "ogreBackground";

//-----------------------------------------------------------------------------

static const ::fwServices::IService::KeyType s_OFFSCREEN_INOUT = "offScreen";

//-----------------------------------------------------------------------------

const ::fwCom::Signals::SignalKeyType SRender::s_COMPOSITOR_UPDATED_SIG = "compositorUpdated";
const ::fwCom::Signals::SignalKeyType SRender::s_FULLSCREEN_SET_SIG     = "fullscreenSet";

//-----------------------------------------------------------------------------

const ::fwCom::Slots::SlotKeyType SRender::s_COMPUTE_CAMERA_ORIG_SLOT     = "computeCameraParameters";
const ::fwCom::Slots::SlotKeyType SRender::s_COMPUTE_CAMERA_CLIPPING_SLOT = "computeCameraClipping";
const ::fwCom::Slots::SlotKeyType SRender::s_REQUEST_RENDER_SLOT          = "requestRender";
const ::fwCom::Slots::SlotKeyType SRender::s_DISABLE_FULLSCREEN           = "disableFullscreen";
const ::fwCom::Slots::SlotKeyType SRender::s_ENABLE_FULLSCREEN            = "enableFullscreen";

static const ::fwCom::Slots::SlotKeyType s_ADD_OBJECTS_SLOT    = "addObject";
static const ::fwCom::Slots::SlotKeyType s_CHANGE_OBJECTS_SLOT = "changeObject";
static const ::fwCom::Slots::SlotKeyType s_REMOVE_OBJECTS_SLOT = "removeObjects";

//-----------------------------------------------------------------------------

SRender::SRender() noexcept
{
    m_ogreRoot = ::fwRenderOgre::Utils::getOgreRoot();

    newSignal<CompositorUpdatedSignalType>(s_COMPOSITOR_UPDATED_SIG);
    m_fullscreenSetSig = newSignal<FullscreenSetSignalType>(s_FULLSCREEN_SET_SIG);

    newSlot(s_COMPUTE_CAMERA_ORIG_SLOT, &SRender::resetCameraCoordinates, this);
    newSlot(s_COMPUTE_CAMERA_CLIPPING_SLOT, &SRender::computeCameraClipping, this);
    newSlot(s_REQUEST_RENDER_SLOT, &SRender::requestRender, this);
    newSlot(s_DISABLE_FULLSCREEN, &SRender::disableFullscreen, this);
    newSlot(s_ENABLE_FULLSCREEN, &SRender::enableFullscreen, this);
}

//-----------------------------------------------------------------------------

SRender::~SRender() noexcept
{
    m_ogreRoot = nullptr;
}

//-----------------------------------------------------------------------------

void SRender::configuring()
{
    const ConfigType config = this->getConfigTree();

    SLM_ERROR_IF("Only one scene must be configured.", config.count("scene") != 1);
    const ConfigType sceneCfg = config.get_child("scene");

    const size_t nbInouts = config.count("inout");
    SLM_ASSERT("This service accepts at most one inout.", nbInouts <= 1);

    if(nbInouts == 1)
    {
        const std::string key = config.get<std::string>("inout.<xmlattr>.key", "");
        m_offScreen = (key == s_OFFSCREEN_INOUT);

        SLM_ASSERT("'" + key + "' is not a valid key. Only '" + s_OFFSCREEN_INOUT +"' is accepted.", m_offScreen);

        m_width  = sceneCfg.get<unsigned int>("<xmlattr>.width", m_width);
        m_height = sceneCfg.get<unsigned int>("<xmlattr>.height", m_height);
        m_flip   = sceneCfg.get<bool>("<xmlattr>.flip", m_flip);

    }
    else // no offscreen rendering.
    {
        this->initialize();
    }

    m_fullscreen = sceneCfg.get<bool>("<xmlattr>.fullscreen", false);

#ifdef __APPLE__
    // TODO: fix fullscreen rendering on macOS.
    SLM_ERROR("Fullscreen is broken on macOS (as of macOS 10.14 and Qt 5.11.2 and Ogre 1.11.4, "
              "it is therefore disabled.");
    m_fullscreen = false;
#endif

    const std::string renderMode = sceneCfg.get<std::string>("<xmlattr>.renderMode", "auto");
    if (renderMode == "auto")
    {
        m_renderMode = RenderMode::AUTO;
    }
    else if (renderMode == "always")
    {
        m_renderMode = RenderMode::ALWAYS;
    }
    else if (renderMode == "sync")
    {
        m_renderMode = RenderMode::SYNC;
    }
    else
    {
        SLM_ERROR("Unknown rendering mode '" + renderMode + "'");
    }

    auto adaptorConfigs = sceneCfg.equal_range("adaptor");
    for( auto it = adaptorConfigs.first; it != adaptorConfigs.second; ++it )
    {
        const std::string uid = it->second.get<std::string>("<xmlattr>.uid");
        auto& registry        = ::fwRenderOgre::registry::getAdaptorRegistry();
        registry[uid] = this->getID();
    }
}

//-----------------------------------------------------------------------------

void SRender::starting()
{
    SLM_TRACE_FUNC();

    bool bHasBackground = false;

    if (!m_offScreen)
    {
        this->create();
    }
    const ConfigType config = this->getConfigTree();

    SLM_ERROR_IF("Only one scene must be configured.", config.count("scene") != 1);

    const ConfigType sceneCfg = config.get_child("scene");

    SLM_ERROR_IF("Overlays should be enabled at the layer level.", !sceneCfg.get("<xmlattr>.overlays", "").empty());

    auto layerConfigs = sceneCfg.equal_range("layer");
    for( auto it = layerConfigs.first; it != layerConfigs.second; ++it )
    {
        this->configureLayer(it->second);
    }
    auto bkgConfigs = sceneCfg.equal_range("background");
    for( auto it = bkgConfigs.first; it != bkgConfigs.second; ++it )
    {
        OSLM_ERROR_IF("A background has already been set, overriding it...", bHasBackground);
        try
        {
            this->configureBackgroundLayer(it->second);
        }
        catch (std::exception& e)
        {
            OSLM_ERROR("Error configuring background for layer '" + this->getID() + "': " + e.what());
        }

        bHasBackground = true;
    }

    if(!bHasBackground)
    {
        // Create a default black background
        ::fwRenderOgre::Layer::sptr ogreLayer = ::fwRenderOgre::Layer::New();
        ogreLayer->setRenderService(::fwRenderOgre::SRender::dynamicCast(this->shared_from_this()));
        ogreLayer->setID("backgroundLayer");
        ogreLayer->setOrder(0);
        ogreLayer->setWorker(m_associatedWorker);
        ogreLayer->setBackgroundColor("#000000", "#000000");
        ogreLayer->setBackgroundScale(0, 0.5);
        ogreLayer->setHasDefaultLight(false);
        ogreLayer->setViewportConfig({0.f, 0.f, 1.f, 1.f});

        m_layers[s_OGREBACKGROUNDID] = ogreLayer;
    }

    if(m_offScreen)
    {
        // Instantiate the manager that help to communicate between this service and the widget
        m_interactorManager = ::fwRenderOgre::IRenderWindowInteractorManager::createOffscreenManager(m_width, m_height);
        m_interactorManager->setRenderService(this->getSptr());
        m_interactorManager->createContainer( nullptr, m_renderMode != RenderMode::ALWAYS, m_fullscreen );
    }
    else
    {
        // Instantiate the manager that help to communicate between this service and the widget
        m_interactorManager = ::fwRenderOgre::IRenderWindowInteractorManager::createManager();
        m_interactorManager->setRenderService(this->getSptr());
        m_interactorManager->createContainer( this->getContainer(), m_renderMode != RenderMode::ALWAYS, m_fullscreen );
    }

    // Initialize resources to load overlay scripts.
    ::Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    m_interactorManager->getRenderTarget()->addListener(&m_viewportListener);

    for (auto it : m_layers)
    {
        ::fwRenderOgre::Layer::sptr layer = it.second;
        layer->setRenderTarget(m_interactorManager->getRenderTarget());
        layer->createScene();

        auto* vp = layer->getViewport();
        m_viewportOverlaysMap.emplace(vp, layer->getEnabledOverlays());
    }

    // Everything is started now, we can safely create connections and thus receive interactions from the widget
    m_interactorManager->connectToContainer();
}

//-----------------------------------------------------------------------------

void SRender::stopping()
{
    this->makeCurrent();

    m_interactorManager->getRenderTarget()->removeAllListeners();
    m_viewportOverlaysMap.clear();

    for (auto it : m_layers)
    {
        ::fwRenderOgre::Layer::sptr layer = it.second;
        layer->destroyScene();
    }
    m_layers.clear();

    m_interactorManager->disconnectInteractor();
    m_interactorManager.reset();

    if(!m_offScreen)
    {
        this->destroy();
    }
}

//-----------------------------------------------------------------------------

void SRender::updating()
{
}

//-----------------------------------------------------------------------------

void SRender::makeCurrent()
{
    m_interactorManager->makeCurrent();
}

//-----------------------------------------------------------------------------

void SRender::configureLayer(const ConfigType& _cfg )
{
    const ConfigType attributes             = _cfg.get_child("<xmlattr>");
    const std::string id                    = attributes.get<std::string>("id", "");
    const std::string compositors           = attributes.get<std::string>("compositors", "");
    const std::string transparencyTechnique = attributes.get<std::string>("transparency", "");
    const std::string numPeels              = attributes.get<std::string>("numPeels", "");
    const std::string stereoMode            = attributes.get<std::string>("stereoMode", "");
    const std::string defaultLight          = attributes.get<std::string>("defaultLight", "");
    const auto viewportConfig               = configureLayerViewport(_cfg);

    auto overlays = std::istringstream(attributes.get<std::string>("overlays", ""));
    std::vector< std::string > enabledOverlays;
    for(std::string overlayName; std::getline(overlays, overlayName, ';'); )
    {
        enabledOverlays.push_back(overlayName);
    }

    SLM_ASSERT( "'id' required attribute missing or empty", !id.empty() );
    SLM_ASSERT( "Unknown 3D mode : " << stereoMode,
                stereoMode.empty() || stereoMode == "no" || stereoMode == "AutoStereo5" || stereoMode == "AutoStereo8" ||
                stereoMode == "Stereo");

    int layerOrder        = 0;
    const auto layerDepth = attributes.get_optional<int>("depth");
    if(layerDepth)
    {
        FW_DEPRECATED_MSG("Attribute 'depth' is deprecated, please used 'order' instead", "22.0")
        layerOrder = layerDepth.get();
    }
    else
    {
        layerOrder = attributes.get<int>("order");
    }
    SLM_ASSERT("Attribute 'order' must be greater than 0", layerOrder > 0);

    ::fwRenderOgre::Layer::sptr ogreLayer = ::fwRenderOgre::Layer::New();
    compositor::Core::StereoModeType layerStereoMode =
        stereoMode == "AutoStereo5" ? compositor::Core::StereoModeType::AUTOSTEREO_5 :
        stereoMode == "AutoStereo8" ? compositor::Core::StereoModeType::AUTOSTEREO_8 :
        stereoMode == "Stereo"      ? compositor::Core::StereoModeType::STEREO :
        compositor::Core::StereoModeType::NONE;

    ogreLayer->setRenderService(::fwRenderOgre::SRender::dynamicCast(this->shared_from_this()));
    ogreLayer->setID(id);
    ogreLayer->setOrder(layerOrder);
    ogreLayer->setWorker(m_associatedWorker);
    ogreLayer->setCoreCompositorEnabled(true, transparencyTechnique, numPeels, layerStereoMode);
    ogreLayer->setCompositorChainEnabled(compositors);
    ogreLayer->setViewportConfig(viewportConfig);
    ogreLayer->setEnabledOverlays(enabledOverlays);

    if(!defaultLight.empty() && defaultLight == "no")
    {
        ogreLayer->setHasDefaultLight(false);
    }

    // Finally, the layer is pushed in the map
    m_layers[id] = ogreLayer;
}

//-----------------------------------------------------------------------------

void SRender::configureBackgroundLayer(const ConfigType& _cfg )
{
    SLM_ASSERT( "'id' required attribute missing or empty", !this->getID().empty() );
    const ConfigType attributes = _cfg.get_child("<xmlattr>");

    ::fwRenderOgre::Layer::sptr ogreLayer = ::fwRenderOgre::Layer::New();
    ogreLayer->setRenderService(::fwRenderOgre::SRender::dynamicCast(this->shared_from_this()));
    ogreLayer->setID(s_OGREBACKGROUNDID);
    ogreLayer->setOrder(0);
    ogreLayer->setWorker(m_associatedWorker);
    ogreLayer->setHasDefaultLight(false);

    if (attributes.count("topColor") && attributes.count("bottomColor"))
    {
        std::string topColor = attributes.get<std::string>("topColor");
        std::string botColor = attributes.get<std::string>("bottomColor");

        ogreLayer->setBackgroundColor(topColor, botColor);
    }

    if (attributes.count("topScale") && attributes.count("bottomScale"))
    {
        const float topScaleVal = attributes.get<float>("topScale");
        const float botScaleVal = attributes.get<float>("bottomScale");

        ogreLayer->setBackgroundScale(topScaleVal, botScaleVal);
    }

    m_layers[s_OGREBACKGROUNDID] = ogreLayer;
}

//-----------------------------------------------------------------------------

Layer::ViewportConfigType
SRender::configureLayerViewport(const ::fwServices::IService::ConfigType& _cfg)
{
    Layer::ViewportConfigType cfgType { 0.f, 0.f, 1.f, 1.f };
    const auto _vpConfig = _cfg.get_child_optional("viewport.<xmlattr>");
    if(_vpConfig.has_value())
    {
        const auto cfg = _vpConfig.value();

        float xPos = cfg.get<float>("hOffset", 0.f);
        float yPos = cfg.get<float>("vOffset", 0.f);

        const float width  = cfg.get<float>("width");
        const float height = cfg.get<float>("height");

        const std::map< std::string, float > horizAlignToX {
            { "left", xPos },
            { "center", 0.5f - width * 0.5f + xPos},
            { "right", 1.f - width - xPos }
        };

        const std::map< std::string, float > vertAlignToY {
            { "bottom", 1.f -height - yPos },
            { "center", 0.5f - height * 0.5f + yPos },
            { "top", yPos }
        };

        const std::string hAlign = cfg.get("hAlign", "left");
        const std::string vAlign = cfg.get("vAlign", "top");

        xPos = horizAlignToX.at(hAlign);
        yPos = vertAlignToY.at(vAlign);

        cfgType = std::tie(xPos, yPos, width, height);
    }

    return cfgType;
}

//-----------------------------------------------------------------------------

void SRender::requestRender()
{
    if ( m_renderMode == RenderMode::SYNC )
    {
        m_interactorManager->renderNow();
    }
    else
    {
        m_interactorManager->requestRender();
    }

    if(m_offScreen)
    {
        FW_PROFILE("Offscreen rendering");

        ::fwData::Image::sptr image = this->getInOut< ::fwData::Image >(s_OFFSCREEN_INOUT);
        SLM_ASSERT("Offscreen image not found.", image);

        {
            ::fwData::mt::ObjectWriteLock lock(image);

            this->makeCurrent();
            ::Ogre::TexturePtr renderTexture = m_interactorManager->getRenderTexture();
            SLM_ASSERT("The offscreen window doesn't write to a texture", renderTexture);
            ::fwRenderOgre::Utils::convertFromOgreTexture(renderTexture, image, m_flip);
        }

        auto sig = image->signal< ::fwData::Object::ModifiedSignalType >(::fwData::Object::s_MODIFIED_SIG);
        sig->asyncEmit();
    }
}

//-----------------------------------------------------------------------------

void SRender::resetCameraCoordinates(const std::string& _layerId)
{
    auto layer = m_layers.find(_layerId);

    if(layer != m_layers.end())
    {
        layer->second->resetCameraCoordinates();
    }

    this->requestRender();
}

//-----------------------------------------------------------------------------

void SRender::computeCameraClipping()
{
    for (auto it : m_layers)
    {
        ::fwRenderOgre::Layer::sptr layer = it.second;
        layer->resetCameraClippingRange();
    }
}

//-----------------------------------------------------------------------------

void SRender::render()
{
    this->requestRender();
}

//-----------------------------------------------------------------------------

bool SRender::isShownOnScreen()
{
    return m_offScreen || m_fullscreen || this->getContainer()->isShownOnScreen();
}

// ----------------------------------------------------------------------------

::Ogre::SceneManager* SRender::getSceneManager(const ::std::string& sceneID)
{
    ::fwRenderOgre::Layer::sptr layer = this->getLayer(sceneID);
    return layer->getSceneManager();
}

// ----------------------------------------------------------------------------

::fwRenderOgre::Layer::sptr SRender::getLayer(const ::std::string& sceneID)
{
    OSLM_ASSERT("Empty sceneID", !sceneID.empty());
    OSLM_ASSERT("Layer ID "<< sceneID <<" does not exist", m_layers.find(sceneID) != m_layers.end());

    ::fwRenderOgre::Layer::sptr layer = m_layers.at(sceneID);

    return layer;
}

// ----------------------------------------------------------------------------

::fwRenderOgre::SRender::LayerMapType SRender::getLayers()
{
    return m_layers;
}

// ----------------------------------------------------------------------------

::fwRenderOgre::IRenderWindowInteractorManager::sptr SRender::getInteractorManager() const
{
    return m_interactorManager;
}

// ----------------------------------------------------------------------------

void SRender::disableFullscreen()
{
    m_fullscreen = false;
    m_interactorManager->setFullscreen(m_fullscreen, -1);

    m_fullscreenSetSig->asyncEmit(false);
}

// ----------------------------------------------------------------------------

void SRender::enableFullscreen(int _screen)
{
    m_fullscreen = true;
    m_interactorManager->setFullscreen(m_fullscreen, _screen);

    m_fullscreenSetSig->asyncEmit(true);
}

// ----------------------------------------------------------------------------

} //namespace fwRenderOgre
