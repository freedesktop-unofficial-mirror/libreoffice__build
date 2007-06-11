EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..
PRJNAME=scsolver
TARGET=numeric
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
	$(SLO)$/diff.obj \
	$(SLO)$/funcobj.obj \
	$(SLO)$/cellfuncobj.obj \
	$(SLO)$/lpbase.obj \
	$(SLO)$/lpmodel.obj \
	$(SLO)$/matrix.obj \
	$(SLO)$/nlpbase.obj \
	$(SLO)$/nlpmodel.obj \
	$(SLO)$/nlpqnewton.obj \
	$(SLO)$/lpsolve.obj \
	$(SLO)$/exception.obj \
	$(SLO)$/lineareqn.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

