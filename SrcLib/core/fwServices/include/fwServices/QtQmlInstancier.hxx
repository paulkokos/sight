/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2018-2018.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#pragma once

# include "fwServices/config.hpp"
# include "fwServices/IQtQmlObject.hpp"

# include <QObject>

# include <vector>
# include <memory>

namespace fwServices
{

/**
 * @brief: This class is a manager for holding registered class and instanciate new objects
 */
class FWSERVICES_CLASS_QT_API QtQmlInstancier
{
public:
    // Cannot be instancied
    FWSERVICES_QT_API QtQmlInstancier()  = delete;
    FWSERVICES_QT_API ~QtQmlInstancier() = delete;

public:
    /**
     *  @brief: This method add IQmlObject to the current list of object (m_classList)
     */
    static FWSERVICES_QT_API void	addClass(std::shared_ptr<IQtQmlObject> const&);
    /**
     *  @brief: This method take a classType as parameters (stored in IQtQmlObject::m_cType)
     *      and create an instance the related object.
     */
    static FWSERVICES_QT_API QObject* instanciate(std::string const& cType);

    /**
     *  @brief: This method take a classType as parameters (stored in IQtQmlObject::m_cType)
     *      and create an instance of the related object.
     *      Then cast the instance with T type to get the type
     */
    template<typename T>
    static std::shared_ptr<T>   instanciate(std::string const& cType)
    {
        auto iterator = m_classList.begin();

        while (iterator != m_classList.end())
        {
            if ((*iterator)->getClassType().compare(cType) == 0)
            {
                auto qPointer = std::shared_ptr<QObject>((*iterator)->instanciate());

                return std::dynamic_pointer_cast<T>(qPointer);
            }
            ++iterator;
        }
        return std::shared_ptr<T>();
    }

private:
    /**
     *  @brief: this vector store all qml registered class
     */
    FWSERVICES_QT_API static std::vector<IQtQmlObject::sptr >	m_classList;
};

} // fwGuiQt
