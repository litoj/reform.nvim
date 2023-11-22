DIR = ./src/

formatter:
	cd $(DIR) && make all

%:
	cd $(DIR) && make $@
