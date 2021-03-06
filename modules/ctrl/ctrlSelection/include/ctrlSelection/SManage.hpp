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

#include "ctrlSelection/config.hpp"

#include <fwData/Object.hpp>

#include <fwServices/IController.hpp>

namespace ctrlSelection
{

/**
 * @brief This service manages an object (add/swap/remove) into a container object (composite, vector, seriesDB).
 *
 * It works on different objects:
 * - ::fwData::Composite: the object is added/swapped/removed from composite at the given key
 * - ::fwData::Vector: the object is added or removed from the container
 * - ::fwMedData::SeriesDB: the object is added or removed from the container
 * - ::fwData::Object: the object is added or removed from the field map at the given key
 *
 * @section Slots Slots
 * - \b add() : Adds the object into the target (vector, seriesDB, composite), if target is a Composite or a Field, it
 *  is added at the key given by config.
 * - \b addCopy() : Adds a copy of the object into the target (vector, seriesDB, composite), if target is a Composite
 * or a Field, it is added at the key given by config.
 * - \b addOrSwap() : Adds the object if it is not present in the target, else if target is a composite or a Field, the
 * object is swapped.
 * - \b swapObj() : Only if target is a Composite or a Field : swaps the object into the composite with the key given by
 * config.
 * - \b remove() : Removes the object.
 * - \b removeIfPresent() : Removes the object if it is present.
 * - \b clear() : Removes all objects.
 *
 * @section XML XML Configuration
 *
 * For ::fwData::Composite:
 * @code{.xml}
   <service type="::ctrlSelection::SManage">
      <inout key="object" uid="..." />
      <inout key="composite" uid="..." />
      <compositeKey>...</compositeKey>
   </service>
   @endcode
 *
 * For ::fwData::Vector:
 * @code{.xml}
   <service type="::ctrlSelection::SManage">
      <inout key="object" uid="..." />
      <inout key="vector" uid="..." />
   </service>
   @endcode
 *
 * For ::fwMedData::SeriesDB:
 * @code{.xml}
   <service type="::ctrlSelection::SManage">
      <inout key="object" uid="..." />
      <inout key="seriesDB" uid="..." />
   </service>
   @endcode
 *
 * For ::fwData::Object:
 * @code{.xml}
   <service type="::ctrlSelection::SManage">
      <inout key="object" uid="..." />
      <inout key="fieldHolder" uid="..." />
      <field>...</field>
   </service>
   @endcode
 *
 * @subsection In-Out In-Out
 * - \b object [::fwData::Object] (optional): object to add/swap/remove. Not needed when invoking clean slot and remove
 * slots with composites and fields, since the removal is based on the name.
 * - \b composite [::fwData::Composite] (optional): Composite where to add/swap/remove object.
 * - \b vector [::fwData::Vector] (optional): Vector where to add/remove object.
 * - \b seriesDB [::fwMedData::SeriesDB] (optional): SeriesDB where to add/remove object.
 * - \b fieldHolder [::fwData::Object] (optional): Object where to add/swap/remove object as a field.
 *
 * <b>Only one of the target (composite, vector or seriesDB) is allowed.</b>
 * For SeriesDB, the object must inherit of Series
 * @subsection Configuration Configuration
 * - \b compositeKey (optional, only if target object in a Composite) : key of the object in the composite
 * - \b field (optional, only if target object in a ::fwData::Object) : name of the field
 */
class CTRLSELECTION_CLASS_API SManage : public ::fwServices::IController
{

public:

    fwCoreServiceMacro(SManage, ::fwServices::IController);

    /// Constructor.  Do nothing.
    CTRLSELECTION_API SManage() noexcept;

    /// Destructor. Do nothing.
    CTRLSELECTION_API virtual ~SManage() noexcept;

    /**
     * @name Slots
     * @{
     */
    static const ::fwCom::Slots::SlotKeyType s_ADD_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_ADD_COPY_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_ADD_OR_SWAP_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_SWAP_OBJ_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_REMOVE_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_REMOVE_IF_PRESENT_SLOT;
    static const ::fwCom::Slots::SlotKeyType s_CLEAR_SLOT;
    /**
     * @}
     */

protected:

    /// Configures the service.
    CTRLSELECTION_API virtual void configuring() override;

    /// Implements starting method derived from IService. Do nothing.
    CTRLSELECTION_API virtual void starting() override;

    /// Implements stopping method derived from IService. Do nothing.
    CTRLSELECTION_API virtual void stopping() override;

    /// Implements updating method derived from IService. Do nothing.
    CTRLSELECTION_API virtual void updating() override;

    /**
     * @name Slots
     * @{
     */
    /// Adds the object into the composite with the key given by config.
    void add();

    /// Adds a copy of object into the composite with the key given by config.
    void addCopy();

    /**
     * @brief Adds or swap the object into the composite with the key given by config.
     *
     * Adds the object if it is not present in the composite, else swaps it.
     */
    void addOrSwap();

    /// Swaps the object into the composite with the key given by config.
    void swap();

    /// Removes the object from the composite at the key given by config.
    void remove();

    /// Removes the object from the composite at the key given by config if it is present.
    void removeIfPresent();

    /// Removes all objects.
    void clear();
    /**
     * @}
     */

private:

    void internalAdd(bool _copy);

    std::string m_objectUid; ///< uid of the object
    std::string m_compositeKey; ///< key of the object to manage in the composite
    std::string m_fieldName; ///< name of the field to manage in the object

};

} // ctrlSelection
