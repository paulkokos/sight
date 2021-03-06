/************************************************************************
 *
 * Copyright (C) 2009-2020 IRCAD France
 * Copyright (C) 2012-2020 IHU Strasbourg
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

#include "fwServices/IService.hpp"

#include "fwServices/registry/ActiveWorkers.hpp"
#include "fwServices/registry/ObjectService.hpp"
#include "fwServices/registry/Proxy.hpp"

#include <fwCom/Slot.hpp>
#include <fwCom/Slot.hxx>
#include <fwCom/Slots.hpp>
#include <fwCom/Slots.hxx>

#include <fwRuntime/Convert.hpp>
#include <fwRuntime/EConfigurationElement.hpp>

#include <fwThread/Worker.hpp>

#include <fwTools/fwID.hpp>

#include <functional>
#include <regex>

namespace fwServices
{

//-----------------------------------------------------------------------------

const ::fwCom::Signals::SignalKeyType IService::s_STARTED_SIG          = "started";
const ::fwCom::Signals::SignalKeyType IService::s_UPDATED_SIG          = "updated";
const ::fwCom::Signals::SignalKeyType IService::s_STOPPED_SIG          = "stopped";
const ::fwCom::Signals::SignalKeyType IService::s_INFO_NOTIFIED_SIG    = "infoNotified";
const ::fwCom::Signals::SignalKeyType IService::s_SUCCESS_NOTIFIED_SIG = "successNotified";
const ::fwCom::Signals::SignalKeyType IService::s_FAILURE_NOTIFIED_SIG = "failureNotified";

const ::fwCom::Slots::SlotKeyType IService::s_START_SLOT   = "start";
const ::fwCom::Slots::SlotKeyType IService::s_STOP_SLOT    = "stop";
const ::fwCom::Slots::SlotKeyType IService::s_UPDATE_SLOT  = "update";
const ::fwCom::Slots::SlotKeyType IService::s_SWAP_SLOT    = "swap";
const ::fwCom::Slots::SlotKeyType IService::s_SWAPKEY_SLOT = "swapKey";

//-----------------------------------------------------------------------------

IService::IService() :
    m_configuration( new ::fwRuntime::EConfigurationElement("EmptyConfigurationElement") ),
    m_globalState( STOPPED ),
    m_updatingState( NOTUPDATING ),
    m_configurationState( UNCONFIGURED )
{
    newSignal<StartedSignalType>( s_STARTED_SIG );
    newSignal<UpdatedSignalType>( s_UPDATED_SIG );
    newSignal<StoppedSignalType>( s_STOPPED_SIG );
    newSignal<InfoNotifiedSignalType>( s_INFO_NOTIFIED_SIG );
    newSignal<SuccessNotifiedSignalType>( s_SUCCESS_NOTIFIED_SIG );
    newSignal<FailureNotifiedSignalType>( s_FAILURE_NOTIFIED_SIG );

    m_slotStart   = newSlot( s_START_SLOT, &IService::startSlot, this );
    m_slotStop    = newSlot( s_STOP_SLOT, &IService::stopSlot, this );
    m_slotUpdate  = newSlot( s_UPDATE_SLOT, &IService::updateSlot, this );
    m_slotSwapKey = newSlot( s_SWAPKEY_SLOT, &IService::swapKeySlot, this );
}

//-----------------------------------------------------------------------------

IService::~IService()
{
    // check if the service manage outputs that are not maintained by someone else.
    if (!m_outputsMap.empty())
    {
        std::string objectKeys;
        for (const auto& obj: m_outputsMap)
        {
            const ::fwData::Object::wptr output = obj.second.get_shared();
            if (output.use_count() == 1)
            {
                if (!objectKeys.empty())
                {
                    objectKeys += ", ";
                }
                objectKeys += "'" + obj.first + "'(nbRef: " + std::to_string(output.use_count()) + ")";
            }
        }
        SLM_WARN_IF(
            "Service "+ this->getID() + " still contains registered outputs: " + objectKeys + ". They will no "
            "longer be maintained. You should call setOutput(key, nullptr) before stopping the service to inform "
            "AppManager and other services that the object will be destroyed.", !objectKeys.empty());
    }
}

//-----------------------------------------------------------------------------

void IService::info( std::ostream& )
{
}

//-----------------------------------------------------------------------------

void IService::setOutput(const IService::KeyType& key, const fwData::Object::sptr& object, size_t index)
{
    std::string outKey = key;

    if(m_keyGroupSize.find(key) != m_keyGroupSize.end())
    {
        outKey = KEY_GROUP_NAME(key, index);
    }
    if(::fwServices::OSR::isRegistered(outKey, ::fwServices::IService::AccessType::OUTPUT, this->getSptr()))
    {
        ::fwServices::OSR::unregisterServiceOutput(outKey, this->getSptr());
    }

    if(object != nullptr)
    {
        ::fwServices::OSR::registerServiceOutput(object, outKey, this->getSptr());
    }
}

//------------------------------------------------------------------------------

void IService::registerInput(const ::fwData::Object::csptr& obj, const std::string& key, const bool autoConnect,
                             const bool optional)
{
    this->registerObject(obj->getID(), key, AccessType::INPUT, autoConnect, optional);

    ::fwServices::OSR::registerServiceInput(obj, key, this->getSptr());
}

//------------------------------------------------------------------------------

void IService::unregisterInput(const std::string& key)
{
    this->unregisterObject(key, AccessType::INPUT);
}

//------------------------------------------------------------------------------

void IService::registerInOut(const ::fwData::Object::sptr& obj, const std::string& key, const bool autoConnect,
                             const bool optional)
{
    this->registerObject(obj, key, AccessType::INOUT, autoConnect, optional);
}

//------------------------------------------------------------------------------

void IService::unregisterInOut(const std::string& key)
{
    this->unregisterObject(key, AccessType::INOUT);
}

//------------------------------------------------------------------------------

void IService::registerObject(const ::fwData::Object::sptr& obj, const std::string& key,
                              AccessType access, const bool autoConnect, const bool optional)
{
    this->registerObject(key, access, autoConnect, optional);

    if (access == AccessType::INPUT)
    {
        m_inputsMap[key] = obj;
    }
    else if (access == AccessType::INOUT)
    {
        m_inOutsMap[key] = obj;
    }
    else if (access == AccessType::OUTPUT)
    {
        m_outputsMap[key] = obj;
    }

    ::fwServices::OSR::registerService(obj, key, access, this->getSptr());
}

//------------------------------------------------------------------------------

void IService::registerObject(const std::string& objId,
                              const ::fwServices::IService::KeyType& key,
                              const ::fwServices::IService::AccessType access,
                              const bool autoConnect, const bool optional)
{
    this->registerObject(key, access, autoConnect, optional);
    SLM_ASSERT("Object id must be defined", !objId.empty());
    this->setObjectId(key, objId);
}

//------------------------------------------------------------------------------

void IService::unregisterObject(const std::string& key, AccessType access)
{
    ::fwServices::OSR::unregisterService(key, access, this->getSptr());

    if(access == ::fwServices::IService::AccessType::INPUT)
    {
        m_inputsMap.erase(key);
    }
    else if(access == ::fwServices::IService::AccessType::INOUT)
    {
        m_inOutsMap.erase(key);
    }
    else
    {
        m_outputsMap.erase(key);
    }
}
//------------------------------------------------------------------------------

void IService::unregisterObject(const std::string& objId)
{
    auto itr = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                             [&](const ObjectServiceConfig& config)
        {
            return (config.m_uid == objId);
        });

    if (itr == m_serviceConfig.m_objects.end())
    {
        SLM_ERROR("object '" + objId + "' is not registered");
        return;
    }

    m_serviceConfig.m_objects.erase(itr);
}

//-----------------------------------------------------------------------------

bool IService::hasObjectId(const KeyType& _key) const
{
    bool hasId = false;
    auto itr   = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                               [&](const ObjectServiceConfig& objCfg)
        {
            return (objCfg.m_key == _key);
        });

    if (itr != m_serviceConfig.m_objects.end())
    {
        hasId = (!itr->m_uid.empty());
    }

    return hasId;
}

//-----------------------------------------------------------------------------

IService::IdType IService::getObjectId(const IService::KeyType& _key) const
{
    const ObjectServiceConfig& cfg = this->getObjInfoFromKey(_key);
    FW_RAISE_IF("Object key '" + _key + "' is not found for service '" + this->getID() + "'", cfg.m_uid.empty());
    return cfg.m_uid;
}

//-----------------------------------------------------------------------------

void IService::setObjectId(const IService::KeyType& _key, const IService::IdType& _id)
{
    auto keyItr = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                                [&](const ObjectServiceConfig& objCfg)
        {
            return (objCfg.m_key == _key);
        });
    FW_RAISE_IF("key '" + _key + "' is not regisreted for '" + this->getID() + "'.",
                keyItr == m_serviceConfig.m_objects.end());
    ObjectServiceConfig& cfg = *keyItr;
    cfg.m_uid = _id;
}

//-----------------------------------------------------------------------------

void IService::setObjectId(const IService::KeyType& _key, const size_t index, const IService::IdType& _id)
{
    const std::string groupKey = KEY_GROUP_NAME(_key, index);

    this->setObjectId(groupKey, _id);
    if (index >= this->getKeyGroupSize(_key))
    {
        m_keyGroupSize[_key] = index+1;
    }
}

//-----------------------------------------------------------------------------

void displayPt(::boost::property_tree::ptree& pt, std::string indent = "")
{
    OSLM_ERROR(indent << " data : '" << pt.data() << "'" );

    for( ::boost::property_tree::ptree::value_type& v :  pt)
    {
        OSLM_ERROR((indent + "  '") << v.first << "':" );
        displayPt(v.second, indent + "      ");

    }
}

//-----------------------------------------------------------------------------

void IService::setConfiguration(const ::fwRuntime::ConfigurationElement::sptr _cfgElement)
{
    SLM_ASSERT( "Invalid ConfigurationElement", _cfgElement );
    m_configuration      = _cfgElement;
    m_configurationState = UNCONFIGURED;
}

//-----------------------------------------------------------------------------

void IService::setConfiguration(const Config& _configuration)
{
    SLM_ASSERT( "Invalid ConfigurationElement", _configuration.m_config );

    // TODO: Remove this ugly const_cast
    m_configuration      = ::fwRuntime::ConfigurationElement::constCast(_configuration.m_config);
    m_configurationState = UNCONFIGURED;

    m_serviceConfig = _configuration;
}

//-----------------------------------------------------------------------------

void IService::setConfiguration( const ConfigType& ptree )
{
    ::fwRuntime::ConfigurationElement::sptr ce;

    ConfigType serviceConfig;
    serviceConfig.add_child("service", ptree);

    ce = ::fwRuntime::Convert::fromPropertyTree(serviceConfig);

    SLM_ASSERT( "Invalid ConfigurationElement", ce );

    this->setConfiguration(ce);
}

//-----------------------------------------------------------------------------

::fwRuntime::ConfigurationElement::sptr IService::getConfiguration() const
{
    return m_configuration;
}

//-----------------------------------------------------------------------------

IService::ConfigType IService::getConfigTree() const
{
    const auto configTree = ::fwRuntime::Convert::toPropertyTree(this->getConfiguration());

    // This is in case we get the configuration from a ::fwServices::registry::ServiceConfig
    auto srvConfig = configTree.get_child_optional("config");

    if(srvConfig.is_initialized())
    {
        return srvConfig.value();
    }
    else
    {
        srvConfig = configTree.get_child_optional("service");
        if(srvConfig.is_initialized())
        {
            return srvConfig.value();
        }
        return IService::ConfigType();
    }
}

//-----------------------------------------------------------------------------

void IService::configure()
{
    if( m_configurationState == UNCONFIGURED )
    {
        m_configurationState = CONFIGURING;
        if( m_globalState == STOPPED )
        {
            try
            {
                this->configuring();
            }
            catch (std::exception& e)
            {
                SLM_ERROR("Error while configuring service '" + this->getID() + "' : " + e.what());
            }
        }
        else if( m_globalState == STARTED )
        {
            this->reconfiguring();
        }
        m_configurationState = CONFIGURED;
    }
}

//-----------------------------------------------------------------------------

void IService::configure(const ConfigType& ptree)
{
    ::fwRuntime::ConfigurationElement::sptr ce;

    ConfigType serviceConfig;
    serviceConfig.add_child("service", ptree);

    ce = ::fwRuntime::Convert::fromPropertyTree(serviceConfig);

    SLM_ASSERT( "Invalid ConfigurationElement", ce );

    this->setConfiguration(ce);
    this->configure();
}

//-----------------------------------------------------------------------------

void IService::reconfiguring()
{
    OSLM_FATAL(
        "If this method (reconfiguring) is called, it must be overridden in the implementation ("
            << this->getClassname() <<", "<< this->getID() << ")" );
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::start()
{
    if( !m_associatedWorker || ::fwThread::getCurrentThreadId() == m_associatedWorker->getThreadId() )
    {
        return this->internalStart(false);
    }
    else
    {
        return m_slotStart->asyncRun();
    }
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::stop()
{
    if( !m_associatedWorker || ::fwThread::getCurrentThreadId() == m_associatedWorker->getThreadId() )
    {
        return this->internalStop(false);
    }
    else
    {
        return m_slotStop->asyncRun();
    }
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::update()
{
    if( !m_associatedWorker || ::fwThread::getCurrentThreadId() == m_associatedWorker->getThreadId() )
    {
        return this->internalUpdate(false);
    }
    else
    {
        return m_slotUpdate->asyncRun();
    }
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::swapKey(const IService::KeyType& _key, fwData::Object::sptr _obj)
{
    if( !m_associatedWorker || ::fwThread::getCurrentThreadId() == m_associatedWorker->getThreadId() )
    {
        return this->internalSwapKey(_key, _obj, false);
    }
    else
    {
        return m_slotSwapKey->asyncRun( _key, _obj );
    }
}

//-----------------------------------------------------------------------------

IService::GlobalStatus IService::getStatus() const noexcept
{
    return m_globalState;
}

//-----------------------------------------------------------------------------

bool IService::isStarted() const noexcept
{
    return (m_globalState == STARTED);
}

//-----------------------------------------------------------------------------

bool IService::isStopped() const noexcept
{
    return (m_globalState == STOPPED);
}

//-----------------------------------------------------------------------------

IService::ConfigurationStatus IService::getConfigurationStatus() const noexcept
{
    return m_configurationState;
}

//-----------------------------------------------------------------------------

IService::UpdatingStatus IService::getUpdatingStatus() const noexcept
{
    return m_updatingState;
}

//-----------------------------------------------------------------------------

void IService::setWorker( ::fwThread::Worker::sptr worker )
{
    m_associatedWorker = worker;
    ::fwCom::HasSlots::m_slots.setWorker( m_associatedWorker );
}

//-----------------------------------------------------------------------------

::fwThread::Worker::sptr IService::getWorker() const
{
    return m_associatedWorker;
}

//-----------------------------------------------------------------------------

IService::KeyConnectionsMap IService::getAutoConnections() const
{
    KeyConnectionsMap connections;
    return connections;
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::startSlot()
{
    return this->internalStart(true);
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::internalStart(bool _async)
{
    OSLM_FATAL_IF("Service "<<this->getID()<<" already started", m_globalState != STOPPED);

    this->connectToConfig();

    m_globalState = STARTING;

    PackagedTaskType task( std::bind(&IService::starting, this) );
    SharedFutureType future = task.get_future();
    task();

    try
    {
        // This allows to trigger the exception if there was one
        future.get();
    }
    catch (const std::exception& e)
    {
        SLM_ERROR("Error while STARTING service '" + this->getID() + "' : " + e.what());
        SLM_ERROR("Service '" + this->getID() + "' is still STOPPED.");
        m_globalState = STOPPED;
        this->disconnectFromConfig();

        if(!_async)
        {
            // The future is shared, thus the caller can still catch the exception if needed with ufuture.get()
            return future;
        }
        else
        {
            // Rethrow the same exception
            throw;
        }

    }
    m_globalState = STARTED;

    this->autoConnect();

    auto sig = this->signal<StartedSignalType>(s_STARTED_SIG);
    sig->asyncEmit();

    return future;
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::stopSlot()
{
    return this->internalStop(true);
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::internalStop(bool _async)
{
    OSLM_FATAL_IF("Service "<<this->getID()<<" already stopped", m_globalState != STARTED);

    this->autoDisconnect();

    PackagedTaskType task( std::bind(&IService::stopping, this) );
    SharedFutureType future = task.get_future();

    m_globalState = STOPPING;
    task();

    try
    {
        // This allows to trigger the exception if there was one
        future.get();
    }
    catch (std::exception& e)
    {
        SLM_ERROR("Error while STOPPING service '" + this->getID() + "' : " + e.what());
        SLM_ERROR("Service '" + this->getID() + "' is still STARTED.");
        m_globalState = STARTED;
        this->autoConnect();

        if(!_async)
        {
            // The future is shared, thus the caller can still catch the exception if needed with ufuture.get()
            return future;
        }
        else
        {
            // Rethrow the same exception
            throw;
        }
    }
    m_globalState = STOPPED;

    auto sig = this->signal<StoppedSignalType>(s_STOPPED_SIG);
    sig->asyncEmit();

    this->disconnectFromConfig();

    return future;

}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::swapKeySlot(const KeyType& _key, ::fwData::Object::sptr _obj)
{
    return this->internalSwapKey(_key, _obj, true);
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::internalSwapKey(const KeyType& _key, ::fwData::Object::sptr _obj, bool _async)
{
    OSLM_FATAL_IF("Service "<< this->getID() << " is not STARTED, no swapping with Object " <<
                  (_obj ? _obj->getID() : "nullptr"),
                  m_globalState != STARTED);

    auto fn = std::bind(static_cast<void (IService::*)(const KeyType&)>(&IService::swapping), this, _key);
    PackagedTaskType task( fn );
    SharedFutureType future = task.get_future();

    this->autoDisconnect();

    m_globalState = SWAPPING;
    task();
    m_globalState = STARTED;

    try
    {
        // This allows to trigger the exception if there was one
        future.get();
    }
    catch (std::exception& e)
    {
        SLM_ERROR("Error while SWAPPING service '" + this->getID() + "' : " + e.what());

        if(!_async)
        {
            // The future is shared, thus the caller can still catch the exception if needed with ufuture.get()
            return future;
        }
        else
        {
            // Rethrow the same exception
            throw;
        }
    }

    this->autoConnect();

    return future;

}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::updateSlot()
{
    return this->internalUpdate(true);
}

//-----------------------------------------------------------------------------

IService::SharedFutureType IService::internalUpdate(bool _async)
{
    if(m_globalState != STARTED)
    {
        OSLM_WARN("INVOKING update WHILE STOPPED ("<<m_globalState<<") on service '" << this->getID() <<
                  "' of type '" << this->getClassname() << "': update is discarded." );
        return SharedFutureType();
    }
    OSLM_ASSERT("INVOKING update WHILE NOT IDLE ("<<m_updatingState<<") on service '" << this->getID() <<
                "' of type '" << this->getClassname() << "'", m_updatingState == NOTUPDATING );

    PackagedTaskType task( std::bind(&IService::updating, this) );
    SharedFutureType future = task.get_future();
    m_updatingState = UPDATING;
    task();

    try
    {
        // This allows to trigger the exception if there was one
        future.get();
    }
    catch (std::exception& e)
    {
        SLM_ERROR("Error while UPDATING service '" + this->getID() + "' : " + e.what());

        m_updatingState = NOTUPDATING;
        if(!_async)
        {
            // The future is shared, thus the caller can still catch the exception if needed with ufuture.get()
            return future;
        }
        else
        {
            // Rethrow the same exception
            throw;
        }
    }
    m_updatingState = NOTUPDATING;

    auto sig = this->signal<StartedSignalType>(s_UPDATED_SIG);
    sig->asyncEmit();

    return future;
}

//-----------------------------------------------------------------------------

void IService::connectToConfig()
{
    ::fwServices::registry::Proxy::sptr proxy = ::fwServices::registry::Proxy::getDefault();

    for(const auto& proxyCfg : m_proxies)
    {
        for(const auto& signalCfg : proxyCfg.second.m_signals)
        {
            SLM_ASSERT("Invalid signal source", signalCfg.first == this->getID());

            ::fwCom::SignalBase::sptr sig = this->signal(signalCfg.second);
            SLM_ASSERT("Signal '" + signalCfg.second + "' not found in source '" + signalCfg.first + "'.", sig);
            try
            {
                proxy->connect(proxyCfg.second.m_channel, sig);

            }
            catch (const std::exception& e)
            {
                SLM_ERROR("Signal '" + signalCfg.second + "' from '" + signalCfg.first + "' can not be connected to the"
                          " channel '" + proxyCfg.second.m_channel + "': " + std::string(e.what()));
            }
        }

        for(const auto& slotCfg : proxyCfg.second.m_slots)
        {
            SLM_ASSERT("Invalid slot destination", slotCfg.first == this->getID());

            ::fwCom::SlotBase::sptr slot = this->slot(slotCfg.second);
            SLM_ASSERT("Slot '" + slotCfg.second + "' not found in source '" + slotCfg.first + "'.", slot);

            try
            {
                proxy->connect(proxyCfg.second.m_channel, slot);
            }
            catch (const std::exception& e)
            {
                SLM_ERROR("Slot '" + slotCfg.second + "' from '" + slotCfg.first + "' can not be connected to the "
                          "channel '" + proxyCfg.second.m_channel + "': " + std::string(e.what()));
            }
        }
    }
}

//-----------------------------------------------------------------------------

void IService::autoConnect()
{
    ::fwServices::IService::KeyConnectionsMap connectionMap = this->getAutoConnections();

    SLM_ERROR_IF("The service '" + this->getID() + "'(" + this->getClassname() +
                 ") is set to 'autoConnect=\"yes\"' but is has no object to connect",
                 m_serviceConfig.m_globalAutoConnect && m_serviceConfig.m_objects.empty());

    for(const auto& objectCfg : m_serviceConfig.m_objects)
    {
        if (m_serviceConfig.m_globalAutoConnect || objectCfg.m_autoConnect)
        {
            ::fwServices::IService::KeyConnectionsType connections;
            if(!connectionMap.empty())
            {
                auto it = connectionMap.find(objectCfg.m_key);
                if( it != connectionMap.end())
                {
                    connections = it->second;
                }
                else
                {
                    // Special case if we have a key from a group we check with the name of the group
                    std::smatch match;
                    static const std::regex reg("(.*)#[0-9]+");
                    if( std::regex_match(objectCfg.m_key, match, reg ) && match.size() == 2)
                    {
                        const std::string group = match[1].str();
                        auto itConnection       = connectionMap.find(group);
                        if( itConnection != connectionMap.end())
                        {
                            connections = itConnection->second;
                        }
                    }
                }
                SLM_ERROR_IF("Object '" + objectCfg.m_key + "' of '" + this->getID() + "'(" + this->getClassname() +
                             ") is set to 'autoConnect=\"yes\"' but there is no connection available.",
                             connections.empty() && objectCfg.m_autoConnect);
            }
            else
            {
                SLM_ERROR("Object '" + objectCfg.m_key + "' of '" + this->getID() + "'(" + this->getClassname() +
                          ") is set to 'autoConnect=\"yes\"' but there is no connection available.");
            }

            ::fwData::Object::csptr obj;

            switch(objectCfg.m_access)
            {
                case AccessType::INPUT:
                {
                    auto itObj = m_inputsMap.find(objectCfg.m_key);
                    if(itObj != m_inputsMap.end())
                    {
                        obj = itObj->second.getShared();
                    }
                    break;
                }
                case AccessType::INOUT:
                {
                    auto itObj = m_inOutsMap.find(objectCfg.m_key);
                    if(itObj != m_inOutsMap.end())
                    {
                        obj = itObj->second.getShared();
                    }
                    break;
                }
                case AccessType::OUTPUT:
                {
                    SLM_WARN("Can't autoConnect to an output for now");
                    auto itObj = m_outputsMap.find(objectCfg.m_key);
                    if(itObj != m_outputsMap.end())
                    {
                        obj = itObj->second.get_shared();
                    }
                    break;
                }
            }

            SLM_ASSERT("Object '" + objectCfg.m_key +
                       "' has not been found when autoConnecting service '" + m_serviceConfig.m_uid + "'.",
                       (!objectCfg.m_optional && obj) || objectCfg.m_optional);

            if(obj)
            {
                m_autoConnections.connect( obj, this->getSptr(), connections );
            }
        }
    }
}

//-----------------------------------------------------------------------------

void IService::disconnectFromConfig()
{
    ::fwServices::registry::Proxy::sptr proxy = ::fwServices::registry::Proxy::getDefault();

    for(const auto& proxyCfg : m_proxies)
    {
        for(const auto& signalCfg : proxyCfg.second.m_signals)
        {
            SLM_ASSERT("Invalid signal source", signalCfg.first == this->getID());

            ::fwCom::SignalBase::sptr sig = this->signal(signalCfg.second);

            try
            {
                proxy->disconnect(proxyCfg.second.m_channel, sig);
            }
            catch (const std::exception& e)
            {
                SLM_ERROR("Signal '" + signalCfg.second + "' from '" + signalCfg.first + "' can not be disconnected "
                          "from the channel '" + proxyCfg.second.m_channel + "': " + std::string(e.what()));
            }
        }
        for(const auto& slotCfg : proxyCfg.second.m_slots)
        {
            SLM_ASSERT("Invalid slot destination", slotCfg.first == this->getID());

            ::fwCom::SlotBase::sptr slot = this->slot(slotCfg.second);
            try
            {
                proxy->disconnect(proxyCfg.second.m_channel, slot);
            }
            catch (const std::exception& e)
            {
                SLM_ERROR("Slot '" + slotCfg.second + "' from '" + slotCfg.first + "' can not be disconnected from the "
                          "channel '" + proxyCfg.second.m_channel + "': " + std::string(e.what()));
            }
        }
    }
}

//-----------------------------------------------------------------------------

void IService::autoDisconnect()
{
    m_autoConnections.disconnect();
}

//-----------------------------------------------------------------------------

void IService::addProxyConnection(const helper::ProxyConnections& proxy)
{
    m_proxies[proxy.m_channel] = proxy;
}

//-----------------------------------------------------------------------------

bool IService::hasObjInfoFromId(const std::string& objId) const
{
    auto itr = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                             [&](const ObjectServiceConfig& objCfg)
        {
            return (objCfg.m_uid == objId);
        });

    return (itr != m_serviceConfig.m_objects.end());
}

//------------------------------------------------------------------------------

bool IService::hasAllRequiredObjects() const
{

    bool hasAllObjects = true;

    for (const auto& objectCfg : m_serviceConfig.m_objects)
    {
        if (objectCfg.m_optional == false)
        {
            if (objectCfg.m_access == ::fwServices::IService::AccessType::INPUT)
            {
                if (nullptr == this->getInput< ::fwData::Object >(objectCfg.m_key))
                {
                    SLM_DEBUG("The 'input' object with key '" + objectCfg.m_key + "' is missing for '" + this->getID()
                              + "'");
                    hasAllObjects = false;
                    break;
                }
            }
            else if (objectCfg.m_access == ::fwServices::IService::AccessType::INOUT)
            {
                if (nullptr == this->getInOut< ::fwData::Object >(objectCfg.m_key))
                {
                    SLM_DEBUG("The 'input' object with key '" + objectCfg.m_key + "' is missing for '" + this->getID()
                              + "'");
                    hasAllObjects = false;
                    break;
                }
            }
        }
    }
    return hasAllObjects;
}

//------------------------------------------------------------------------------

const IService::ObjectServiceConfig& IService::getObjInfoFromId(const std::string& objId) const
{
    auto idItr = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                               [&](const ObjectServiceConfig& objCfg)
        {
            return (objCfg.m_uid == objId);
        });
    FW_RAISE_IF("Object '" + objId + "' is not regisreted for '" + this->getID() + "'.",
                idItr == m_serviceConfig.m_objects.end());

    return *idItr;
}
//------------------------------------------------------------------------------

const IService::ObjectServiceConfig& IService::getObjInfoFromKey(const std::string& key) const
{
    auto keyItr = std::find_if( m_serviceConfig.m_objects.begin(),  m_serviceConfig.m_objects.end(),
                                [&](const ObjectServiceConfig& objCfg)
        {
            return (objCfg.m_key == key);
        });
    FW_RAISE_IF("key '" + key + "' is not regisreted for '" + this->getID() + "'.",
                keyItr == m_serviceConfig.m_objects.end());

    return *keyItr;
}

//-----------------------------------------------------------------------------

void IService::registerObject(const ::fwServices::IService::KeyType& key,
                              const ::fwServices::IService::AccessType access,
                              const bool autoConnect, const bool optional)
{
    auto itr = std::find_if( m_serviceConfig.m_objects.begin(), m_serviceConfig.m_objects.end(),
                             [&](const ObjectServiceConfig& objInfo)
        {

            return (objInfo.m_key == key);
        });
    if (itr == m_serviceConfig.m_objects.end())
    {
        ObjectServiceConfig objConfig;
        objConfig.m_key         = key;
        objConfig.m_access      = access;
        objConfig.m_autoConnect = autoConnect;
        objConfig.m_optional    = optional;

        m_serviceConfig.m_objects.push_back(objConfig);
    }
    else
    {
        SLM_WARN("object '" + key + "' is already registered, it will be overridden");

        ObjectServiceConfig& objConfig = *itr;
        objConfig.m_key         = key;
        objConfig.m_access      = access;
        objConfig.m_autoConnect = autoConnect;
        objConfig.m_optional    = optional;
    }
}

//-----------------------------------------------------------------------------

void IService::registerObjectGroup(const std::string& key, AccessType access, const std::uint8_t minNbObject,
                                   const bool autoConnect, const std::uint8_t maxNbObject)
{
    for (std::uint8_t i = 0; i < maxNbObject; ++i)
    {
        const bool optional = (i < minNbObject ? false : true);
        ObjectServiceConfig objConfig;
        objConfig.m_key         = KEY_GROUP_NAME(key, i);
        objConfig.m_access      = access;
        objConfig.m_autoConnect = autoConnect;
        objConfig.m_optional    = optional;

        m_serviceConfig.m_objects.push_back(objConfig);
    }
    m_keyGroupSize[key] = minNbObject;
}

//-----------------------------------------------------------------------------

/**
 * @brief Streaming a service
 * @see IService::operator<<(std::ostream & _ostream, IService& _service)
 * @note Invoke IService::info( std::ostream )
 */
std::ostream& operator<<(std::ostream& _ostream, IService& _service)
{
    _service.info( _ostream );
    return _ostream;
}

//-----------------------------------------------------------------------------

}
