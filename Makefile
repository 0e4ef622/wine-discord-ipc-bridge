OUTPUT ?= winediscordipcbridge.exe


CC := i686-w64-mingw32-gcc
MC := i686-w64-mingw32-windmc
RC := i686-w64-mingw32-windres

<<sources>> := $(wildcard src/*.c)
<<mcs>>     := $(wildcard res/*.mc)
<<rcs>>     := $(wildcard res/*.rc)
<<objects>> := $(<<sources>>:src/%.c=obj/%.o)
<<objects>> += $(<<mcs>>:res/%.mc=obj/%.rc.o)
<<objects>> += $(<<rcs>>:res/%.rc=obj/%.rc.o)

<< := @echo
ifneq ($(shell eval 'echo -e'),-e)
	<< += -e
endif

src/%.h rcs/%.rc : res/%.mc
	$(<<) "  MC\t"$(<)
	@mkdir -p rcs && $(MC) -h src -r rcs $(<)

obj/%.rc.o: res/%.rc
	$(<<) "  RC\t"$(@)
	@$(RC) -I src $(<) $(@)

obj/%.rc.o: rcs/%.rc
	$(<<) "  RC\t"$(@)
	@$(RC) -I src $(<) $(@)

obj/%.o: src/%.c
	$(<<) "  CC\t$(<)"
	@mkdir -p obj && $(CC) -masm=intel -c $(<) -o $(@)

all: $(OUTPUT)

$(OUTPUT): $(<<objects>>)
	$(<<) "LINK\t$(@)"
	@$(CC) $(^) -o $(@) -lshlwapi

clean:
	@rm -fv $(<<objects>>) $(OUTPUT) rcs/* $(<<mcs>>:res/%.mc=src/%.h) $(<<rcs>>:res/%.rc=src/%.h)
	@rm -rf rcs

.PHONY: clean all
.INTERMEDIATE: $(<<mcs>>:res/%.mc=src/%.h) $(<<rcs>>:res/%.rc=src/%.h) $(<<mcs>>:res/%.mc=rcs/%.rc)
