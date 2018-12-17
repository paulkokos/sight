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

#ifndef __FWSTRUCTURALPATCH_PATCHLOADER_HPP__
#define __FWSTRUCTURALPATCH_PATCHLOADER_HPP__

#include "fwStructuralPatch/config.hpp"

namespace fwStructuralPatch
{

/// Loads patches.
class PatchLoader
{
public:
    /// Function used to force link.
    FWSTRUCTURALPATCH_API static void loadPatches();
};

} //namespace fwStructuralPatch

#endif /* __FWSTRUCTURALPATCH_PATCHLOADER_HPP__ */

