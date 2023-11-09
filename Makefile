DIR = ./src/

docfmt:
	cd $(DIR) && make all

%:
	cd $(DIR) && make $@
