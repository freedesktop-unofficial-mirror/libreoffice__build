/*************************************************************************
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Kohei Yoshida.
 *    1039 Kingsway Dr., Apex, NC 27502, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
 
#include <unoglobal.hxx>
#include <iostream>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/table/CellAddress.hpp>

using ::com::sun::star::uno::Any;

namespace scsolver {

void printOUStr( const rtl::OUString & ou )
{
    rtl::OString o = OUStringToOString( ou, RTL_TEXTENCODING_UTF8 );
    std::cout << o.getStr() << std::endl;
}

const ::rtl::OUString ascii(const sal_Char* text)
{
	return rtl::OUString::createFromAscii(text);
}

const Any asciiAny(const sal_Char* text)
{
    Any any;
    any <<= ascii(text);
    return any;
}

/** Splits a string into a list of string elements separated by a specified
	separator string.

	@param sStr original string
	@param sSep separator string
	@param cn container containing a list of string elements
 */
void split( const rtl::OUString& sStr, const rtl::OUString& sSep,
		std::vector<rtl::OUString>& cn )
{
	sal_Int32 nLenSep = sSep.getLength();
	sal_Int32 nPos = sStr.indexOf( sSep );
	sal_Int32 nStart = 0;
	
	while ( nPos > -1 )
	{
		rtl::OUString s = sStr.copy( nStart, nPos - nStart );
		nStart = nPos + nLenSep;
		nPos = sStr.indexOf( sSep, nStart );
		cn.push_back( s );
	}
	rtl::OUString s = sStr.copy( nStart );
	cn.push_back( s );
}


// Global Parameters

const rtl::OUString Global::STRING_SEPARATOR = ascii( "," );
const rtl::OUString Global::MODEL_METADATA_NAME = ascii( "us.kohei.ooo.solver:model" );

RuntimeError::RuntimeError( const rtl::OUString& umsg ) :
	m_sUniMsg( umsg )
{
}

RuntimeError::~RuntimeError() throw()
{
}

const char* RuntimeError::what() const throw()
{
	rtl::OString o = OUStringToOString(m_sUniMsg, RTL_TEXTENCODING_UTF8);
    return o.getStr();
}

const rtl::OUString RuntimeError::getMessage() const throw()
{
	return m_sUniMsg;
}


}



