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

#include "ProcessingTest.hpp"

#include <fwData/Histogram.hpp>
#include <fwData/Image.hpp>

#include <fwRuntime/EConfigurationElement.hpp>

#include <fwServices/IController.hpp>
#include <fwServices/macros.hpp>
#include <fwServices/op/Add.hpp>
#include <fwServices/registry/ActiveWorkers.hpp>
#include <fwServices/registry/ObjectService.hpp>

#include <fwTest/Exception.hpp>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( ::scene2D::ut::ProcessingTest );

namespace scene2D
{
namespace ut
{

static ::fwTest::Exception fwTestException(""); // force link with fwTest

//------------------------------------------------------------------------------

void ProcessingTest::setUp()
{
    // Set up context before running a test.
}

//------------------------------------------------------------------------------

void ProcessingTest::tearDown()
{
    // Clean up after the test run.
}

//------------------------------------------------------------------------------

void ProcessingTest::histogramTest()
{
    ::fwServices::registry::ActiveWorkers::sptr activeWorkers = ::fwServices::registry::ActiveWorkers::getDefault();
    activeWorkers->initRegistry();

    typedef signed short ImageType;
    const int sizeX     = 50;
    const int sizeY     = 50;
    const int sizeZ     = 50;
    const int imageSize = sizeX * sizeY * sizeZ;

    std::string implementation = "::scene2D::processing::SComputeHistogram";

    // Configure data hirearchy
    ::fwData::Image::sptr image         = ::fwData::Image::New();
    ::fwData::Histogram::sptr histogram = ::fwData::Histogram::New();

    // Create image.
    image->resize(sizeX, sizeY,  sizeZ, ::fwTools::Type::s_INT16, ::fwData::Image::GRAY_SCALE);

    const auto dumpLock = image->lock();
    auto itr            = image->begin< ImageType >();
    const auto itrEnd   = image->end< ImageType >();

    int count = 0;
    for(; itr != itrEnd; ++itr)
    {
        if(count < imageSize/4)
        {
            *itr = 10;
        }
        else if( count < imageSize/2)
        {
            *itr = 20;
        }
        else if( count < 3*imageSize/4)
        {
            *itr = 30;
        }
        else if( count < imageSize)
        {
            *itr = 40;
        }
        ++count;
    }

    auto srv = ::fwServices::add< ::fwServices::IController >(implementation, "");
    CPPUNIT_ASSERT_MESSAGE("Impossible to create the service '" + implementation + "'", srv);

    ::fwRuntime::EConfigurationElement::sptr binsWidthCfg = ::fwRuntime::EConfigurationElement::New("binsWidth");
    binsWidthCfg->setValue("1.0");

    srv->registerInput(image, "image");
    srv->registerInOut(histogram, "histogram");

    srv->setConfiguration(binsWidthCfg);
    srv->configure();
    srv->start().wait();
    srv->stop().wait();
    ::fwServices::OSR::unregisterService(srv);

    ::fwData::Histogram::fwHistogramValues values = histogram->getValues();
    CPPUNIT_ASSERT_EQUAL((size_t) 40-10+1, values.size());

    CPPUNIT_ASSERT_EQUAL((float) 10, histogram->getMinValue());

    CPPUNIT_ASSERT_EQUAL((float) 40, histogram->getMaxValue());

    CPPUNIT_ASSERT_EQUAL((long) imageSize/4, histogram->getNbPixels(10, 11));
    CPPUNIT_ASSERT_EQUAL((long) imageSize/4, histogram->getNbPixels(20, 21));
    CPPUNIT_ASSERT_EQUAL((long) imageSize/4, histogram->getNbPixels(30, 31));
    CPPUNIT_ASSERT_EQUAL((long) imageSize/4, histogram->getNbPixels(40, 41));

    activeWorkers->clearRegistry();
}

//------------------------------------------------------------------------------

} // namespace ut
} // namespace scene2D
