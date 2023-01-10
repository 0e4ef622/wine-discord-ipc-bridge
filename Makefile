OUTPUT ?= winediscordipcbridge.exe


CC := i686-w64-mingw32-gcc

<<sources>> := $(wildcard src/*.c)
<<objects>> := $(<<sources>>:src/%.c=obj/%.o)

<< := @echo
ifneq ($(shell eval 'echo -e'),-e)
	<< += -e
endif

obj/%.o: src/%.c
	$(<<) "CC\t$(<)"
	@mkdir -p obj && $(CC) -masm=intel -c $(<) -o $(@)

all: $(OUTPUT)

$(OUTPUT): $(<<objects>>)
	$(<<) "LINK\t$(@)"
	@$(CC) $(^) -o $(@) -lshlwapi

clean:
	@rm -fv $(<<objects>>) $(OUTPUT)

.PHONY: clean all
