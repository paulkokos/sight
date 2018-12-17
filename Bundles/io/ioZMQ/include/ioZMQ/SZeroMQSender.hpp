/************************************************************************
 *
 * Copyright (C) 2009-2018 IRCAD France
 * Copyright (C) 2012-2018 IHU Strasbourg
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

#include "ioZMQ/config.hpp"

#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>

#include <fwTools/Failed.hpp>

#include <zmqNetwork/Socket.hpp>

#include <ioNetwork/INetworkSender.hpp>

namespace ioZMQ
{

/**
 *
 * @brief network sender using ZMQNetwork library
 */
class IOZMQ_CLASS_API SZeroMQSender : public ::ioNetwork::INetworkSender
{
public:

    fwCoreServiceClassDefinitionsMacro( (SZeroMQSender) (::ioNetwork::INetworkSender));

    /**
     * @brief Slot to update configuration
     */
    IOZMQ_API static const ::fwCom::Slots::SlotKeyType s_UPDATE_CONFIGURATION_SLOT;

    /// Constructor
    IOZMQ_API SZeroMQSender();

    /// Destructor
    IOZMQ_API virtual ~SZeroMQSender();

protected:
    /**
     * @brief configure the listener and parse configuration with ZeroMQConfigurationParser
     *
     * @see ZeroMQConfigurationParser for the format of the configuration
     */
    IOZMQ_API virtual void configuring() override;

    /**
     * @brief start the socket in a thread
     */
    IOZMQ_API virtual void starting() override;

    /**
     * @brief stop the socket
     */
    IOZMQ_API virtual void stopping() override;

    /**
     * @brief change the port of the service work only in tcp mode
     */
    IOZMQ_API void setPort(const std::uint16_t port);

    /**
     * @brief slot method to update configuration
     *
     * @param[in] patternMode pattern mode (push, pull,...)
     * @param[in] sockMode socket mode (client or server)
     * @param[in] host new host
     */
    IOZMQ_API void updateConfiguration(const ::zmqNetwork::Socket::PatternMode patternMode,
                                       const ::zmqNetwork::Socket::SocketMode sockMode,
                                       const std::string& host);

private:

    /**
     * @brief method to send data object
     *
     * @see INetworkSender
     */
    void sendObject(const ::fwData::Object::csptr& obj);

    /**
     * @brief method to send data object
     *
     * @see INetworkSender
     */
    void sendObject(const ::fwData::Object::csptr& obj, const size_t index) override;

    /// Socket instance
    ::zmqNetwork::Socket::sptr m_socket;

    /// PatternMode to use (Push, Pull, Publish,...)
    ::zmqNetwork::Socket::PatternMode m_patternMode;

    /// Socket mode to use (Client, Server)
    ::zmqNetwork::Socket::SocketMode m_sockMode;

    /// host to connect or interface to bind
    std::string m_hostStr;
};

} // namespace ioZMQ
