DIR = ./formatter/

formatter:
	cd $(DIR) && make all

%:
	cd $(DIR) && make $@
