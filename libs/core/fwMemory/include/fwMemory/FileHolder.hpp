/************************************************************************
 *
 * Copyright (C) 2009-2019 IRCAD France
 * Copyright (C) 2012-2019 IHU Strasbourg
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

#include "fwMemory/config.hpp"

#include <fwCore/macros.hpp>

#include <filesystem>

namespace fwMemory
{

struct FileAutoDelete;

class FWMEMORY_CLASS_API FileHolder
{
public:
    FileHolder()
    {
    }
    FWMEMORY_API FileHolder(const std::filesystem::path& file, bool autodelete = false);

    operator std::filesystem::path() const
    {
        return m_path;
    }

    //------------------------------------------------------------------------------

    bool empty() const
    {
        return m_path.empty();
    }

    //------------------------------------------------------------------------------

    void clear()
    {
        m_path.clear();
        m_autoDelete.reset();
    }

    //------------------------------------------------------------------------------

    std::string string() const
    {
        return m_path.string();
    }

protected:
    std::filesystem::path m_path;
    SPTR(FileAutoDelete) m_autoDelete;
};

} // namespace fwMemory
