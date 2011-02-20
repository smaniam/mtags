all:
	@mkdir -p build
	@cd build; cmake ..
	@make -C build

clean:
	@rm -rf build

install: all
	@make install -C build

.PHONY: all install
