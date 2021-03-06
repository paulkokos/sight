/************************************************************************
 *
 * Copyright (C) 2017-2019 IRCAD France
 * Copyright (C) 2017-2019 IHU Strasbourg
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

#include "arStructuralPatch/config.hpp"

#include <fwAtomsPatch/IStructuralPatch.hpp>

namespace arStructuralPatch
{

namespace arData
{

namespace Camera
{

/// Structural patch to convert a arData::Camera from version '3' to version '2'.
class ARSTRUCTURALPATCH_CLASS_API V3ToV2 : public ::fwAtomsPatch::IStructuralPatch
{
public:
    fwCoreClassMacro(V3ToV2, ::arStructuralPatch::arData::Camera::V3ToV2, new V3ToV2);

    /// Constructor
    ARSTRUCTURALPATCH_API V3ToV2();

    /// Destructor
    ARSTRUCTURALPATCH_API ~V3ToV2();

    /// Copy constructor
    ARSTRUCTURALPATCH_API V3ToV2( const V3ToV2& cpy );

    /**
     * @brief Applies patch
     *
     * Removes scale attribute.
     */
    ARSTRUCTURALPATCH_API virtual void apply(
        const ::fwAtoms::Object::sptr& previous,
        const ::fwAtoms::Object::sptr& current,
        ::fwAtomsPatch::IPatch::NewVersionsType& newVersions) override;

};

} // namespace Camera

} // namespace arData

} // namespace arStructuralPatch
