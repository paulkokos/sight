/************************************************************************
 *
 * Copyright (C) 2019-2020 IRCAD France
 * Copyright (C) 2019-2020 IHU Strasbourg
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

#include "uiReconstructionQt/SOrganMaterialEditor.hpp"

#include <fwCore/base.hpp>

#include <fwData/Material.hpp>
#include <fwData/Mesh.hpp>
#include <fwData/mt/ObjectReadLock.hpp>
#include <fwData/mt/ObjectWriteLock.hpp>
#include <fwData/Reconstruction.hpp>

#include <fwGuiQt/container/QtContainer.hpp>

#include <fwRuntime/ConfigurationElement.hpp>
#include <fwRuntime/operations.hpp>

#include <fwServices/IService.hpp>
#include <fwServices/macros.hpp>
#include <fwServices/op/Get.hpp>

#include <QColor>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>

namespace uiReconstructionQt
{

fwServicesRegisterMacro(::fwGui::editor::IEditor, ::uiReconstructionQt::SOrganMaterialEditor, ::fwData::Reconstruction )

static const ::fwServices::IService::KeyType s_RECONSTRUCTION_INOUT = "reconstruction";

SOrganMaterialEditor::SOrganMaterialEditor() noexcept
{
    this->registerObject(s_RECONSTRUCTION_INOUT, ::fwServices::IService::AccessType::INOUT, true);
}

//------------------------------------------------------------------------------

SOrganMaterialEditor::~SOrganMaterialEditor() noexcept
{
}

//------------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsMap SOrganMaterialEditor::getAutoConnections() const
{
    KeyConnectionsMap connections;
    connections.push(s_RECONSTRUCTION_INOUT, ::fwData::Object::s_MODIFIED_SIG, s_UPDATE_SLOT);
    return connections;
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::configuring()
{
    this->initialize();
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::starting()
{
    this->create();
    ::fwGuiQt::container::QtContainer::sptr qtContainer
        = ::fwGuiQt::container::QtContainer::dynamicCast(this->getContainer() );

    m_diffuseColourButton = new QPushButton(tr("Diffuse"));
    m_diffuseColourButton->setToolTip(tr("Selected organ's diffuse color"));
    m_diffuseColourButton->setMinimumSize(m_diffuseColourButton->sizeHint());

    m_ambientColourButton = new QPushButton(tr("Ambient"));
    m_ambientColourButton->setToolTip(tr("Selected organ's ambient color"));
    m_ambientColourButton->setMinimumSize(m_ambientColourButton->sizeHint());

    QLabel* const transparencyLabel = new QLabel(tr("Transparency : "));
    m_opacitySlider = new QSlider( Qt::Horizontal);
    m_opacitySlider->setToolTip(tr("Selected organ's opacity"));
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setTickInterval(20);
    m_opacitySlider->setTickPosition(QSlider::TicksBelow);
    m_opacitySlider->setMinimumSize(m_opacitySlider->sizeHint());

    m_transparencyValue = new QLabel("");
    m_transparencyValue->setMinimumSize(m_transparencyValue->sizeHint());

    QVBoxLayout* const mainLayout = new QVBoxLayout();

    QHBoxLayout* const buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( m_diffuseColourButton, 0 );
    buttonLayout->addWidget( m_ambientColourButton, 0 );
    mainLayout->addLayout( buttonLayout, 0);

    QHBoxLayout* const transparencyLayout = new QHBoxLayout( );
    transparencyLayout->addWidget( transparencyLabel, 0);
    transparencyLayout->addWidget( m_opacitySlider, 1 );
    transparencyLayout->addWidget( m_transparencyValue, 0);
    mainLayout->addLayout( transparencyLayout, 0);

    qtContainer->setLayout( mainLayout );
    qtContainer->setEnabled(false);

    QObject::connect(m_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(onOpacitySlider(int)));
    QObject::connect(m_diffuseColourButton, SIGNAL(clicked()), this, SLOT(onDiffuseColorButton()));
    QObject::connect(m_ambientColourButton, SIGNAL(clicked()), this, SLOT(onAmbientColorButton()));

    this->updating();
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::updating()
{
    this->refreshMaterial();
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::stopping()
{
    QObject::disconnect(m_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(onOpacitySlider(int)));
    QObject::disconnect(m_diffuseColourButton, SIGNAL(clicked()), this, SLOT(onDiffuseColorButton()));
    QObject::disconnect(m_ambientColourButton, SIGNAL(clicked()), this, SLOT(onAmbientColorButton()));

    this->destroy();
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::onDiffuseColorButton()
{
    ::fwData::Reconstruction::sptr reconstruction = this->getInOut< ::fwData::Reconstruction >(s_RECONSTRUCTION_INOUT);
    SLM_ASSERT("inout '" + s_RECONSTRUCTION_INOUT + "' does not exist.", reconstruction);

    ::fwData::Material::sptr material = reconstruction->getMaterial();
    ::fwData::mt::ObjectWriteLock lock(material);

    int red   = static_cast<int>(material->diffuse()->red()*255);
    int green = static_cast<int>(material->diffuse()->green()*255);
    int blue  = static_cast<int>(material->diffuse()->blue()*255);

    // Create Color choice dialog.
    ::fwGuiQt::container::QtContainer::sptr qtContainer
        = ::fwGuiQt::container::QtContainer::dynamicCast(this->getContainer() );
    QWidget* const container = qtContainer->getQtContainer();
    SLM_ASSERT("container not instanced", container);

    const QColor oldColor(red, green, blue);
    const QColor color = QColorDialog::getColor(oldColor, container);
    if(color.isValid())
    {
        material->diffuse()->red()   = static_cast<float>(color.redF());
        material->diffuse()->green() = static_cast<float>(color.greenF());
        material->diffuse()->blue()  = static_cast<float>(color.blueF());
        this->materialNotification();
        lock.unlock();
        refreshMaterial();
    }
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::onAmbientColorButton()
{
    ::fwData::Reconstruction::sptr reconstruction = this->getInOut< ::fwData::Reconstruction >(s_RECONSTRUCTION_INOUT);
    SLM_ASSERT("inout '" + s_RECONSTRUCTION_INOUT + "' does not exist.", reconstruction);

    ::fwData::Material::sptr material = reconstruction->getMaterial();
    ::fwData::mt::ObjectWriteLock lock(material);

    const int red   = static_cast<int>(material->ambient()->red()*255.f);
    const int green = static_cast<int>(material->ambient()->green()*255.f);
    const int blue  = static_cast<int>(material->ambient()->blue()*255.f);

    // Create Color choice dialog.
    ::fwGuiQt::container::QtContainer::sptr qtContainer
        = ::fwGuiQt::container::QtContainer::dynamicCast(this->getContainer() );
    QWidget* const container = qtContainer->getQtContainer();
    SLM_ASSERT("container not instanced", container);

    const QColor oldColor(red, green, blue);
    const QColor color = QColorDialog::getColor(oldColor, container);
    if(color.isValid())
    {
        material->ambient()->red()   = static_cast<float>(color.redF());
        material->ambient()->green() = static_cast<float>(color.greenF());
        material->ambient()->blue()  = static_cast<float>(color.blueF());
        this->materialNotification();
        lock.unlock();
        refreshMaterial();
    }
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::onOpacitySlider(int _value)
{
    ::fwData::Reconstruction::sptr reconstruction = this->getInOut< ::fwData::Reconstruction >(s_RECONSTRUCTION_INOUT);
    SLM_ASSERT("inout '" + s_RECONSTRUCTION_INOUT + "' does not exist.", reconstruction);

    ::fwData::Material::sptr material = reconstruction->getMaterial();
    ::fwData::mt::ObjectWriteLock lock(material);

    material->diffuse()->alpha() = static_cast<float>(_value)/100.f;
    std::stringstream ss;
    ss << _value << "%";
    m_transparencyValue->setText(QString::fromStdString(ss.str()));

    this->materialNotification();
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::refreshMaterial()
{
    ::fwData::Reconstruction::csptr reconstruction = this->getInOut< ::fwData::Reconstruction >(s_RECONSTRUCTION_INOUT);
    SLM_ASSERT("inout '" + s_RECONSTRUCTION_INOUT + "' does not exist.", reconstruction);

    ::fwGuiQt::container::QtContainer::sptr qtContainer = ::fwGuiQt::container::QtContainer::dynamicCast(
        this->getContainer() );
    QWidget* const container = qtContainer->getQtContainer();
    SLM_ASSERT("container not instanced", container);

    container->setEnabled(!reconstruction->getOrganName().empty());

    ::fwData::Material::csptr material = reconstruction->getMaterial();
    ::fwData::mt::ObjectReadLock lock(material);

    {
        const QColor materialDiffuseColor = QColor(
            static_cast<int>(material->diffuse()->red()*255.f),
            static_cast<int>(material->diffuse()->green()*255.f),
            static_cast<int>(material->diffuse()->blue()*255.f),
            static_cast<int>(material->diffuse()->alpha()*255.f)
            );

        const int iconSize = m_diffuseColourButton->style()->pixelMetric(QStyle::PM_LargeIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(materialDiffuseColor);
        m_diffuseColourButton->setIcon(QIcon(pix));
    }

    {
        const QColor materialAmbientColor = QColor(
            static_cast<int>(material->ambient()->red()*255.f),
            static_cast<int>(material->ambient()->green()*255.f),
            static_cast<int>(material->ambient()->blue()*255.f),
            static_cast<int>(material->ambient()->alpha()*255.f)
            );

        const int iconSize = m_ambientColourButton->style()->pixelMetric(QStyle::PM_LargeIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(materialAmbientColor);
        m_ambientColourButton->setIcon(QIcon(pix));
    }

    const int a = static_cast<int>(material->diffuse()->alpha()*100.f);
    lock.unlock();
    m_opacitySlider->setValue( a );
    std::stringstream ss;
    ss << a << "%";
    m_transparencyValue->setText(QString::fromStdString(ss.str()));
}

//------------------------------------------------------------------------------

void SOrganMaterialEditor::materialNotification()
{
    ::fwData::Reconstruction::csptr reconstruction = this->getInOut< ::fwData::Reconstruction >(s_RECONSTRUCTION_INOUT);
    SLM_ASSERT("inout '" + s_RECONSTRUCTION_INOUT + "' does not exist.", reconstruction);

    ::fwData::Object::ModifiedSignalType::sptr sig
        = reconstruction->getMaterial()->signal< ::fwData::Object::ModifiedSignalType >(
              ::fwData::Object::s_MODIFIED_SIG);
    sig->asyncEmit();
}

//------------------------------------------------------------------------------

}
