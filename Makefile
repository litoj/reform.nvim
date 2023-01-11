.PHONY: docfmt
docfmt:
	cd ./lua/reform/docfmt/ && gcc main.c -fPIC -O2 -shared -o main.so
