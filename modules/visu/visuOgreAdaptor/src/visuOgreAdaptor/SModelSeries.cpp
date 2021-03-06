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

#include "visuOgreAdaptor/SModelSeries.hpp"

#include "visuOgreAdaptor/SMesh.hpp"
#include "visuOgreAdaptor/SReconstruction.hpp"

#include <fwCom/Signal.hxx>
#include <fwCom/Slots.hxx>

#include <fwData/Boolean.hpp>
#include <fwData/Material.hpp>
#include <fwData/Mesh.hpp>
#include <fwData/mt/ObjectReadLock.hpp>
#include <fwData/Reconstruction.hpp>
#include <fwData/TransformationMatrix3D.hpp>

#include <fwMedData/ModelSeries.hpp>

#include <fwServices/macros.hpp>
#include <fwServices/op/Add.hpp>

namespace visuOgreAdaptor
{
//-----------------------------------------------------------------------------

static const ::fwCom::Slots::SlotKeyType s_CHANGE_FIELD_SLOT = "changeField";

static const std::string s_MODEL_INPUT = "model";

static const std::string s_AUTORESET_CAMERA_CONFIG = "autoresetcamera";
static const std::string s_MATERIAL_CONFIG         = "material";
static const std::string s_DYNAMIC_CONFIG          = "dynamic";
static const std::string s_DYNAMIC_VERTICES_CONFIG = "dynamicVertices";
static const std::string s_QUERY_CONFIG            = "queryFlags";

//------------------------------------------------------------------------------

SModelSeries::SModelSeries() noexcept
{
    newSlot(s_CHANGE_FIELD_SLOT, &SModelSeries::showReconstructionsOnFieldChanged, this);
    newSlot("showReconstructions", &SModelSeries::showReconstructionsDeprecatedSlot, this);
}

//------------------------------------------------------------------------------

SModelSeries::~SModelSeries() noexcept
{
}

//------------------------------------------------------------------------------

void SModelSeries::configuring()
{
    this->configureParams();

    const ConfigType configType = this->getConfigTree();
    const ConfigType config     = configType.get_child("config.<xmlattr>");

    this->setTransformId(config.get<std::string>( ::fwRenderOgre::ITransformable::s_TRANSFORM_CONFIG,
                                                  this->getID() + "_transform"));

    m_autoResetCamera = config.get<std::string>(s_AUTORESET_CAMERA_CONFIG, "yes") == "yes";

    m_materialTemplateName = config.get<std::string>(s_MATERIAL_CONFIG, m_materialTemplateName);
    m_isDynamic            = config.get<bool>(s_DYNAMIC_CONFIG, m_isDynamic);
    m_isDynamicVertices    = config.get<bool>(s_DYNAMIC_VERTICES_CONFIG, m_isDynamicVertices);

    if(config.count(s_QUERY_CONFIG))
    {
        const std::string hexaMask = config.get<std::string>(s_QUERY_CONFIG);
        SLM_ASSERT(
            "Hexadecimal values should start with '0x'"
            "Given value : " + hexaMask,
            hexaMask.length() > 2 &&
            hexaMask.substr(0, 2) == "0x");
        m_queryFlags = static_cast< std::uint32_t >(std::stoul(hexaMask, nullptr, 16));
    }
}

//------------------------------------------------------------------------------

void SModelSeries::starting()
{
    this->initialize();

    this->updating();
}

//-----------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsMap SModelSeries::getAutoConnections() const
{
    ::fwServices::IService::KeyConnectionsMap connections;
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_MODIFIED_SIG, s_UPDATE_SLOT);
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_RECONSTRUCTIONS_ADDED_SIG, s_UPDATE_SLOT);
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_RECONSTRUCTIONS_REMOVED_SIG, s_UPDATE_SLOT);
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_ADDED_FIELDS_SIG, s_CHANGE_FIELD_SLOT);
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_REMOVED_FIELDS_SIG, s_CHANGE_FIELD_SLOT);
    connections.push(s_MODEL_INPUT, ::fwMedData::ModelSeries::s_CHANGED_FIELDS_SIG, s_CHANGE_FIELD_SLOT);
    return connections;
}

//------------------------------------------------------------------------------

void SModelSeries::updating()
{
    // Retrieves the associated Sight ModelSeries object
    const auto modelSeries = this->getInput< ::fwMedData::ModelSeries >(s_MODEL_INPUT);
    SLM_ASSERT("input '" + s_MODEL_INPUT + "' does not exist.", modelSeries);

    this->stopping();

    ::fwData::mt::ObjectReadLock lock(modelSeries);

    // showRec indicates if we have to show the associated reconstructions or not
    const bool showRec = modelSeries->getField("ShowReconstructions", ::fwData::Boolean::New(true))->value();

    for(auto reconstruction : modelSeries->getReconstructionDB())
    {
        auto adaptor = this->registerService< ::visuOgreAdaptor::SReconstruction>("::visuOgreAdaptor::SReconstruction");
        adaptor->registerInput(reconstruction, "reconstruction", true);

        // We use the default service ID to get a unique number because a ModelSeries contains several Reconstructions
        adaptor->setID(this->getID() + "_" + adaptor->getID());

        adaptor->setRenderService(this->getRenderService());
        adaptor->setLayerID(m_layerID);
        adaptor->setTransformId(this->getTransformId());
        adaptor->setMaterialTemplateName(m_materialTemplateName);
        adaptor->setAutoResetCamera(m_autoResetCamera);
        adaptor->setQueryFlags(m_queryFlags);

        adaptor->start();
        adaptor->updateVisibility(!showRec);

        ::visuOgreAdaptor::SMesh::sptr meshAdaptor = adaptor->getMeshAdaptor();
        meshAdaptor->setDynamic(m_isDynamic);
        meshAdaptor->setDynamicVertices(m_isDynamicVertices);
    }
}

//------------------------------------------------------------------------------

void SModelSeries::stopping()
{
    this->unregisterServices();
}

//------------------------------------------------------------------------------

void SModelSeries::setVisible(bool _visible)
{
    auto adaptors = this->getRegisteredServices();
    for(auto adaptor : adaptors)
    {
        auto recAdaptor = ::visuOgreAdaptor::SReconstruction::dynamicCast(adaptor.lock());
        recAdaptor->updateVisibility(!_visible);
    }
}

//------------------------------------------------------------------------------

void SModelSeries::showReconstructionsDeprecatedSlot(bool _show)
{
    FW_DEPRECATED_MSG("::visuOgreAdaptor::SModelSeries::showReconstructions is no longer supported", "21.0")
    this->setVisible(_show);
}

//------------------------------------------------------------------------------

void SModelSeries::showReconstructionsOnFieldChanged()
{
    const auto modelSeries = this->getInput< ::fwMedData::ModelSeries >(s_MODEL_INPUT);
    SLM_ASSERT("input '" + s_MODEL_INPUT + "' does not exist.", modelSeries);

    ::fwData::mt::ObjectReadLock lock(modelSeries);

    const bool showRec = modelSeries->getField("ShowReconstructions", ::fwData::Boolean::New(true))->value();

    auto adaptors = this->getRegisteredServices();
    for(auto adaptor : adaptors)
    {
        auto recAdaptor = ::visuOgreAdaptor::SReconstruction::dynamicCast(adaptor.lock());
        recAdaptor->updateVisibility(!showRec);
    }
}

} // namespace visuOgreAdaptor.
