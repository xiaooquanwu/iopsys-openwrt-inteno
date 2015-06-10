TARGET:=juci-theme-vodafone
CSS:=htdocs/css/90-$(TARGET).css
JS:=htdocs/js/90-$(TARGET).js
HTML:=htdocs/js/90-$(TARGET)-tpl.js
CP:=cp -Rp
INSTALL_DIR=mkdir -p

all: files $(CSS) $(JS) $(HTML);

.PHONY: files
files: 
	$(INSTALL_DIR) ./htdocs/css
	$(INSTALL_DIR) ./htdocs/js
	$(CP) ./src/img htdocs/
	$(CP) ./src/fonts htdocs/

STYLES:=$(wildcard src/css/*.css) 
CODE:=$(wildcard src/*.js src/pages/*.js src/widgets/*.js)
TEMPLATES:=$(wildcard src/widgets/*.html) $(wildcard src/pages/*.html)

$(CSS): $(STYLES)
	cat $(STYLES) > $@
	gzip $@

$(JS): $(CODE)
	cat $(CODE) > $@
	gzip $@
	
$(HTML): $(TEMPLATES);
	./juci-build-tpl-cache $(TEMPLATES) $(HTML) 
	gzip $@

clean: 
	rm -rf htdocs
