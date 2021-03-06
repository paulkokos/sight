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

#define FUSION_MAX_VECTOR_SIZE 20

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#ifdef DEBUG
static std::stringstream spiritDebugStream;
  #define BOOST_SPIRIT_DEBUG_OUT spiritDebugStream
  #define BOOST_SPIRIT_DEBUG
#endif

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/detail/case_conv.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/container/vector/vector.hpp>
#include <boost/fusion/include/vector.hpp>

#include <fwCore/exceptionmacros.hpp>

#include <fwData/Color.hpp>
#include <fwData/StructureTraitsDictionary.hpp>
#include <fwData/StructureTraits.hpp>
#include <fwData/StructureTraitsHelper.hpp>

#include <fwRuntime/operations.hpp>

#include "fwDataIO/reader/DictionaryReader.hpp"
#include "fwDataIO/reader/registry/macros.hpp"

fwDataIOReaderRegisterMacro( ::fwDataIO::reader::DictionaryReader );

namespace fwDataIO
{

struct line
{
    std::string type;
    double red;
    double green;
    double blue;
    double alpha;
    std::string catgegory;
    std::string organClass;
    std::string attachment;
    std::string nativeExp;
    std::string nativeExpGeo;
    std::string anatomicRegion;
    std::string propertyCategory;
    std::string propertyType;
};
}

BOOST_FUSION_ADAPT_STRUCT(
    ::fwDataIO::line,
    (std::string, type)
        (double, red)
        (double, green)
        (double, blue)
        (double, alpha)
        (std::string, catgegory)
        (std::string, organClass)
        (std::string, attachment)
        (std::string, nativeExp)
        (std::string, nativeExpGeo)
        (std::string, anatomicRegion)
        (std::string, propertyCategory)
        (std::string, propertyType)
    )

//------------------------------------------------------------------------------

inline std::string trim ( std::string& s )
{
    return ::boost::algorithm::trim_copy(s);
}

//------------------------------------------------------------------------------

/// Reformat string in the following way :first letter is uppercase and the rest is lowercase).
std::string  reformatString(std::string& expr)
{
    std::string trimStr = ::boost::algorithm::trim_copy(expr);
    std::string result  = ::boost::algorithm::to_upper_copy(trimStr.substr(0, 1))
                          + ::boost::algorithm::to_lower_copy(trimStr.substr(1));
    return (result);
}

//------------------------------------------------------------------------------
/// Return the list of availabe value for the key of the map m.

template< typename MapType >
std::string getValues(const MapType& m)
{
    std::stringstream str;
    typedef typename MapType::const_iterator const_iterator;
    const_iterator iter = m.begin();
    str << "( " << iter->first;
    for(; iter != m.end(); ++iter )
    {
        str << ", " << iter->first;
    }
    str << ") ";
    return str.str();
}
//------------------------------------------------------------------------------

namespace fwDataIO
{

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct line_parser : qi::grammar<Iterator, std::vector <line>() >
{
    line_parser() :
        line_parser::base_type(lines)
    {
        using qi::int_;
        using qi::lit;
        using qi::_1;
        using qi::double_;
        using qi::blank;
        using qi::alnum;
        using qi::omit;
        using ascii::char_;
        using boost::spirit::qi::eol;
        using boost::spirit::qi::eoi;
        using boost::phoenix::construct;
        namespace phx = boost::phoenix;

        error.clear();

        lines   = +( line[phx::push_back(qi::_val, qi::_1)] | comment ) >> eoi;
        comment = *blank >> lit('#') >> *(char_- eol)>> +qi::eol;

        line = trimmedString >> lit(';')
               >> omit[*blank]>> lit('(')
               >>  dbl   >> lit(',')
               >>  dbl   >> lit(',')
               >>  dbl   >> lit(',')
               >>  dbl
               >> lit(')') >> omit[*blank]
               >> lit(';')
               >>  stringSet >> lit(';')
               >>  trimmedString >> lit(';')
               >>  trimmedString >> lit(';')
               >>  trimmedStringExpr >> lit(';')
               >>  trimmedStringExpr >> lit(';')
               >>  trimmedString >> lit(';')
               >>  trimmedString >> lit(';')
               >>  trimmedString
               >> +qi::eol;

        trimmedString = str[qi::_val = phx::bind(trim, qi::_1)];
        str           = *( (alnum|char_("_"))[qi::_val += qi::_1] | blank[qi::_val += " "]);

        trimmedStringExpr = stringExpr[qi::_val = phx::bind(trim, qi::_1)];
        stringExpr        = *( (alnum|char_("()_,.+-"))[qi::_val += qi::_1] | blank[qi::_val += " "] );

        stringSet       = stringWithComma[qi::_val = phx::bind(trim, qi::_1)];
        stringWithComma = *( (alnum| char_(",_"))[qi::_val += qi::_1] | blank[qi::_val += " "] );

        dbl = omit[*blank] >> double_ >> omit[*blank];

    #ifdef BOOST_SPIRIT_DEBUG
        BOOST_SPIRIT_DEBUG_NODE(comment);
        BOOST_SPIRIT_DEBUG_NODE(trimmedString);
        BOOST_SPIRIT_DEBUG_NODE(trimmedStringExpr);
        BOOST_SPIRIT_DEBUG_NODE(stringSet);
        BOOST_SPIRIT_DEBUG_NODE(dbl);
        BOOST_SPIRIT_DEBUG_NODE(line);
        BOOST_SPIRIT_DEBUG_NODE(lines);
        SLM_DEBUG(spiritDebugStream.str());
        spiritDebugStream.str( std::string() );
    #endif

        qi::on_error< qi::fail>
        (
            line
            , phx::ref( (std::ostream&)error )
                << phx::val("Error! Expecting ")
                << qi::_4                          // what failed?
                << phx::val(" here: \"")
                << phx::construct<std::string>(qi::_3, qi::_2)       // iterators to error-pos, end
                << phx::val("\"")
                << std::endl
        );

    }

    qi::rule<Iterator, double()> dbl;
    qi::rule<Iterator, std::string()> str;
    qi::rule<Iterator, std::string()> stringExpr;
    qi::rule<Iterator, std::string()> stringWithComma;

    qi::rule<Iterator, std::string()> comment;
    qi::rule<Iterator, std::string()> trimmedStringExpr;
    qi::rule<Iterator, std::string()> trimmedString;
    qi::rule<Iterator, std::string()> stringSet;

    qi::rule<Iterator, ::fwDataIO::line()> line;
    qi::rule<Iterator, std::vector< ::fwDataIO::line >() > lines;
    std::stringstream error;
};

namespace reader
{
//------------------------------------------------------------------------------

std::pair<bool, std::string> parse(std::string& buf, std::vector<fwDataIO::line>& lines)
{
    using boost::spirit::ascii::space;
    using boost::spirit::ascii::blank;
    using boost::spirit::qi::eol;

    using boost::spirit::qi::phrase_parse;

    typedef std::string::const_iterator iterator_type;
    typedef ::fwDataIO::line_parser<iterator_type> line_parser;

    iterator_type iter = buf.begin();
    iterator_type end  = buf.end();

    line_parser grammar; // Our grammar

    bool result     = phrase_parse(iter, end, grammar,  space - blank - eol, lines);
    bool success    = result && (iter == end);
    std::string msg = grammar.error.str();
    return std::make_pair( success, msg );
}

//------------------------------------------------------------------------------

DictionaryReader::DictionaryReader(::fwDataIO::reader::IObjectReader::Key) :
    ::fwData::location::enableSingleFile< IObjectReader >(this)
{
}

//------------------------------------------------------------------------------

DictionaryReader::~DictionaryReader()
{
}

//------------------------------------------------------------------------------

void DictionaryReader::read()
{
    SLM_TRACE_FUNC();
    assert( std::dynamic_pointer_cast< ::fwData::location::SingleFile >(m_location) );
    std::filesystem::path path = std::dynamic_pointer_cast< ::fwData::location::SingleFile >(m_location)->getPath();

    OSLM_INFO( "[DictionaryReader::read] dictionary file: " << path.string());
    SLM_ASSERT("Empty path for dictionary file", !path.empty());

    // Reading of the file
    std::streamsize length;
    std::string buf;
    std::ifstream file;
    file.open(path.string().c_str(), std::ios::binary );

    std::string errorOpen = "Unable to open " + path.string();
    FW_RAISE_IF(errorOpen, !file.is_open());

    file.seekg(0, std::ios::end);
    length = file.tellg();
    file.seekg(0, std::ios::beg);

    buf.resize(static_cast<size_t>(length));
    char* buffer = &buf[0];

    file.read(buffer, length);
    file.close();

    std::vector < ::fwDataIO::line > dicolines;
    std::pair<bool, std::string> result = parse(buf, dicolines);

    std::string error = "Unable to parse " + path.string() + " : Bad file format.Error : " + result.second;
    FW_RAISE_IF(error, !result.first);

    // File the dictionary Structure
    ::fwData::StructureTraitsDictionary::sptr structDico = getConcreteObject();

    for(::fwDataIO::line line :  dicolines)
    {
        ::fwData::StructureTraits::sptr newOrgan = ::fwData::StructureTraits::New();
        newOrgan->setType(line.type);

        std::string classReformated = reformatString(line.organClass);
        ::fwData::StructureTraitsHelper::ClassTranslatorType::right_const_iterator strClassIter =
            ::fwData::StructureTraitsHelper::s_CLASSTRANSLATOR.right.find(classReformated);
        std::string availableValues = getValues(::fwData::StructureTraitsHelper::s_CLASSTRANSLATOR.right);
        error = "Organ class " + classReformated + " isn't available. Authorized type are " + availableValues;
        FW_RAISE_IF(error, !(strClassIter != ::fwData::StructureTraitsHelper::s_CLASSTRANSLATOR.right.end()));
        newOrgan->setClass(strClassIter->second);

        newOrgan->setColor(::fwData::Color::New(static_cast<float>(line.red)/255.0f,
                                                static_cast<float>(line.green)/255.0f,
                                                static_cast<float>(line.blue)/255.0f,
                                                static_cast<float>(line.alpha)/100.0f));
        std::vector<std::string> categorylist;
        ::boost::algorithm::split( categorylist, line.catgegory, ::boost::algorithm::is_any_of(",") );
        ::fwData::StructureTraits::CategoryContainer categories;
        for(std::string category :  categorylist)
        {
            std::string catReformated = reformatString(category);
            ::fwData::StructureTraitsHelper::CategoryTranslatorType::right_const_iterator strCategoryIter =
                ::fwData::StructureTraitsHelper::s_CATEGORYTRANSLATOR.right.find(catReformated);
            availableValues = getValues(
                ::fwData::StructureTraitsHelper::s_CATEGORYTRANSLATOR.right);
            error =
                "Category " + catReformated + " isn't available. Authorized type are " + availableValues;
            FW_RAISE_IF(error, !(strCategoryIter != ::fwData::StructureTraitsHelper::s_CATEGORYTRANSLATOR.right.end()));
            categories.push_back(strCategoryIter->second);
        }
        newOrgan->setCategories(categories);
        newOrgan->setAttachmentType(line.attachment);
        newOrgan->setNativeExp(line.nativeExp);
        newOrgan->setNativeGeometricExp(line.nativeExpGeo);
        newOrgan->setAnatomicRegion(line.anatomicRegion);
        newOrgan->setPropertyCategory(line.propertyCategory);
        newOrgan->setPropertyType(line.propertyType);
        structDico->addStructure(newOrgan);
    }
}

//------------------------------------------------------------------------------

std::string DictionaryReader::extension()
{
    SLM_TRACE_FUNC();
    return (".dic");
}

//------------------------------------------------------------------------------

std::filesystem::path DictionaryReader::getDefaultDictionaryPath()
{
    return ::fwRuntime::getLibraryResourceFilePath(PRJ_NAME "-" FWDATAIO_VER "/OrganDictionary.dic");
}

//------------------------------------------------------------------------------

} // namespace reader
} // namespace fwDataIO
