
#include "dpresulttester.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotTableRegion.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotField.hpp>
#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <com/sun/star/sheet/XDataPilotTable2.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellContentType.hpp>

#include <stdio.h>
#include <cmath>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sheet;

using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCell;
using ::com::sun::star::table::CellContentType;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::vector;
using ::boost::shared_ptr;

namespace dptest {

ResultTester::DataFieldSetting::DataFieldSetting() : 
        FieldRef(static_cast<DataPilotFieldReference*>(NULL)) 
{
}

// ============================================================================


ResultTester::ResultTester(const RuntimeData& data, const Reference<XDataPilotTable2>& xDPTab) :
    maData(data), mxDPTab(xDPTab), mnFailureCount(0)
{
    init();
}

ResultTester::ResultTester(const ResultTester& other) :
    maData(other.maData), mxDPTab(other.mxDPTab), 
    maDataFieldSettings(other.maDataFieldSettings),
    mnFailureCount(other.mnFailureCount)
{
    init();
}

void ResultTester::init()
{
    Reference<XDataPilotDescriptor> xDPDesc(mxDPTab, UNO_QUERY_THROW);

    Reference<container::XIndexAccess> xDataFields = xDPDesc->getDataFields();
    sal_Int32 fieldCount = xDataFields->getCount();
    maDataFieldSettings.reserve(fieldCount);
    for (sal_Int32 i = 0; i < fieldCount; ++i)
    {
        Reference<XDataPilotField> xField(xDataFields->getByIndex(i), UNO_QUERY_THROW);
        DataFieldSetting setting;

        // Get the field ID of a given data field.
        Reference<container::XNamed> xNamed(xField, UNO_QUERY_THROW);
        OUString name = xNamed->getName();
        setting.FieldId = maData.CacheTable.getFieldIndex(name);

        // Get the function used for aggregation.
        Reference<XPropertySet> xPS(xField, UNO_QUERY_THROW);
        getPropertyValue(xPS, ascii("Function"), setting.Function);

        // Get the referenced item information (if any).
        bool hasReference = false;
        getPropertyValue(xPS, ascii("HasReference"), hasReference);
        if (hasReference)
        {
            setting.FieldRef.reset(new DataPilotFieldReference);
            getPropertyValue(xPS, ascii("Reference"), *setting.FieldRef);
        }
        maDataFieldSettings.push_back(setting);
    }
}

void ResultTester::operator()(const CellAddress& cell)
{
    DataPilotTablePositionData posData = mxDPTab->getPositionData(cell);

    if (posData.PositionType != DataPilotTablePositionType::RESULT)
        // This cell doesn't belong to the result area.  Bail out.
        return;

    DataPilotTableResultData resData;
    if (!(posData.PositionData >>= resData))
        // For whatever reason unpacking of the result data failed.  Bail out.
        return;

    vector<DataTable::Filter> filters;
    sal_Int32 filterSize = resData.FieldFilters.getLength();
    filters.reserve(filterSize);
    for (sal_Int32 i = 0; i < filterSize; ++i)
    {
        sal_Int32 nFieldId = maData.CacheTable.getFieldIndex(resData.FieldFilters[i].FieldName);

        if (nFieldId >= 0)
        {
            DataTable::Filter filter;
            filter.FieldIndex = nFieldId;
            filter.MatchStrId = DataTable::getStringId(resData.FieldFilters[i].MatchValue);
            filters.push_back(filter);
        }
    }

    // ID only for the data field set, not the actual column ID.
    sal_Int32 nId = resData.DataFieldIndex; 
    const DataFieldSetting& setting = maDataFieldSettings.at(nId);

    if (setting.FieldRef.get())
    {
        // referenced item exists.
        verifyRefValue(cell, setting, filters, resData.Result);
    }
    else
    {
        // normal display mode with no reference

        Reference<XCell> xCell = maData.OutputSheetRef->getCellByPosition(cell.Column, cell.Row);
        if (xCell->getType() != table::CellContentType_VALUE)
        {
            fprintf(stdout, "Error: cell not value (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
            return;
        }

        // This ID is the actual column ID of the data field.
        const sal_Int32 nFieldId = setting.FieldId;
        const sheet::GeneralFunction func = setting.Function;
        double val1 = xCell->getValue();
        double val2 = maData.CacheTable.aggregateValue(filters, nFieldId, func);
        if (val1 != val2)
        {
            fprintf(stdout, "Error: values differ (%ld, %ld) : real value = %g  check value = %g (%s)\n", 
                    cell.Row, cell.Column,
                    val1, val2, getFunctionName(func).c_str());
            fflush(stdout);
            ++mnFailureCount;
        }
    }
}

sal_Int16 ResultTester::getFailureCount() const
{
    return mnFailureCount;
}

void ResultTester::verifyRefValue(const CellAddress& cell, 
                                  const DataFieldSetting& setting, 
                                  const vector<DataTable::Filter>& filters,
                                  const DataResult& result)
{
    Reference<XCell> xCell = maData.OutputSheetRef->getCellByPosition(cell.Column, cell.Row);

    const sal_Int32 nFieldId = setting.FieldId;
    const sheet::GeneralFunction func = setting.Function;
    const DataPilotFieldReference& ref = *setting.FieldRef;
    sal_Int32 refFieldId = maData.CacheTable.getFieldIndex(ref.ReferenceField);

    // Obtain the aggregate value with the original filter set.
    double valOrig = maData.CacheTable.aggregateValue(filters, nFieldId, func);

    // Go through the filters and find the field that matches the referenced field, then
    // replace the match value with the referenced item name.
    vector<DataTable::Filter> filters2;
    vector<DataTable::Filter>::const_iterator itr = filters.begin(), itrEnd = filters.end();
    bool isRefItem = false;
    for (; itr != itrEnd; ++itr)
    {
        if (itr->FieldIndex == refFieldId)
        {
            // This is the referenced field.  Replace the match value with
            // the referenced item name.
            DataTable::Filter filter(*itr);
            sal_Int32 newStrId = DataTable::getStringId(ref.ReferenceItemName); 
            if (filter.MatchStrId == newStrId)
                isRefItem = true;
            else
                filter.MatchStrId = newStrId;
            filters2.push_back(filter);
        }
        else
            filters2.push_back(*itr);
    }

    // A row subtotal cell and the referenced itmes are both supposed to be 
    // empty for the following reference types:
    // 
    //   * ITEM_DIFFERENCE
    //   * ITEM_PERCENTAGE
    //   * ITEM_PERCENTAGE_DIFFERENCE
    // 
    // For the following ref type, only the row subtotal cells should be
    // empty.
    // 
    //   * RUNNING_TOTAL
    // 
    // For the following ref types, the result area should be filled in its
    // entirety.
    // 
    //   * ROW_PERCENTAGE
    //   * COLUMN_PERCENTAGE
    //   * TOTAL_PERCENTAGE
    //   * INDEX

    table::CellContentType cellType = xCell->getType();
    CellRangeAddress resRange = mxDPTab->getOutputRangeByType(DataPilotTableRegion::RESULT);
    bool isRowSubTotal = (result.Flags & DataResultFlags::SUBTOTAL) && (resRange.EndColumn == cell.Column);
    double valRef = maData.CacheTable.aggregateValue(filters2, nFieldId, func);
    double valCell = xCell->getValue();

    switch (ref.ReferenceType)
    {
        case DataPilotFieldReferenceType::NONE:
            // no reference mode.
            return;
        case DataPilotFieldReferenceType::ITEM_DIFFERENCE:
        {
            // subtract the reference value and display the difference.

            if (isRefItem || isRowSubTotal)
            {
                // should be empty.
                if (cellType == table::CellContentType_EMPTY)
                    return;
            }

            if (valCell != valOrig - valRef)
            {
                fprintf(stdout, "Error: values differ (%ld, %ld) : real value = %g  check value %g - %g = %g (%s)\n",
                        cell.Row, cell.Column,
                        valCell, valOrig, valRef, valOrig - valRef,
                        getFunctionName(func).c_str());
                ++mnFailureCount;
            }
            return;
        }
        case DataPilotFieldReferenceType::ITEM_PERCENTAGE:
        {
            // each result is dividied by its reference value.
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
        {
            // from each result, its reference value is subtracted, and the difference
            // is further divided by the reference value.
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::RUNNING_TOTAL:
        {
            // Each result is added to the sum of the results for preceding items 
            // in the base field, in the base field's sort order, and the total 
            // sum is shown.
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::ROW_PERCENTAGE:
        {
            // Each result is divided by the total result for its row in 
            // the DataPilot table. 
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::COLUMN_PERCENTAGE:
        {
            // Same as DataPilotFieldReferenceType::ROW_PERCENTAGE , but the total 
            // for the result's column is used. 
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::TOTAL_PERCENTAGE:
        {
            // Same as DataPilotFieldReferenceType::ROW_PERCENTAGE , but the grand 
            // total for the result's data field is used. 
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        case DataPilotFieldReferenceType::INDEX:
        {
            // The row and column totals and the grand total, following the same 
            // rules as above, are used to calculate the following expression.
            fprintf(stdout, "* TEST CODE NOT IMPLEMENTED (%ld, %ld)\n", cell.Row, cell.Column);
            ++mnFailureCount;
        }
        break;
        default:
            fprintf(stdout, "* UNKNOWN REFERENCE TYPE (%ld, %ld)\n",
                    cell.Row, cell.Column);
            ++mnFailureCount;
    }
}

}
