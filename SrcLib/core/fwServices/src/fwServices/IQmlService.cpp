#include "fwServices/IQmlService.hpp"
#include "fwServices/registry/ActiveWorkers.hpp"

namespace fwServices
{

IQmlService::IQmlService() : m_serviceState(STOPPED)
{
    m_associatedWorker = ::fwThread::Worker::New();
}

IQmlService::~IQmlService()
{
}

void	IQmlService::start()
{
	m_serviceState = STARTED;
    this->starting();
    started();
}

void	IQmlService::update()
{
    this->updating();
    updated();
}

void    IQmlService::configure()
{
    if (m_configurationState == NOT_CONFIGURED)
    {
        try {
            this->configuring();
        }
        catch (std::exception& e) {
            SLM_ERROR(std::string("Error while configuring service: '") + e.what());
        }
    }
    else {
        this->reconfiguring();
    }
}

void	IQmlService::stop()
{
	SLM_ASSERT("Service isn't running", m_serviceState == STARTED);
	m_serviceState = STOPPED;
    this->stopping();
    stopped();
}

void	IQmlService::destroy()
{
	m_serviceState = DESTROYED;
    this->destroying();
    destroyed();
}

IQmlService::ServiceState 	IQmlService::getStatus() const
{
	return m_serviceState;
}

void    IQmlService::starting()
{
}

void    IQmlService::stopping()
{
}

void    IQmlService::updating()
{
}

void    IQmlService::configuring()
{
}

void    IQmlService::reconfiguring()
{
}

void    IQmlService::destroying()
{
}

void    IQmlService::setProperty(const std::string& name, const QVariant& value)
{
    QObject::setProperty(name.c_str(), value);
}

} // fwServices