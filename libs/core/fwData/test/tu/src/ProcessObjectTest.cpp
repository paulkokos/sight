/************************************************************************
 *
 * Copyright (C) 2009-2015 IRCAD France
 * Copyright (C) 2012-2015 IHU Strasbourg
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

#include <fwData/Image.hpp>
#include <fwData/Integer.hpp>
#include <fwData/ProcessObject.hpp>
#include "ProcessObjectTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( ::fwData::ut::ProcessObjectTest );

namespace fwData
{
namespace ut
{

void ProcessObjectTest::constructeur()
{
    const std::string IMAGEID1 = "myImage1";
    const std::string IMAGEID2 = "myImage2";
    const std::string FIELDID1 = "myField1";
    const std::string FIELDID2 = "myField2";
    ::fwData::Image::sptr image1   = ::fwData::Image::New();
    ::fwData::Image::sptr image2   = ::fwData::Image::New();
    ::fwData::Integer::sptr field1 = ::fwData::Integer::New(3);
    ::fwData::Integer::sptr field2 = ::fwData::Integer::New(8);

    // process
    ::fwData::ProcessObject::sptr po = ::fwData::ProcessObject::New();
    po->setInputValue(IMAGEID1, image1);
    po->setInputValue(FIELDID1, field1);
    po->setInputValue(FIELDID2, field2);
    po->setOutputValue(IMAGEID2, image2);

    // check
    CPPUNIT_ASSERT_EQUAL(image1, po->getInput< ::fwData::Image >(IMAGEID1));
    CPPUNIT_ASSERT_EQUAL(field1, po->getInput< ::fwData::Integer >(FIELDID1));
    CPPUNIT_ASSERT_EQUAL(field2, po->getInput< ::fwData::Integer >(FIELDID2));
    CPPUNIT_ASSERT_EQUAL(image2, po->getOutput< ::fwData::Image >(IMAGEID2));
}

} //namespace ut
} //namespace fwData
