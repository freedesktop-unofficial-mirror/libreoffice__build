PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=excel
PACKAGE=org$/openoffice$/vba$/Excel

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

# ------------------------------------------------------------------------

IDLFILES=\
	Constants.idl \
	XlApplicationInternational.idl \
	XlApplyNamesOrder.idl \
	XlArabicModes.idl \
	XlArrangeStyle.idl \
	XlArrowHeadLength.idl \
	XlArrowHeadStyle.idl \
	XlArrowHeadWidth.idl \
	XlAutoFillType.idl \
	XlAutoFilterOperator.idl \
	XlAxisCrosses.idl \
	XlAxisGroup.idl \
	XlAxisType.idl \
	XlBackground.idl \
	XlBarShape.idl \
	XlBordersIndex.idl \
	XlBorderWeight.idl \
	XlBuildInDialog.idl \
	XlCVError.idl \
	XlCalculatedMemberType.idl \
	XlCalculation.idl \
	XlCalculationInterruptKey.idl \
	XlCalculationState.idl \
	XlCategoryType.idl \
	XlCellInsertionMode.idl \
	XlCellType.idl \
	XlChartGallery.idl \
	XlChartItem.idl \
	XlChartLocation.idl \
	XlChartPicturePlacement.idl \
	XlChartPictureType.idl \
	XlChartSplitType.idl \
	XlChartType.idl \
	XlClipboardFormat.idl \
	XlCmdType.idl \
	XlColorIndex.idl \
	XlColumnDataType.idl \
	XlCommandUnderlines.idl \
	XlCommentDisplayMode.idl \
	XlConsolidationFunction.idl \
	XlCopyPictureFormat.idl \
	XlCorruptLoad.idl \
	XlCreator.idl \
	XlCubeFieldType.idl \
	XlCutCopyMode.idl \
	XlDVAlterStyle.idl \
	XlDVType.idl \
	XlDVAlertStyle.idl \
	XlDataLabelPosition.idl \
	XlDataLabelSeparator.idl \
	XlDataLabelsType.idl \
	XlDataSeriesDate.idl \
	XlDataSeriesType.idl \
	XlDeleteShiftDirection.idl \
	XlDirection.idl \
	XlDisplayBlanksAs.idl \
	XlDisplayDrawingObjects.idl \
	XlDisplayUnit.idl \
	XlEditionFormat.idl \
	XlEditionOptionsOption.idl \
	XlEditionType.idl \
	XlEnableCancelKey.idl \
	XlEnableSelection.idl \
	XlEndStyleCap.idl \
	XlErrorBarDirection.idl \
	XlErrorBarInclude.idl \
	XlErrorBarType.idl \
	XlErrorChecks.idl \
	XlFileAccess.idl \
	XlFileFormat.idl \
	XlFillWith.idl \
	XlFilterAction.idl \
	XlFindLookIn.idl \
	XlFormControl.idl \
	XlFormatConditionOperator.idl \
	XlFormatConditionType.idl \
	XlFormulaLabel.idl \
	XlHAlign.idl \
	XlHebrewModes.idl \
	XlHighlightChangesTime.idl \
	XlHtmlType.idl \
	XlIMEMode.idl \
	XlImportDataAs.idl \
	XlInsertFormatOrigin.idl \
	XlInsertShiftDirection.idl \
	XlLayoutFormType.idl \
	XlLegendPosition.idl \
	XlLineStyle.idl \
	XlLink.idl \
	XlLinkInfo.idl \
	XlLinkInfoType.idl \
	XlLinkStatus.idl \
	XlLinkType.idl \
	XlListConflict.idl \
	XlListDataType.idl \
	XlListObjectSourceType.idl \
	XlLocationInTable.idl \
	XlLookAt.idl \
	XlMSApplication.idl \
	XlMailSystem.idl \
	XlMarkerStyle.idl \
	XlMouseButton.idl \
	XlMousePointer.idl \
	XlOLEType.idl \
	XlOLEVerb.idl \
	XlObjectSize.idl \
	XlOrder.idl \
	XlOrientation.idl \
	XlPTSelectionMode.idl \
	XlPageBreak.idl \
	XlPageBreakExtent.idl \
	XlPageOrientation.idl \
	XlPaperSize.idl \
	XlParameterDataType.idl \
	XlParameterType.idl \
	XlPasteSpecialOperation.idl \
	XlPasteType.idl \
	XlPattern.idl \
	XlPhoneticAlignment.idl \
	XlPictureAppearance.idl \
	XlPictureConvertorType.idl \
	XlPivotCellType.idl \
	XlPivotFieldCalculation.idl \
	XlPivotFieldDataType.idl \
	XlPivotFieldOrientation.idl \
	XlPivotFormatType.idl \
	XlPivotTableMissingItems.idl \
	XlPivotTableSourceType.idl \
	XlPivotTableVersionList.idl \
	XlPlacement.idl \
	XlPlatform.idl \
	XlPrintErrors.idl \
	XlPrintLocation.idl \
	XlPriority.idl \
	XlQueryType.idl \
	XlRangeAutoFormat.idl \
	XlRangeValueDataType.idl \
	XlReferenceStyle.idl \
	XlReferenceType.idl \
	XlRobustConnect.idl \
	XlRoutingSlipDelivery.idl \
	XlRoutingSlipStatus.idl \
	XlRowCol.idl \
	XlRunAutoMacro.idl \
	XlSaveAction.idl \
	XlSaveAsAccessMode.idl \
	XlSaveConflictResolution.idl \
	XlScaleType.idl \
	XlSearchDirection.idl \
	XlSearchOrder.idl \
	XlSearchWithin.idl \
	XlSheetType.idl \
	XlSheetVisibility.idl \
	XlSizeRepresents.idl \
	XlSmartTagControlType.idl \
	XlSmartTagDisplayMode.idl \
	XlSortDataOption.idl \
	XlSortMethod.idl \
	XlSortMethodOld.idl \
	XlSortOrder.idl \
	XlSortOrientation.idl \
	XlSortType.idl \
	XlSourceType.idl \
	XlSpeakDirection.idl \
	XlSpecialCellsValue.idl \
	XlSubscribeToFormat.idl \
	XlSubtototalLocationType.idl \
	XlSummaryColumn.idl \
	XlSummaryReportType.idl \
	XlSummaryRow.idl \
	XlTabPosition.idl \
	XlTextParsingType.idl \
	XlTextQualifier.idl \
	XlTextVisualLayoutType.idl \
	XlTickLabelOrientation.idl \
	XlTickLabelPosition.idl \
	XlTimeMark.idl \
	XlTimeUnit.idl \
	XlToolbarProtection.idl \
	XlTotalsCalculation.idl \
	XlTrendlineType.idl \
	XlUnderlineStyle.idl \
	XlUpdateLinks.idl \
	XlVAlign.idl \
	XlWBATemplate.idl \
	XlWebFormatting.idl \
	XlWebSectionType.idl \
	XlWindowState.idl \
	XlWindowType.idl \
	XlWindowView.idl \
	XlXLMMacroType.idl \
	XlXmlExportResult.idl \
	XlXmlImportResult.idl \
	XlXmlLoadOption.idl \
	XlYesNoGuess.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk

