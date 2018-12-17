/************************************************************************
 *
 * Copyright (C) 2009-2015 IRCAD France
 * Copyright (C) 2012-2015 IHU Strasbourg
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

#ifndef __FWCORE_LOGICSTAMP_HPP__
#define __FWCORE_LOGICSTAMP_HPP__

#include "fwCore/base.hpp"
#include "fwCore/HiResClock.hpp"


namespace fwCore
{

/**
 * @brief Provides a Logical timestamp system.
 *
 *
 */
class FWCORE_CLASS_API LogicStamp : public BaseObject
{

public:
    fwCoreClassDefinitionsWithFactoryMacro( (LogicStamp)(BaseObject), (()), new LogicStamp);

    /**
     * @brief Type used in logical typestamp.
     */
    typedef unsigned long LogicStampType;

    /**
     * @name Constructor/Destructor
     * @{ */

    FWCORE_API LogicStamp()
    {
        this->m_modifiedLogicalTime = 0;
    }

    /**  @} */


    /**
     * @brief Increment global Logical counter and copy it to this LogicStamp.
     */
    FWCORE_API void modified();


    /**
     * @return LogicStamp's current value
     */
    FWCORE_API LogicStampType getLogicStamp() const
    {
        return this->m_modifiedLogicalTime;
    }


    /**
     * @brief Greater than operator for LogicStamp.
     *
     * @param ls LogicStamp to compare to
     */
    FWCORE_API bool operator>(const LogicStamp& ls) const
    {
        return ( this->m_modifiedLogicalTime > ls.m_modifiedLogicalTime );
    }

    /**
     * @brief Lesser than operator for LogicStamp.
     *
     * @param ls LogicStamp to compare to
     */
    FWCORE_API bool operator<(const LogicStamp& ls) const
    {
        return ( this->m_modifiedLogicalTime < ls.m_modifiedLogicalTime );
    }

    /**
     * @brief Cast operator for LogicStamp.
     */
    FWCORE_API operator LogicStampType() const
    {
        return this->m_modifiedLogicalTime;
    }


private:

    /**
     * @brief Stored logical time
     */
    LogicStampType m_modifiedLogicalTime;

};

} //namespace fwCore

#endif // __FWCORE_LOGICSTAMP_HPP__

