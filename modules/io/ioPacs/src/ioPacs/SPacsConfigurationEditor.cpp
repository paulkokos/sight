/************************************************************************
 *
 * Copyright (C) 2009-2020 IRCAD France
 * Copyright (C) 2012-2020 IHU Strasbourg
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

#include "ioPacs/SPacsConfigurationEditor.hpp"

#include <fwCom/Signal.hpp>
#include <fwCom/Signal.hxx>
#include <fwCom/Signals.hpp>

#include <fwGui/dialog/MessageDialog.hpp>

#include <fwGuiQt/container/QtContainer.hpp>

#include <fwPacsIO/exceptions/Base.hpp>

#include <fwServices/macros.hpp>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace ioPacs
{

static const ::fwServices::IService::KeyType s_CONFIG_INOUT = "config";

//------------------------------------------------------------------------------

SPacsConfigurationEditor::SPacsConfigurationEditor() noexcept
{
}
//------------------------------------------------------------------------------

SPacsConfigurationEditor::~SPacsConfigurationEditor() noexcept
{
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::configuring()
{
    ::fwGui::IGuiContainerSrv::initialize();
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::starting()
{
    ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    ::fwGui::IGuiContainerSrv::create();
    ::fwGuiQt::container::QtContainer::sptr qtContainer = fwGuiQt::container::QtContainer::dynamicCast(getContainer());

    QGridLayout* gridLayout = new QGridLayout();

    m_SCUAppEntityTitleEdit = new QLineEdit();
    m_SCUAppEntityTitleEdit->setText(pacsConfiguration->getLocalApplicationTitle().c_str());
    QLabel* const AETofSCU = new QLabel("AET of the SCU:");
    AETofSCU->setToolTip("Application entity title of the client");
    AETofSCU->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(AETofSCU, 0, 0);
    gridLayout->addWidget(m_SCUAppEntityTitleEdit, 0, 1);

    m_SCPAppEntityTitleEdit = new QLineEdit();
    m_SCPAppEntityTitleEdit->setText(pacsConfiguration->getPacsApplicationTitle().c_str());
    QLabel* const AETofSCP = new QLabel("AET of the SCP:");
    AETofSCP->setToolTip("Application entity title of the PACS server");
    AETofSCP->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(AETofSCP, 1, 0);
    gridLayout->addWidget(m_SCPAppEntityTitleEdit, 1, 1);

    m_SCPHostNameEdit = new QLineEdit();
    m_SCPHostNameEdit->setText(pacsConfiguration->getPacsHostName().c_str());
    QLabel* const hostNameOfSCP = new QLabel("Host name of the SCP:");
    hostNameOfSCP->setToolTip("Host name of the PACS server");
    hostNameOfSCP->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(hostNameOfSCP, 2, 0);
    gridLayout->addWidget(m_SCPHostNameEdit, 2, 1);

    m_SCPPortEdit = new QSpinBox();
    m_SCPPortEdit->setRange(0, 65535);
    m_SCPPortEdit->setValue(pacsConfiguration->getPacsApplicationPort());
    QLabel* const SCPPort = new QLabel("Port of the SCP");
    SCPPort->setToolTip("Port of the PACS server");
    SCPPort->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(SCPPort, 3, 0);
    gridLayout->addWidget(m_SCPPortEdit, 3, 1);

    m_moveAppEntityTitleEdit = new QLineEdit();
    m_moveAppEntityTitleEdit->setText(pacsConfiguration->getMoveApplicationTitle().c_str());
    QLabel* const AETOfMoveSCU = new QLabel("AET of the move SCU");
    AETOfMoveSCU->setToolTip("Application entity title of the move client");
    AETOfMoveSCU->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(AETOfMoveSCU, 4, 0);
    gridLayout->addWidget(m_moveAppEntityTitleEdit, 4, 1);

    m_movePort = new QSpinBox();
    m_movePort->setRange(0, 65535);
    m_movePort->setValue(pacsConfiguration->getMoveApplicationPort());
    QLabel* const PortOfMoveSCU = new QLabel("Port of the move SCU:");
    PortOfMoveSCU->setToolTip("Port of the move client");
    PortOfMoveSCU->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(PortOfMoveSCU, 5, 0);
    gridLayout->addWidget(m_movePort, 5, 1);

    m_retrieveMethodWidget = new QComboBox();
    m_retrieveMethodWidget->addItem("Move");
    m_retrieveMethodWidget->addItem("Get");
    m_retrieveMethodWidget->setCurrentIndex(
        (pacsConfiguration->getRetrieveMethod() == ::fwPacsIO::data::PacsConfiguration::MOVE_RETRIEVE_METHOD) ? 0 : 1);
    QLabel* const RetrieveMethod = new QLabel("Retrieve method:");
    RetrieveMethod->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(RetrieveMethod, 6, 0);
    gridLayout->addWidget(m_retrieveMethodWidget, 6, 1);

    m_pingPacsButtonWidget = new QPushButton("Ping PACS");
    gridLayout->addWidget(m_pingPacsButtonWidget, 7, 0, 1, 2);

    qtContainer->setLayout(gridLayout);

    // Connect signals.
    QObject::connect(m_pingPacsButtonWidget, SIGNAL(clicked()), this, SLOT(pingPACS()));
    QObject::connect(m_SCUAppEntityTitleEdit, SIGNAL(editingFinished()), this, SLOT(
                         onSCUAppEntityTitleChanged()));
    QObject::connect(m_SCPHostNameEdit, SIGNAL(editingFinished()), this, SLOT(onSCPHostNameChanged()));
    QObject::connect(m_SCPAppEntityTitleEdit, SIGNAL(editingFinished()), this,
                     SLOT(onSCPAppEntityTitleChanged()));
    QObject::connect(m_SCPPortEdit, SIGNAL(valueChanged(int)), this,
                     SLOT(onSCPPortChanged(int)));
    QObject::connect(m_moveAppEntityTitleEdit, SIGNAL(editingFinished()), this,
                     SLOT(onMoveAppEntityTitleChanged()));
    QObject::connect(m_movePort, SIGNAL(valueChanged(int)), this,
                     SLOT(onMovePortChanged(int)));
    QObject::connect(m_retrieveMethodWidget, SIGNAL(currentIndexChanged(int)), this,
                     SLOT(onRetrieveMethodChanged(int)));

}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::updating()
{
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::stopping()
{
    // Disconnect signals.
    QObject::disconnect(m_pingPacsButtonWidget, SIGNAL(clicked()), this, SLOT(pingPACS()));
    QObject::disconnect(m_SCUAppEntityTitleEdit, SIGNAL(editingFinished()), this,
                        SLOT(onSCUAppEntityTitleChanged()));
    QObject::disconnect(m_SCPHostNameEdit, SIGNAL(editingFinished()), this,
                        SLOT(onSCPHostNameChanged()));
    QObject::disconnect(m_SCPAppEntityTitleEdit, SIGNAL(editingFinished()), this,
                        SLOT(onSCPAppEntityTitleChanged()));
    QObject::disconnect(m_SCPPortEdit, SIGNAL(valueChanged(int)), this,
                        SLOT(onSCPPortChanged(int)));
    QObject::disconnect(m_moveAppEntityTitleEdit, SIGNAL(editingFinished()), this,
                        SLOT(onMoveAppEntityTitleChanged()));
    QObject::disconnect(m_movePort, SIGNAL(valueChanged(int)), this,
                        SLOT(onMovePortChanged(int)));
    QObject::disconnect(m_retrieveMethodWidget, SIGNAL(currentIndexChanged(int)), this,
                        SLOT(onRetrieveMethodChanged(int)));

    this->destroy();
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::pingPACS()
{
    ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(
        s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    ::fwPacsIO::SeriesEnquirer::sptr seriesEnquirer = ::fwPacsIO::SeriesEnquirer::New();

    bool success = false;
    try
    {
        seriesEnquirer->initialize(
            pacsConfiguration->getLocalApplicationTitle(),
            pacsConfiguration->getPacsHostName(),
            pacsConfiguration->getPacsApplicationPort(),
            pacsConfiguration->getPacsApplicationTitle());
        seriesEnquirer->connect();
        success = seriesEnquirer->pingPacs();
    }
    catch (::fwPacsIO::exceptions::Base& _e)
    {
        SLM_TRACE(_e.what());
        success = false;
    }

    if(seriesEnquirer->isConnectedToPacs())
    {
        seriesEnquirer->disconnect();
    }

    // Display a message with the ping result.
    ::fwGui::dialog::MessageDialog messageBox;
    messageBox.setTitle("Ping Pacs");
    if(success)
    {
        messageBox.setMessage( "Ping succeed !" );
    }
    else
    {
        messageBox.setMessage( "Ping failed !" );
    }
    messageBox.setIcon(::fwGui::dialog::IMessageDialog::INFO);
    messageBox.addButton(::fwGui::dialog::IMessageDialog::OK);
    messageBox.show();

}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::modifiedNotify(::fwPacsIO::data::PacsConfiguration::sptr _pacsConfiguration)
{
    auto sig = _pacsConfiguration->signal< ::fwData::Object::ModifiedSignalType >(::fwData::Object::s_MODIFIED_SIG);
    {
        ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onSCUAppEntityTitleChanged()
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    pacsConfiguration->setLocalApplicationTitle(m_SCUAppEntityTitleEdit->text().toStdString());

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onSCPHostNameChanged()
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);
    pacsConfiguration->setPacsHostName(m_SCPHostNameEdit->text().toStdString());

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onSCPAppEntityTitleChanged()
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration =
        this->getInOut< ::fwPacsIO::data::PacsConfiguration >(
            s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    pacsConfiguration->setPacsApplicationTitle(m_SCPAppEntityTitleEdit->text().toStdString());

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onSCPPortChanged(int value)
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    pacsConfiguration->setPacsApplicationPort(static_cast<unsigned short>(value));

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onMoveAppEntityTitleChanged()
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    pacsConfiguration->setMoveApplicationTitle(m_moveAppEntityTitleEdit->text().toStdString());

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onMovePortChanged(int _value)
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("The inout key '" + s_CONFIG_INOUT + "' is not correctly set.", pacsConfiguration);

    pacsConfiguration->setMoveApplicationPort(static_cast<unsigned short>(_value));

    modifiedNotify(pacsConfiguration);
}

//------------------------------------------------------------------------------

void SPacsConfigurationEditor::onRetrieveMethodChanged(int _index)
{
    const ::fwPacsIO::data::PacsConfiguration::sptr pacsConfiguration
        = this->getInOut< ::fwPacsIO::data::PacsConfiguration >(s_CONFIG_INOUT);
    SLM_ASSERT("input '" + s_CONFIG_INOUT +"' does not exist.", pacsConfiguration);

    pacsConfiguration->setRetrieveMethod(
        (_index ==
         0) ? (::fwPacsIO::data::PacsConfiguration::MOVE_RETRIEVE_METHOD): (::fwPacsIO::data::PacsConfiguration::
                                                                            GET_RETRIEVE_METHOD));

    modifiedNotify(pacsConfiguration);
}

} // namespace ioPacs
