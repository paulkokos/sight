/************************************************************************
 *
 * Copyright (C) 2018-2019 IRCAD France
 * Copyright (C) 2018-2019 IHU Strasbourg
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

#include "ioDicomWeb/config.hpp"

#include <fwGui/editor/IEditor.hpp>

#include <fwMedData/SeriesDB.hpp>

#include <fwNetworkIO/http/ClientQt.hpp>

#include <filesystem>
#include <QDateEdit>
#include <QLineEdit>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QWidget>

namespace ioDicomWeb
{

/**
 * @brief   This editor service is used to perform an HTTP query on a Pacs.
 *
 * @section Slots Slots
 * - \b displayErrorMessage(const std::string&) : display an error message.

 * @section XML XML Configuration
 *
 * @code{.xml}
        <service type="::ioDicomWeb::SQueryEditor">
            <inout key="seriesDB" uid="..." />
            <server>%PACS_SERVER_HOSTNAME%:%PACS_SERVER_PORT%</server>
       </service>
   @endcode
 * @subsection In-Out In-Out:
 * - \b seriesDB [::fwMedData::SeriesDB]: SeriesDB on which the queried data will be pushed.
 * @subsection Configuration Configuration:
 * - \b server: server URL. Need hostname and port in this format addr:port (default value is 127.0.0.1:8042).
 * @note : hostname and port of this service can be a value or a nameKey from preference settings
 *  (for example <server>%HOSTNAME%:%PORT%</server>)

 */
class IODICOMWEB_CLASS_API SQueryEditor : public QObject,
                                          public ::fwGui::editor::IEditor
{
Q_OBJECT;

public:

    fwCoreServiceMacro(SQueryEditor,  ::fwGui::editor::IEditor );

    /// Constructor
    IODICOMWEB_API SQueryEditor() noexcept;

    /// Destructor
    IODICOMWEB_API virtual ~SQueryEditor() noexcept;

protected:

    /// Gets the configurations.
    IODICOMWEB_API virtual void configuring() override;

    /// Creates the widgets & connect the signals.
    IODICOMWEB_API virtual void starting() override;

    /// Disconnect the signals and destroy the widgets.
    IODICOMWEB_API virtual void stopping() override;

    /// Does nothing.
    IODICOMWEB_API void updating() override;

private Q_SLOTS:
    /// Slot called when querying on patient name
    void queryPatientName();

    /// Slot called when querying on study date
    void queryStudyDate();

private:
    /**
     * @brief Display an error message
     * @param[in] message Error message to display
     */
    void displayErrorMessage(const std::string& message) const;

    /**
     * @brief Update the seriesDB with the series retrieved from the pacs
     * @param[in] series Series which must be added to the SeriesDB
     */
    void updateSeriesDB(::fwMedData::SeriesDB::ContainerType series);

    /// Patient Name Field
    QPointer< QLineEdit > m_patientNameLineEdit;

    /// Patient Name Query Button
    QPointer< QPushButton > m_patientNameQueryButton;

    /// Begin study date widget
    QPointer< QDateEdit > m_beginStudyDateEdit;

    /// End study date widget
    QPointer< QDateEdit > m_endStudyDateEdit;

    /// Study Date Query Button
    QPointer< QPushButton > m_studyDateQueryButton;

    /// Http Qt Client
    ::fwNetworkIO::http::ClientQt m_clientQt;

    /// Server hostname preference key
    std::string m_serverHostnameKey;

    /// Server port preference key
    std::string m_serverPortKey;

    /// Server hostname
    std::string m_serverHostname{"localhost"};

    /// Server port
    int m_serverPort{4242};

};

} // namespace ioDicomWeb
