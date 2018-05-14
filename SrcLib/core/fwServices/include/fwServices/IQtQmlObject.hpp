#pragma once

#include "fwServices/config.hpp"

#include <QObject>

namespace fwServices
{

class FWSERVICES_CLASS_API IQtQmlObject
{
public:
	IQtQmlObject(std::string const& cType);
	virtual ~IQtQmlObject();

	/**
	 *	@brief: This method create a new object derived of QObject
	 */
	virtual QObject *instanciate() const = 0;

	/**
	 *	@brief: This method return the classType
	 */
	std::string const&	getClassType() const;

private:
	std::string	m_cType;
};

} // fwGuiQt