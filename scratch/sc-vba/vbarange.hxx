#ifndef SC_VBA_RANGE_HXX
#define SC_VBA_RANGE_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/vba/XRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <org/openoffice/vba/XFont.hpp>
#include "org/openoffice/vba/xlPasteType.hdl"
#include "org/openoffice/vba/xlPasteSpecialOperation.hdl"

#include "vbahelper.hxx"

class ScTableSheetsObj;

typedef ::cppu::WeakImplHelper1< org::openoffice::vba::XRange > ScVbaRange_BASE;

class ScVbaRange : public ScVbaRange_BASE
{
	uno::Reference< table::XCellRange > mxRange;
	uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
	sal_Bool mbIsRows;
	sal_Bool mbIsColumns;

public:
	ScVbaRange( uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext, uno::Reference< table::XCellRange > xRange, sal_Bool bIsRows = false, sal_Bool bIsColumns = false ) : mxRange( xRange ),
		m_xContext(xContext),
		mbIsRows( bIsRows ),
		mbIsColumns( bIsColumns ){}
	virtual ~ScVbaRange() {}

    // Attributes
	virtual uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException);
	virtual void   SAL_CALL setValue( const uno::Any& aValue ) throw ( uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getFormula() throw (uno::RuntimeException);
	virtual void   SAL_CALL setFormula( const ::rtl::OUString &rFormula ) throw (uno::RuntimeException);
	virtual double SAL_CALL getCount() throw (uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getRow() throw (uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getColumn() throw (uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getText() throw (uno::RuntimeException);
	virtual void   SAL_CALL setText( const ::rtl::OUString &rString ) throw (uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getFormulaArray() throw (uno::RuntimeException);
	virtual void   SAL_CALL setFormulaArray(const ::rtl::OUString &rFormula) throw (uno::RuntimeException);
	virtual void SAL_CALL setNumberFormat( const ::rtl::OUString &rNumberFormat ) throw (uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getNumberFormat() throw (uno::RuntimeException);
	virtual void SAL_CALL setMergeCells( sal_Bool bMerge ) throw (uno::RuntimeException);
	virtual sal_Bool SAL_CALL getMergeCells() throw (uno::RuntimeException);
	virtual void SAL_CALL setWrapText( sal_Bool bIsWrapped ) throw (uno::RuntimeException);
	virtual sal_Bool SAL_CALL getWrapText() throw (uno::RuntimeException);

	// Methods
	sal_Bool IsRows() { return mbIsRows; }
	sal_Bool IsColumns() { return mbIsColumns; }
	virtual void SAL_CALL Clear() throw (uno::RuntimeException);
	virtual void SAL_CALL ClearComments() throw (uno::RuntimeException);
	virtual void SAL_CALL ClearContents() throw (uno::RuntimeException);
	virtual void SAL_CALL ClearFormats() throw (uno::RuntimeException);
	virtual sal_Bool SAL_CALL HasFormula() throw (uno::RuntimeException);
	virtual void SAL_CALL FillLeft() throw (uno::RuntimeException);
	virtual void SAL_CALL FillRight() throw (uno::RuntimeException);
	virtual void SAL_CALL FillUp() throw (uno::RuntimeException);
	virtual void SAL_CALL FillDown() throw (uno::RuntimeException);
	virtual	uno::Reference< vba::XRange > SAL_CALL Offset( const ::uno::Any &nRowOffset, const ::uno::Any &nColOffset )
														   throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange > SAL_CALL CurrentRegion() throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange > SAL_CALL CurrentArray() throw (uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL Characters( const ::uno::Any& nIndex, const ::uno::Any& nCount ) 
												 throw (uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL Address() throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange > SAL_CALL Cells( const ::uno::Any &nRow, const ::uno::Any &nCol ) 
														  throw (uno::RuntimeException);
	virtual void SAL_CALL Select() throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange >  SAL_CALL Rows( const uno::Any& nIndex ) throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange >  SAL_CALL Columns( const uno::Any &nIndex ) throw (uno::RuntimeException);
	virtual void SAL_CALL Copy( const ::uno::Any& Destination ) throw (uno::RuntimeException);
	virtual void SAL_CALL Cut( const ::uno::Any& Destination ) throw (uno::RuntimeException);
	virtual uno::Reference< vba::XRange > SAL_CALL Resize( const ::uno::Any& RowSize, const ::uno::Any& ColumnSize ) 
														   throw (uno::RuntimeException);
	virtual uno::Reference< vba::XFont > SAL_CALL Font() throw (uno::RuntimeException);
	virtual uno::Reference< vba::XInterior > SAL_CALL Interior(  ) throw (::com::sun::star::uno::RuntimeException) ;
	virtual uno::Reference< vba::XRange > SAL_CALL Range( const uno::Any &Cell1, const uno::Any &Cell2 )
															throw (uno::RuntimeException);
	virtual ::com::sun::star::uno::Any SAL_CALL getCellRange(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL PasteSpecial( sal_Int16 Paste, sal_Int16 Operation, ::sal_Bool SkipBlanks, ::sal_Bool Transpose ) throw (::com::sun::star::uno::RuntimeException);
};

#endif /* SC_VBA_RANGE_HXX */

