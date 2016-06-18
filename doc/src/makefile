###RUBYROOT	= c:\programs\ruby-1.8.4-i386-mingw32\usr\local
RUBYROOT	= c:\Ruby22
RUBY	= $(RUBYROOT)\bin\ruby -Ku
RD2	= $(RUBYROOT)\bin\rd2 -r rd/rd2html-lib.rb 
RD2OPTS	= --with-css=style/kanchoku.css --with-part=html:include \
	  --html-title=kw128 --html-lang=ja --html-charset=UTF-8

RD_SRC	= intro.rd install.rd usage.rd auxil.rd customize.rd \
	  history.rd changes.rd \
	  kansaku.rd \
	  refurl.rd thanks.rd

# --------------------------------------------------------------------

MANUAL	= kanchoku.html

# --------------------------------------------------------------------

target : $(MANUAL) frame

$(MANUAL) : title.rd toc.rd $(RD_SRC)
	$(RUBY) $(RD2) $(RD2OPTS) title.rd toc.rd $(RD_SRC) > $@
	$(RUBY) postproc.rb $@

toc.rd : $(RD_SRC)
	$(RUBY) mktoc.rb $(RD_SRC) > $@

# --------------------------------------------------------------------

frame : $(MANUAL)
	$(RUBY) mkframe.rb $(MANUAL)

# --------------------------------------------------------------------

help:
	$(RUBY) $(RD2) --help

# XXX
install : target
	- del ..\kanchoku.html
	- del ..\kanchoku_frame.html
	- del ..\kanchoku_frame_toc.html
	- del ..\kanchoku_frame_content.html
	- copy kanchoku.html ..
	- copy kanchoku_frame.html ..
	- copy kanchoku_frame_toc.html ..
	- copy kanchoku_frame_content.html ..

# XXX
clean :
	- del kanchoku.html
	- del kanchoku.html~
	- del kanchoku_frame.html
	- del kanchoku_frame_toc.html
	- del kanchoku_frame_content.html
	- del toc.rd
