/************************************************************************
 *
 * Copyright (C) 2020 IRCAD France
 * Copyright (C) 2020 IHU Strasbourg
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

#include "fwStructuralPatch/config.hpp"

#include <fwAtomsPatch/IStructuralPatch.hpp>

namespace fwStructuralPatch
{
namespace fwMedData
{
namespace Study
{

/// Structural patch to convert a fwMedData::Study from version '1' to version '2'.
class FWSTRUCTURALPATCH_CLASS_API V1ToV2 : public ::fwAtomsPatch::IStructuralPatch
{

public:

    fwCoreClassMacro(V1ToV2, ::fwStructuralPatch::fwMedData::Study::V1ToV2, new V1ToV2)

    /// Initialiaze members.
    FWSTRUCTURALPATCH_API V1ToV2();

    /// Deletes resources.
    FWSTRUCTURALPATCH_API ~V1ToV2();

    /**
     * @brief Copies the patch.
     * @param _cpy the patch to copy.
     */
    FWSTRUCTURALPATCH_API V1ToV2(const V1ToV2& _cpy);

    /**
     * @brief Applies patch
     *
     * Adds new attributes study_id, consulting_physician_name, patient_size, patient_weight, patient_body_mass_index.
     */
    FWSTRUCTURALPATCH_API virtual void apply(
        const ::fwAtoms::Object::sptr& _previous,
        const ::fwAtoms::Object::sptr& _current,
        ::fwAtomsPatch::IPatch::NewVersionsType& _newVersions) override;

};

} // namespace Study
} // namespace fwMedData
} // namespace fwStructuralPatch
