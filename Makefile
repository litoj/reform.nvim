DIR = ./src/

docfmt:
	cd $(DIR) && make docfmt

%:
	cd $(DIR) && make $@
