EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..
PRJNAME=scsolver
TARGET=ui
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME = scsolver
SRC1FILES = scsolver.src

SLOFILES =  \
	$(SLO)$/basedlg.obj \
	$(SLO)$/baselistener.obj \
	$(SLO)$/dialog.obj \
	$(SLO)$/optiondlg.obj \
	$(SLO)$/listener.obj \
	$(SLO)$/lpbuilder.obj \
	$(SLO)$/nlpbuilder.obj \
	$(SLO)$/msgdlg.obj \
	$(SLO)$/resmgr.obj \
	$(SLO)$/solvemodel.obj \
	$(SLO)$/unoglobal.obj \
	$(SLO)$/unohelper.obj \
	$(SLO)$/xcalc.obj \
	$(SLO)$/option.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

