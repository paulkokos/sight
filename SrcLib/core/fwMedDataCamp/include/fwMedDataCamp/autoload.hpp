/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2009-2016.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __FWMEDDATACAMP_AUTOLOAD_HPP__
#define __FWMEDDATACAMP_AUTOLOAD_HPP__

#include "fwMedDataCamp/ActivitySeries.hpp"
#include "fwMedDataCamp/AttachmentSeries.hpp"
#include "fwMedDataCamp/DicomSeries.hpp"
#include "fwMedDataCamp/ImageSeries.hpp"
#include "fwMedDataCamp/Series.hpp"

#include <fwCamp/macros.hpp>

#include <fwMedData/ActivitySeries.hpp>
#include <fwMedData/DicomSeries.hpp>
#include <fwMedData/Equipment.hpp>
#include <fwMedData/ImageSeries.hpp>
#include <fwMedData/ModelSeries.hpp>
#include <fwMedData/NavigationSeries.hpp>
#include <fwMedData/Patient.hpp>
#include <fwMedData/Series.hpp>
#include <fwMedData/SeriesDB.hpp>
#include <fwMedData/Study.hpp>

namespace fwMedDataCamp
{

struct runner
{
    runner()
    {
        localDeclarefwMedDataEquipment();
        localDeclarefwMedDataStudy();
        localDeclarefwMedDataPatient();
        localDeclarefwMedDataSeries();
        localDeclarefwMedDataSeriesDB();
        localDeclarefwMedDataImageSeries();
        localDeclarefwMedDataModelSeries();
        localDeclarefwMedDataActivitySeries();
        localDeclarefwMedDataDicomSeriesDICOM_AVAILABILITY();
        localDeclarefwMedDataDicomSeries();
        localDeclarefwMedDataAttachmentSeriesAttachmentAvailability();
        localDeclarefwMedDataAttachmentSeries();
        localDeclarefwMedDataNavigationSeries();
    }

    static runner r;
};

} //end namespace fwMedDataCamp

#endif //__FWMEDDATACAMP_AUTOLOAD_HPP__
