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


#ifndef _UNOGLOBAL_HXX_
#define _UNOGLOBAL_HXX_

#include <cppuhelper/implementationentry.hxx>
#include <vector>
#include <string>

using namespace com::sun::star;


namespace scsolver {

void printOUStr( const rtl::OUString & );
rtl::OUString ascii( const sal_Char* );
void split( const rtl::OUString&, const rtl::OUString&, std::vector<rtl::OUString>& );


struct Global
{
	static const rtl::OUString STRING_SEPARATOR;
	static const rtl::OUString MODEL_METADATA_NAME;
};


/**
 * This class is designed to be a generic exception class that
 * can have arbitrary error message.  It consists of one ASCII
 * message for stdout, and one OUString message which can be
 * localized.
 */
class RuntimeError : public ::std::exception
{
public:
	RuntimeError( const rtl::OUString& umsg );
	RuntimeError( const ::std::string& msg, const rtl::OUString& umsg );
	~RuntimeError() throw();

	virtual const char* what() const throw();
	virtual const rtl::OUString getMessage() const throw();

private:
	::std::string m_sMsg;
	rtl::OUString m_sUniMsg;
};


}

#endif //_UNOGLOBAL_HXX_
