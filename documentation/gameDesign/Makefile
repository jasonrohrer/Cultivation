#
# Modification History
#
# 2006-June-27  Jason Rohrer
# Created.
#


all:  dvi ps pdf


dvi: design.dvi

ps: design.ps

pdf: design.pdf




design.dvi: design.tex
	latex design.tex


design.ps: design.dvi
	dvips -t letter -o design.ps design.dvi

design.pdf: design.ps
	ps2pdf design.ps

