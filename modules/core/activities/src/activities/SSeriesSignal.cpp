/************************************************************************
 *
 * Copyright (C) 2016-2020 IRCAD France
 * Copyright (C) 2016-2020 IHU Strasbourg
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

#include "activities/SSeriesSignal.hpp"

#include <fwCom/Signal.hpp>
#include <fwCom/Signal.hxx>
#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>
#include <fwCom/Slots.hxx>

#include <fwData/mt/ObjectReadLock.hpp>

#include <fwMedData/ActivitySeries.hpp>

#include <fwRuntime/Convert.hpp>
#include <fwRuntime/operations.hpp>

#include <fwServices/macros.hpp>

#include <boost/foreach.hpp>

namespace activities
{

//------------------------------------------------------------------------------

fwServicesRegisterMacro( ::fwServices::IController, ::activities::SSeriesSignal, ::fwMedData::SeriesDB )

//------------------------------------------------------------------------------

const ::fwCom::Slots::SlotKeyType SSeriesSignal::s_REPORT_SERIES_SLOT = "reportSeries";

const ::fwCom::Signals::SignalKeyType SSeriesSignal::s_SERIES_ADDED_SIG = "seriesAdded";

static const std::string s_SERIES_DB_INPUT = "seriesDB";

//------------------------------------------------------------------------------

SSeriesSignal::SSeriesSignal() noexcept
{
    m_sigSeriesAdded = newSignal< SeriesAddedSignalType >(s_SERIES_ADDED_SIG);

    newSlot(s_REPORT_SERIES_SLOT, &SSeriesSignal::reportSeries, this);
}

//------------------------------------------------------------------------------

SSeriesSignal::~SSeriesSignal() noexcept
{
}

//------------------------------------------------------------------------------

void SSeriesSignal::starting()
{
}

//------------------------------------------------------------------------------

void SSeriesSignal::stopping()
{
}

//------------------------------------------------------------------------------

void SSeriesSignal::configuring()
{

    const ::fwServices::IService::ConfigType srvconfig = this->getConfigTree();

    if(srvconfig.count("filter") == 1 )
    {
        const ::fwServices::IService::ConfigType& configFilter = srvconfig.get_child("filter");
        SLM_ASSERT("A maximum of 1 <mode> tag is allowed", configFilter.count("mode") < 2);

        const std::string mode = configFilter.get< std::string >("mode");
        SLM_ASSERT("'" + mode + "' value for <mode> tag isn't valid. Allowed values are : 'include', 'exclude'.",
                   mode == "include" || mode == "exclude");
        m_filterMode = mode;

        BOOST_FOREACH( const ConfigType::value_type& v,  configFilter.equal_range("type") )
        {
            m_types.push_back(v.second.get<std::string>(""));
        }
    }
    SLM_ASSERT("A maximum of 1 <filter> tag is allowed", srvconfig.count("filter") < 2);
}

//------------------------------------------------------------------------------

void SSeriesSignal::reportSeries(::fwMedData::SeriesDB::ContainerType addedSeries)
{
    for(const ::fwMedData::Series::sptr& series : addedSeries)
    {
        const bool isIncludeMode = m_filterMode == "include";

        std::string classname     = series->getClassname();
        TypesType::iterator keyIt = std::find(m_types.begin(), m_types.end(), classname);

        if(keyIt != m_types.end() && isIncludeMode)
        {
            m_sigSeriesAdded->asyncEmit(series);
        }
        else if(keyIt == m_types.end() && !isIncludeMode)
        {
            m_sigSeriesAdded->asyncEmit(series);
        }
    }
}

//------------------------------------------------------------------------------

void SSeriesSignal::updating()
{
    const ::fwMedData::SeriesDB::csptr seriesDB = this->getInput< ::fwMedData::SeriesDB >(s_SERIES_DB_INPUT);
    SLM_ASSERT("input '" + s_SERIES_DB_INPUT + "' does not exist.", seriesDB);
    ::fwData::mt::ObjectReadLock lock(seriesDB);

    this->reportSeries(seriesDB->getContainer());
}

//------------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsMap SSeriesSignal::getAutoConnections() const
{
    KeyConnectionsMap connections;
    connections.push(s_SERIES_DB_INPUT,  ::fwMedData::SeriesDB::s_ADDED_SERIES_SIG, s_REPORT_SERIES_SLOT);
    connections.push(s_SERIES_DB_INPUT,  ::fwMedData::SeriesDB::s_MODIFIED_SIG, s_UPDATE_SLOT);
    return connections;
}

//------------------------------------------------------------------------------

} // namespace activities
