/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
#include "vbachart.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <org/openoffice/excel/XlChartType.hpp>
#include <org/openoffice/excel/XlRowCol.hpp>
#include <basic/sberrors.hxx>
#include "vbachartobject.hxx"
#include "vbarange.hxx"
#include "vbacharttitle.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;
using namespace ::org::openoffice::excel::XlChartType;
using namespace ::org::openoffice::excel::XlRowCol;

const rtl::OUString CHART_NAME( RTL_CONSTASCII_USTRINGPARAM("Name") );
// #TODO move this constant to vbaseries.[ch]xx ( when it exists )
const rtl::OUString DEFAULTSERIESPREFIX( RTL_CONSTASCII_USTRINGPARAM("Series") );
const rtl::OUString DATAROWSOURCE( RTL_CONSTASCII_USTRINGPARAM("DataRowSource") );
const rtl::OUString UPDOWN( RTL_CONSTASCII_USTRINGPARAM("UpDown") );
const rtl::OUString VOLUME( RTL_CONSTASCII_USTRINGPARAM("Volume") );
const rtl::OUString LINES( RTL_CONSTASCII_USTRINGPARAM("Lines") );
const rtl::OUString SPLINETYPE( RTL_CONSTASCII_USTRINGPARAM("SplineType") );
const rtl::OUString SYMBOLTYPE( RTL_CONSTASCII_USTRINGPARAM("SymbolType") );
const rtl::OUString DEEP( RTL_CONSTASCII_USTRINGPARAM("Deep") );
const rtl::OUString SOLIDTYPE( RTL_CONSTASCII_USTRINGPARAM("SolidType") );
const rtl::OUString VERTICAL( RTL_CONSTASCII_USTRINGPARAM("Vertical") );
const rtl::OUString PERCENT( RTL_CONSTASCII_USTRINGPARAM("Percent") );
const rtl::OUString STACKED( RTL_CONSTASCII_USTRINGPARAM("Stacked") );
const rtl::OUString DIM3D( RTL_CONSTASCII_USTRINGPARAM("Dim3D") );
const rtl::OUString HASMAINTITLE( RTL_CONSTASCII_USTRINGPARAM("HasMainTitle") );
const rtl::OUString HASLEGEND( RTL_CONSTASCII_USTRINGPARAM("HasLegend") );
const rtl::OUString DATACAPTION( RTL_CONSTASCII_USTRINGPARAM("DataCaption") );

ScVbaChart::ScVbaChart( const css::uno::Reference< oo::vba::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::lang::XComponent >& _xChartComponent, const css::uno::Reference< css::table::XTableChart >& _xTableChart ) : ChartImpl_BASE( _xParent, _xContext ), mxTableChart( _xTableChart )
{
	mxChartDocument.set( _xChartComponent, uno::UNO_QUERY_THROW ) ;
	// #TODO is is possible that the XPropertySet interface is not set
	// code in setPlotBy seems to indicate that this is possible? but
	// additionally there is no check in most of the places where it is used
	// ( and therefore could possibly be NULL )
	// I'm going to let it throw for the moment ( npower )
	mxDiagramPropertySet.set( mxChartDocument->getDiagram(), uno::UNO_QUERY_THROW );
	mxChartPropertySet.set( _xChartComponent, uno::UNO_QUERY_THROW ) ;
}

::rtl::OUString SAL_CALL 
ScVbaChart::getName() throw (css::uno::RuntimeException)
{
	rtl::OUString sName;
	uno::Reference< beans::XPropertySet > xProps( mxChartDocument, uno::UNO_QUERY_THROW );
	try
	{
		xProps->getPropertyValue( CHART_NAME ) >>= sName;
	}
	catch( uno::Exception e ) // swallow exceptions
	{
	}
	return sName;
}

uno::Any  SAL_CALL
ScVbaChart::SeriesCollection(const uno::Any&) throw (uno::RuntimeException)
{
	return uno::Any();
}

::sal_Int32 SAL_CALL 
ScVbaChart::getChartType() throw ( uno::RuntimeException, script::BasicErrorException)
{
	sal_Int32 nChartType = -1;
	try
	{
		rtl::OUString sDiagramType = mxChartDocument->getDiagram()->getDiagramType();
		if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.AreaDiagram" ))))
		{
			if (is3D())
			{
				nChartType = getStackedType(xl3DAreaStacked, xl3DAreaStacked100, xl3DArea);
			}
			else
			{
				nChartType = getStackedType(xlAreaStacked, xlAreaStacked100, xlArea);
			}
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.PieDiagram"))))
		{
			if (is3D())
				nChartType = xl3DPie;
			else
				nChartType = xlPie;                 /*TODO XlChartType  xlPieExploded, XlChartType xlPieOfPie */
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.BarDiagram"))))
		{
			sal_Int32 nSolidType = chart::ChartSolidType::RECTANGULAR_SOLID;
			if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SOLIDTYPE)) 
			{       //in 2D diagrams 'SolidType' may not be set
				if (is3D())
					mxDiagramPropertySet->getPropertyValue(SOLIDTYPE) >>= nSolidType;
			}
			switch (nSolidType)
			{
				case chart::ChartSolidType::CONE:
					nChartType = getSolidType(xlConeCol, xlConeColStacked, xlConeColStacked100, xlConeColClustered, xlConeBarStacked, xlConeBarStacked100, xlConeBarClustered);
					break;
				case chart::ChartSolidType::CYLINDER:
					nChartType = getSolidType(xlCylinderCol, xlCylinderColStacked, xlCylinderColStacked100, xlCylinderColClustered, xlCylinderBarStacked, xlCylinderBarStacked100, xlCylinderBarClustered);
					break;
				case chart::ChartSolidType::PYRAMID:
					nChartType = getSolidType(xlPyramidCol, xlPyramidColStacked, xlPyramidColStacked100, xlPyramidColClustered, xlPyramidBarStacked, xlPyramidBarStacked100, xlPyramidBarClustered);
					break;
				default: // RECTANGULAR_SOLID
					if (is3D())
					{
						nChartType = getSolidType(xl3DColumn, xl3DColumnStacked, xl3DColumnStacked100, xl3DColumnClustered, xl3DBarStacked, xl3DBarStacked100, xl3DBarClustered);
					}
					else
					{
						nChartType = getSolidType(xlColumnClustered,  xlColumnStacked, xlColumnStacked100, xlColumnClustered, xlBarStacked, xlBarStacked100, xlBarClustered);
					}
					break;
				}
			}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.StockDiagram"))))
		{
			sal_Bool bVolume = sal_False;
			mxDiagramPropertySet->getPropertyValue(VOLUME) >>= bVolume;
			if (bVolume)
			{
				nChartType = getStockUpDownValue(xlStockVOHLC, xlStockVHLC);
			}
			else
			{
				nChartType = getStockUpDownValue(xlStockOHLC, xlStockHLC);
			}
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.XYDiagram"))))
		{
			sal_Bool bHasLines = sal_False;
			mxDiagramPropertySet->getPropertyValue(LINES) >>= bHasLines;
			sal_Int32 nSplineType = 0;
			mxDiagramPropertySet->getPropertyValue(SPLINETYPE) >>= nSplineType;
			if (nSplineType == 1)
			{
				nChartType = getMarkerType(xlXYScatterSmooth, xlXYScatterSmoothNoMarkers);
			}
			else if (bHasLines)
			{
				nChartType = getMarkerType(xlXYScatterLines, xlXYScatterLinesNoMarkers);
			}
			else
			{
				nChartType = xlXYScatter;
			}
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.LineDiagram"))))
		{
			if (is3D())
			{
				nChartType = xl3DLine;
			}
			else if (hasMarkers())
			{
				nChartType = getStackedType(xlLineMarkersStacked, xlLineMarkersStacked100, xlLineMarkers);
			}
			else
			{
				nChartType = getStackedType(xlLineStacked, xlLineStacked100, xlLine);
			}
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.DonutDiagram"))))
		{
			nChartType = xlDoughnut;                    // TODO DoughnutExploded ??
		}
		else if (sDiagramType.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.NetDiagram"))))
		{
			nChartType = getMarkerType(xlRadarMarkers, xlRadar);
		}
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	return nChartType;
}

void SAL_CALL 
ScVbaChart::setChartType( ::sal_Int32 _nChartType ) throw ( uno::RuntimeException, script::BasicErrorException)
{
try
{
	switch (_nChartType)
	{
		case xlColumnClustered:
		case xlColumnStacked:
		case xlColumnStacked100:
		case xl3DColumnClustered:
		case xl3DColumnStacked:
		case xl3DColumnStacked100:
		case xl3DColumn:
		case xlBarClustered:
		case xlBarStacked:
		case xlBarStacked100:
		case xl3DBarClustered:
		case xl3DBarStacked:
		case xl3DBarStacked100:
		case xlConeColClustered:
		case xlConeColStacked:
		case xlConeColStacked100:
		case xlConeBarClustered:
		case xlConeBarStacked:
		case xlConeBarStacked100:
		case xlConeCol:
		case xlPyramidColClustered:
		case xlPyramidColStacked:
		case xlPyramidColStacked100:
		case xlPyramidBarClustered:
		case xlPyramidBarStacked:
		case xlPyramidBarStacked100:
		case xlPyramidCol:
		case xlCylinderColClustered:
		case xlCylinderColStacked:
		case xlCylinderColStacked100:
		case xlCylinderBarClustered:
		case xlCylinderBarStacked:
		case xlCylinderBarStacked100:
		case xlCylinderCol:
		case xlSurface: // not possible
		case xlSurfaceWireframe:
		case xlSurfaceTopView:
		case xlSurfaceTopViewWireframe:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.BarDiagram")));
			break;
		case xlLine:
		case xl3DLine:
		case xlLineStacked:
		case xlLineStacked100:
		case xlLineMarkers:
		case xlLineMarkersStacked:
		case xlLineMarkersStacked100:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.LineDiagram")));
			break;
		case xl3DArea:
		case xlArea:
		case xlAreaStacked:
		case xlAreaStacked100:
		case xl3DAreaStacked:
		case xl3DAreaStacked100:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.AreaDiagram")) );
			break;
		case xlDoughnut:
		case xlDoughnutExploded:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.DonutDiagram") ) );
			break;
		case xlStockHLC:
		case xlStockOHLC:
		case xlStockVHLC:
		case xlStockVOHLC:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.StockDiagram")));
			mxDiagramPropertySet->setPropertyValue( UPDOWN, uno::makeAny(sal_Bool((_nChartType == xlStockOHLC) || (_nChartType == xlStockVOHLC))));
			mxDiagramPropertySet->setPropertyValue(VOLUME, uno::makeAny(sal_Bool((_nChartType == xlStockVHLC) || (_nChartType == xlStockVOHLC))));
			break;
	
		case xlPieOfPie:                            // not possible
		case xlPieExploded: // SegmentOffset an ChartDataPointProperties ->am XDiagram abholen //wie macht Excel das?
		case xl3DPieExploded:
		case xl3DPie:
		case xlPie:
		case xlBarOfPie:                            // not possible (Zoom pie)
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.PieDiagram")));
			break;
		
		case xlRadar:
		case xlRadarMarkers:
		case xlRadarFilled:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.NetDiagram")));
			break;
		case xlXYScatter:
		case xlBubble:                      // not possible
		case xlBubble3DEffect:              // not possible
		case xlXYScatterLines:
		case xlXYScatterLinesNoMarkers:
		case xlXYScatterSmooth:
		case xlXYScatterSmoothNoMarkers:
			setDiagram( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.XYDiagram")));
			switch(_nChartType)
			{
				case xlXYScatter:
				case xlBubble:                      // not possible
				case xlBubble3DEffect:              // not possible
					mxDiagramPropertySet->setPropertyValue(LINES, uno::makeAny( sal_False ));
					break;
				case xlXYScatterLines:
				case xlXYScatterLinesNoMarkers:
					mxDiagramPropertySet->setPropertyValue(LINES, uno::makeAny( sal_True ));
					break;
				case xlXYScatterSmooth:
				case xlXYScatterSmoothNoMarkers:
					mxDiagramPropertySet->setPropertyValue(SPLINETYPE, uno::makeAny( sal_Int32(1)));
					break;
				default:
					break;
			}
			break;
		default:
			throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_CONVERSION, rtl::OUString() );
		break;
	}
	
	switch (_nChartType) 
	{
		case xlLineMarkers:
		case xlLineMarkersStacked:
		case xlLineMarkersStacked100:
		case xlRadarMarkers:
		case xlXYScatterLines:
		case xlXYScatterSmooth:
		case xlXYScatter:
		case xlBubble:                      // not possible
		case xlBubble3DEffect:              // not possible
			mxDiagramPropertySet->setPropertyValue(SYMBOLTYPE, uno::makeAny( chart::ChartSymbolType::AUTO));
			break;
		default:
			if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SYMBOLTYPE))
			{
				mxDiagramPropertySet->setPropertyValue(SYMBOLTYPE, uno::makeAny(chart::ChartSymbolType::NONE));
			}
			break;
	}

	switch (_nChartType)
	{
		case xlConeCol:
		case xlPyramidCol:
		case xlCylinderCol:
		case xl3DColumn:
		case xlSurface:                         // not possible
		case xlSurfaceWireframe:
		case xlSurfaceTopView:
		case xlSurfaceTopViewWireframe:
			mxDiagramPropertySet->setPropertyValue(DEEP,uno::makeAny( sal_True ));
			break;
		default:
				if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(DEEP))
				{
					mxDiagramPropertySet->setPropertyValue(DEEP, uno::makeAny( sal_False));
				}
				break;
		}


		switch (_nChartType)
		{
				case xlConeColClustered:
				case xlConeColStacked:
				case xlConeColStacked100:
				case xlConeBarClustered:
				case xlConeBarStacked:
				case xlConeBarStacked100:
				case xlConeCol:
						mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::CONE));
						break;
				case xlPyramidColClustered:
				case xlPyramidColStacked:
				case xlPyramidColStacked100:
				case xlPyramidBarClustered:
				case xlPyramidBarStacked:
				case xlPyramidBarStacked100:
				case xlPyramidCol:
						mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::PYRAMID));
						break;
				case xlCylinderColClustered:
				case xlCylinderColStacked:
				case xlCylinderColStacked100:
				case xlCylinderBarClustered:
				case xlCylinderBarStacked:
				case xlCylinderBarStacked100:
				case xlCylinderCol:
						mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::CYLINDER));
						break;
				default:
					if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SOLIDTYPE))
					{
							mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::RECTANGULAR_SOLID));
					}
					break;
		}

		switch ( _nChartType)
		{
			case xlConeCol:
			case xlConeColClustered:
			case xlConeColStacked:
			case xlConeColStacked100:
			case xlPyramidColClustered:
			case xlPyramidColStacked:
			case xlPyramidColStacked100:
			case xlCylinderColClustered:
			case xlCylinderColStacked:
			case xlCylinderColStacked100:
			case xlColumnClustered:
			case xlColumnStacked:
			case xlColumnStacked100:
			case xl3DColumnClustered:
			case xl3DColumnStacked:
			case xl3DColumnStacked100:
			case xlSurface: // not possible
			case xlSurfaceWireframe:
			case xlSurfaceTopView:
			case xlSurfaceTopViewWireframe:
				mxDiagramPropertySet->setPropertyValue(VERTICAL, uno::makeAny( sal_True));
				break;
			default:
				if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(VERTICAL))
				{
					mxDiagramPropertySet->setPropertyValue(VERTICAL, uno::makeAny(sal_False));
				}
				break;
		}
	
		switch (_nChartType)
		{
			case xlColumnStacked:
			case xl3DColumnStacked:
			case xlBarStacked:
			case xl3DBarStacked:
			case xlLineStacked:
			case xlLineMarkersStacked:
			case xlAreaStacked:
			case xl3DAreaStacked:
			case xlCylinderColStacked:
			case xlCylinderBarStacked:
			case xlConeColStacked:
			case xlConeBarStacked:
			case xlPyramidColStacked:
			case xlPyramidBarStacked:
				mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_False ));
				mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_True ));
				break;
			case xlPyramidColStacked100:
			case xlPyramidBarStacked100:
			case xlConeColStacked100:
			case xlConeBarStacked100:
			case xlCylinderBarStacked100:
			case xlCylinderColStacked100:
			case xl3DAreaStacked100:
			case xlLineMarkersStacked100:
			case xlAreaStacked100:
			case xlLineStacked100:
			case xl3DBarStacked100:
			case xlBarStacked100:
			case xl3DColumnStacked100:
			case xlColumnStacked100:
				mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_True));
				mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_True ));
				break;
			default:
				mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_False));
				mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_False));
				break;
		}
		switch (_nChartType) 
		{
			case xl3DArea:
			case xl3DAreaStacked:
			case xl3DAreaStacked100:
			case xl3DBarClustered:
			case xl3DBarStacked:
			case xl3DBarStacked100:
			case xl3DColumn:
			case xl3DColumnClustered:
			case xl3DColumnStacked:
			case xl3DColumnStacked100:
			case xl3DLine:
			case xl3DPie:
			case xl3DPieExploded:
			case xlConeColClustered:
			case xlConeColStacked:
			case xlConeColStacked100:
			case xlConeBarClustered:
			case xlConeBarStacked:
			case xlConeBarStacked100:
			case xlConeCol:
			case xlPyramidColClustered:
			case xlPyramidColStacked:
			case xlPyramidColStacked100:
			case xlPyramidBarClustered:
			case xlPyramidBarStacked:
			case xlPyramidBarStacked100:
			case xlPyramidCol:
			case xlCylinderColClustered:
			case xlCylinderColStacked:
			case xlCylinderColStacked100:
			case xlCylinderBarClustered:
			case xlCylinderBarStacked:
			case xlCylinderBarStacked100:
			case xlCylinderCol:
				mxDiagramPropertySet->setPropertyValue(DIM3D, uno::makeAny( sal_True));
				break;
			default:
				if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(DIM3D))
				{
					mxDiagramPropertySet->setPropertyValue(DIM3D, uno::makeAny( sal_False));
				}
				break;
		}
	}
	catch ( uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
}

void SAL_CALL
ScVbaChart::Activate() throw (script::BasicErrorException, uno::RuntimeException)
{
	// #TODO how are Chart sheets handled ( I know we don't even consider
	// them in the worksheets/sheets collections ), but.....???
	// note: in vba for excel the parent of a Chart sheet is a workbook,
	// e.g. 'ThisWorkbook'
	uno::Reference< vba::XHelperInterface > xParent( getParent() );
	ScVbaChartObject* pChartObj = static_cast< ScVbaChartObject* >( xParent.get() );
	if ( pChartObj )
		pChartObj->Activate();
	else
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no ChartObject as parent" ) ) );
}

void SAL_CALL 
ScVbaChart::setSourceData( const css::uno::Reference< ::org::openoffice::excel::XRange >& _xCalcRange, const css::uno::Any& _aPlotBy ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	try
	{
		uno::Sequence< table::CellRangeAddress > mRangeAddresses(1);
		table::CellRangeAddress mSingleRangeAddress;

		uno::Reference< sheet::XCellRangeAddressable > xAddressable( _xCalcRange->getCellRange(), uno::UNO_QUERY_THROW );
		mSingleRangeAddress = xAddressable->getRangeAddress();

		mRangeAddresses[0] = mSingleRangeAddress;

		mxTableChart->setRanges(mRangeAddresses);

		sal_Bool bsetRowHeaders = sal_False;
		sal_Bool bsetColumnHeaders = sal_False;

		ScVbaRange* pRange = static_cast< ScVbaRange* >( _xCalcRange.get() );
		if ( pRange )
		{
			ScDocument* pDoc = pRange->getScDocument();
			if ( pDoc )
			{
				bsetRowHeaders = pDoc->HasRowHeader(  static_cast< SCCOL >( mSingleRangeAddress.StartColumn ), static_cast< SCROW >( mSingleRangeAddress.StartRow ), static_cast< SCCOL >( mSingleRangeAddress.EndColumn ), static_cast< SCROW >( mSingleRangeAddress.EndRow ), static_cast< SCTAB >( mSingleRangeAddress.Sheet ) );;
				bsetColumnHeaders =  pDoc->HasColHeader(  static_cast< SCCOL >( mSingleRangeAddress.StartColumn ), static_cast< SCROW >( mSingleRangeAddress.StartRow ), static_cast< SCCOL >( mSingleRangeAddress.EndColumn ), static_cast< SCROW >( mSingleRangeAddress.EndRow ), static_cast< SCTAB >( mSingleRangeAddress.Sheet ));
;
			}
		}
		mxTableChart->setHasRowHeaders(bsetRowHeaders);
		mxTableChart->setHasColumnHeaders(bsetColumnHeaders);

		if ((!bsetColumnHeaders) || (!bsetRowHeaders))
		{
			uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
			if (!bsetColumnHeaders)
			{
				xChartDataArray->setColumnDescriptions( getDefaultSeriesDescriptions(xChartDataArray->getColumnDescriptions().getLength() ));
			}
			if (!bsetRowHeaders)
			{
				xChartDataArray->setRowDescriptions(getDefaultSeriesDescriptions(xChartDataArray->getRowDescriptions().getLength() ));
			}
		}

		if ( _aPlotBy.hasValue() )
		{
			sal_Int32 nVal;
			_aPlotBy >>= nVal;
			setPlotBy( nVal );
		}
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
}

uno::Sequence< rtl::OUString > 
ScVbaChart::getDefaultSeriesDescriptions( sal_Int32 _nCount )
{
	uno::Sequence< rtl::OUString > sDescriptions ( _nCount );
	sal_Int32 nLen = sDescriptions.getLength();
	for (sal_Int32 i = 0; i < nLen; i++)
	{
		sDescriptions[i] = DEFAULTSERIESPREFIX + rtl::OUString::valueOf(i+1);
	}
	return sDescriptions;
}

void
ScVbaChart::setDefaultChartType() throw ( script::BasicErrorException )
{
	setChartType( xlColumnClustered );
}

void
ScVbaChart::setPlotBy( ::sal_Int32 _nPlotBy ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	try
	{
		if ( !mxDiagramPropertySet.is() )
			setDefaultChartType();
		switch (_nPlotBy)
		{
			case xlRows:
				mxDiagramPropertySet->setPropertyValue( DATAROWSOURCE,  uno::makeAny( chart::ChartDataRowSource_ROWS ) );
				break;
			case xlColumns:
				mxDiagramPropertySet->setPropertyValue( DATAROWSOURCE, uno::makeAny( chart::ChartDataRowSource_COLUMNS) );
				break;
			default:
				throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
		}
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}	
}

::sal_Int32 SAL_CALL 
ScVbaChart::getPlotBy(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
	try
	{
		chart::ChartDataRowSource aChartDataRowSource;
		mxDiagramPropertySet->getPropertyValue(DATAROWSOURCE) >>= aChartDataRowSource;
		if (aChartDataRowSource == chart::ChartDataRowSource_COLUMNS)
		{
			return xlColumns;
		}
		else
		{
			return xlRows;
		}
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}	
	return xlColumns;
}

void
ScVbaChart::setDiagram( const rtl::OUString& _sDiagramType ) throw( script::BasicErrorException )
{
	try
	{
		uno::Reference< chart::XDiagram > xDiagram( mxContext->getServiceManager()->createInstanceWithContext( _sDiagramType, mxContext ), uno::UNO_QUERY_THROW );
		mxChartDocument->setDiagram( xDiagram );
		mxDiagramPropertySet.set( xDiagram, uno::UNO_QUERY_THROW );
	}
	catch ( uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
}

// #TODO find out why we have Location/getLocation ? there is afaiks no
// Location property, just a Location function for the Chart object
sal_Int32 SAL_CALL 
ScVbaChart::Location() throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	return getLocation();	
}

sal_Int32 SAL_CALL 
ScVbaChart::getLocation() throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	 return -1;
}

void SAL_CALL 
ScVbaChart::setLocation( ::sal_Int32 /*where*/, const css::uno::Any& /*Name*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
	// Helper api just stubs out the code <shrug>
	// #TODO come back and make sense out of this	
//        String sheetName = null;
//
//        if ((name != null) && name instanceof String) {
//            sheetName = (String) name;
//        }
//        XSpreadsheetDocument xShDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface( XSpreadsheetDocument.class,getXModel() );
//        com.sun.star.sheet.XSpreadsheets xSheets = xShDoc.Sheets();
//
//        switch (where) {
//        case ClLocationType.clLocationAsObject_value: //{
//
//            if (sheetName == null) {
//                DebugHelper.writeInfo("Can't embed in Chart without knowing SheetName");
//                return;
//            }
//
//            try {
//                Any any = (Any) xSheets.getByName(sheetName);
//                chartSheet = (XSpreadsheet) any.getObject();
//
//                // chartSheet = (XSpreadsheet) xSheets.getByName( sheetName );
//            } catch (NoSuchElementException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (WrappedTargetException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (java.lang.Exception e) {
//                e.printStackTrace();
//            }
//
//            XTableChartsSupplier xTCS = (XTableChartsSupplier) UnoRuntime.queryInterface( XTableChartsSupplier.class, chartSheet);
//            XTableCharts xTableCharts = xTCS.getCharts();
//            XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface( XIndexAccess.class, xTableCharts);
//            int numCharts = xIA.getCount();
//            chartName = "Chart " + (numCharts + 1);
//
//            //}
//            break;
//
//        case ClLocationType.clLocationAsNewSheet_value:
//        case ClLocationType.clLocationAutomatic_value:default: //{
//            chartName = "Chart 1"; // Since it's a new sheet, it's the first on it...
//
//            XIndexAccess xSheetIA = (XIndexAccess) UnoRuntime.queryInterface( XIndexAccess.class, xSheets);
//
//            short newSheetNum = (short) (xSheetIA.getCount() + 1);
//
//            if (sheetName == null){
//                sheetName = "ChartSheet " + newSheetNum; // Why not?
//            }
//            // DPK TODO : Probably should use Sheets to create this!
//            xSheets.insertNewByName(sheetName, newSheetNum);
//
//            try {
//                chartSheet =
//                    (XSpreadsheet) xSheets.getByName(sheetName);
//            } catch (NoSuchElementException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (WrappedTargetException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            }
//
//            //}
//            break;
//        }
//
//        // Last thing should be a call to createChartForReal(), one of them
//        // should succeed.
//        createChartForReal();

}

sal_Bool SAL_CALL 
ScVbaChart::getHasTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
	sal_Bool bHasTitle = sal_False;
	try
	{
		mxChartPropertySet->getPropertyValue(HASMAINTITLE) >>= bHasTitle;
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	return bHasTitle;
}

void SAL_CALL 
ScVbaChart::setHasTitle( ::sal_Bool bTitle ) throw (script::BasicErrorException, uno::RuntimeException)
{
	try
	{
		mxChartPropertySet->setPropertyValue(HASMAINTITLE, uno::makeAny( bTitle ));
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	
}

::sal_Bool SAL_CALL 
ScVbaChart::getHasLegend(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
	sal_Bool bHasLegend = sal_False;
	try
	{
		mxChartPropertySet->getPropertyValue(HASLEGEND) >>= bHasLegend;
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	return bHasLegend;
}

void SAL_CALL 
ScVbaChart::setHasLegend( ::sal_Bool bLegend ) throw (script::BasicErrorException, uno::RuntimeException)
{
	try
	{
		mxChartPropertySet->setPropertyValue(HASLEGEND, uno::makeAny(bLegend));
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
}

uno::Reference< excel::XChartTitle > SAL_CALL 
ScVbaChart::getChartTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
	uno::Reference< drawing::XShape > xTitleShape = mxChartDocument->getTitle();
	// #TODO check parent
	return new ScVbaChartTitle(this, mxContext, xTitleShape);		
}

uno::Any SAL_CALL 
ScVbaChart::Axes( const uno::Any& Type, const uno::Any& AxisGroup ) throw (script::BasicErrorException, uno::RuntimeException)
{
	if ( !Type.hasValue() )
	{
		// return ScVbaAxes collection of Axis
	}
	else
	{
		//ScVbaAxes xAxes;
		//xAxes.item( Type, AxisGroup );
	}
}
bool
ScVbaChart::is3D() throw ( uno::RuntimeException )
{
	// #TODO perhaps provide limited Debughelper functionality
	sal_Bool is3d = sal_False;
	mxDiagramPropertySet->getPropertyValue(DIM3D) >>= is3d;
	return is3d;
} 

sal_Int32
ScVbaChart::getStackedType( sal_Int32 _nStacked, sal_Int32 _n100PercentStacked, sal_Int32 _nUnStacked ) throw ( uno::RuntimeException )
{
	// #TODO perhaps provide limited Debughelper functionality
	if (isStacked())
	{
		if (is100PercentStacked())
			return _n100PercentStacked;
		else
			return _nStacked;
	}
	else
		return _nUnStacked;
}

bool
ScVbaChart::isStacked() throw ( uno::RuntimeException )
{
	// #TODO perhaps provide limited Debughelper functionality
	sal_Bool bStacked = sal_False;
	mxDiagramPropertySet->getPropertyValue(STACKED) >>= bStacked;
	return bStacked;
}

bool
ScVbaChart::is100PercentStacked() throw ( uno::RuntimeException )
{
	// #TODO perhaps provide limited Debughelper functionality
	sal_Bool b100Percent = sal_False;
	mxDiagramPropertySet->getPropertyValue(PERCENT) >>= b100Percent;
	return b100Percent;
}

sal_Int32 
ScVbaChart::getSolidType(sal_Int32 _nDeep, sal_Int32 _nVertiStacked, sal_Int32 _nVerti100PercentStacked, sal_Int32 _nVertiUnStacked, sal_Int32 _nHoriStacked, sal_Int32 _nHori100PercentStacked, sal_Int32 _nHoriUnStacked) throw ( script::BasicErrorException )
{
	sal_Bool bIsVertical = true;
	try
	{
		mxDiagramPropertySet->getPropertyValue(VERTICAL) >>= bIsVertical;
		sal_Bool bIsDeep = false;
		mxDiagramPropertySet->getPropertyValue(DEEP) >>= bIsDeep;

		if (bIsDeep)
		{
			return _nDeep;
		}
		else
		{
			if (bIsVertical)
			{
				return getStackedType(_nVertiStacked, _nVerti100PercentStacked, _nVertiUnStacked);
			}
			else
			{
				return getStackedType(_nHoriStacked, _nHori100PercentStacked, _nHoriUnStacked);
			}
		}
	}
	catch (uno::Exception& e)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	if (bIsVertical)
	{
		return _nVertiStacked;
	}
	else
	{
		return _nHoriStacked;
	}	
}


sal_Int32 
ScVbaChart::getStockUpDownValue(sal_Int32 _nUpDown, sal_Int32 _nNotUpDown) throw (script::BasicErrorException)
{
	sal_Bool bUpDown = sal_False;
	try
	{
		mxDiagramPropertySet->getPropertyValue(UPDOWN) >>= bUpDown;
		if (bUpDown)
		{
			return _nUpDown;
		}
		else
		{
			return _nNotUpDown;
		}
	}
	catch (uno::Exception& e)
	{
		script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );	
	}
	return _nNotUpDown;
}

bool 
ScVbaChart::hasMarkers() throw ( script::BasicErrorException )
{
	bool bHasMarkers = false;
	try
	{
		sal_Int32 nSymbol=0;
		mxDiagramPropertySet->getPropertyValue(SYMBOLTYPE) >>= nSymbol;
		bHasMarkers = nSymbol != chart::ChartSymbolType::NONE;
	}
	catch ( uno::Exception& e )
	{
		script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );	
	}
	return bHasMarkers;
}

sal_Int32 
ScVbaChart::getMarkerType(sal_Int32 _nWithMarkers, sal_Int32 _nWithoutMarkers) throw ( script::BasicErrorException )
{
	if (hasMarkers())
		return _nWithMarkers;
	return _nWithoutMarkers;
}

void 
ScVbaChart::assignDiagramAttributes()
{
	xAxisXSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
	xAxisYSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
	xAxisZSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
	xTwoAxisXSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
	xTwoAxisYSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
}

bool
ScVbaChart::isSeriesIndexValid(sal_Int32 _seriesindex) throw( script::BasicErrorException )
{
	bool bret = false;
	try
	{
		uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
		//        dblValues = xChartDataArray.getData();
		//TODO I guess we have to differentiate between XlRowCol
		if ( !xChartDataArray.is() )
		{
			if (getPlotBy() == xlRows)
			{
				if ((_seriesindex < xChartDataArray->getRowDescriptions().getLength() ) && (_seriesindex >= 0))
					bret = true;
			}
			else
			{
				if ((_seriesindex < xChartDataArray->getColumnDescriptions().getLength() ) && (_seriesindex >= 0))
					bret = true;
			}
		}
	} 
	catch (uno::Exception& e) 
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	if (!bret)
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_OUT_OF_RANGE, rtl::OUString() );
	}
	return bret;
}

bool
ScVbaChart::areIndicesValid( sal_Int32 _seriesindex, sal_Int32 _valindex) throw ( css::script::BasicErrorException )
{
	if (isSeriesIndexValid(_seriesindex))
	{
		uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
		dblValues = xChartDataArray->getData();
		return (_valindex < dblValues[_seriesindex].getLength() );
        }
	return false;
}

sal_Int32
ScVbaChart::getSeriesIndex(rtl::OUString _sseriesname) throw ( script::BasicErrorException )
{
	uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
	if (getPlotBy() == xlRows)
		return ContainerUtilities::FieldInList(xChartDataArray->getRowDescriptions(), _sseriesname);
	return ContainerUtilities::FieldInList(xChartDataArray->getColumnDescriptions(), _sseriesname);
}
void
ScVbaChart::setSeriesName(sal_Int32 _index, rtl::OUString _sname) throw ( script::BasicErrorException )
{
	uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
	if (isSeriesIndexValid(_index))
	{
		uno::Sequence< rtl::OUString > sDescriptions = xChartDataArray->getColumnDescriptions();
		sDescriptions[_index] = _sname;
		xChartDataArray->setColumnDescriptions(sDescriptions);
	}
}

sal_Int32 
ScVbaChart::getSeriesCount() throw ( script::BasicErrorException )
{		
	uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );

	if (getPlotBy() == xlRows)
		return xChartDataArray->getRowDescriptions().getLength();
	return xChartDataArray->getColumnDescriptions().getLength();

}

rtl::OUString
ScVbaChart::getSeriesName(sal_Int32 _index) throw ( script::BasicErrorException )
{
	uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
        uno::Sequence< rtl::OUString > sDescriptions;
	rtl::OUString sName;
	if (isSeriesIndexValid(_index))
	{
		if (getPlotBy() == xlRows)
			sDescriptions = xChartDataArray->getRowDescriptions();
		else
			sDescriptions = xChartDataArray->getColumnDescriptions();
		sName =  sDescriptions[_index];
        }
        return sName;	
}

double 
ScVbaChart::getValue(sal_Int32 _seriesindex, sal_Int32 _valindex) throw ( script::BasicErrorException )
{
	double result = -1.0;
	if (areIndicesValid(_seriesindex, _valindex))
	{
		if (getPlotBy() == xlRows)
			result =  dblValues[_seriesindex][_valindex];
		else
			result =  dblValues[_valindex][_seriesindex];
	}
	return result;
}

sal_Int32 
ScVbaChart::getValuesCount(sal_Int32 _seriesIndex) throw ( script::BasicErrorException )
{
	sal_Int32 nCount = 0;
	uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
	if (isSeriesIndexValid(_seriesIndex))
	{
		dblValues = xChartDataArray->getData();
		if (getPlotBy() == xlRows)
			nCount = dblValues[_seriesIndex].getLength();
		else
			nCount =  dblValues.getLength();
	}
	return nCount;	
}


uno::Reference< excel::XDataLabels > 
ScVbaChart::DataLabels( const uno::Reference< oo::excel::XSeries > /*_oSeries*/ ) throw ( css::script::BasicErrorException )
{
	if ( true )
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	// #TODO #FIXE provide implementation
	return uno::Reference< excel::XDataLabels > ();
}

bool 
ScVbaChart::getHasDataCaption( const uno::Reference< css::beans::XPropertySet >& _xPropertySet )throw ( script::BasicErrorException )
{
	bool bResult = false;
	try
	{
		sal_Int32 nChartDataCaption = 0;
		_xPropertySet->getPropertyValue(DATACAPTION) >>= nChartDataCaption;
		bResult = (nChartDataCaption != chart::ChartDataCaption::NONE);
	} 
	catch (uno::Exception& e) 
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
	return bResult;
}

void 
ScVbaChart::setHasDataCaption( const uno::Reference< beans::XPropertySet >& _xPropertySet, bool _bHasDataLabels )throw ( script::BasicErrorException )
{
	try
	{
		if ( _bHasDataLabels )
			_xPropertySet->setPropertyValue(DATACAPTION, uno::makeAny ( chart::ChartDataCaption::VALUE) );
		else
			_xPropertySet->setPropertyValue(DATACAPTION, uno::makeAny ( chart::ChartDataCaption::NONE) );
	} 
	catch (uno::Exception& e) 
	{
		throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
	}
}

rtl::OUString&
ScVbaChart::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaChart") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
ScVbaChart::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Chart" ) );
	}
	return aServiceNames;
}

