OUTPUT ?= winediscordipcbridge.exe
DEBUG  ?= false


CC := i686-w64-mingw32-gcc
MC := i686-w64-mingw32-windmc
RC := i686-w64-mingw32-windres

STRIP := i686-w64-mingw32-strip

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
	$(<<) "   MC\t"$(<)
	@mkdir -p rcs && $(MC) -h src -r rcs $(<)

obj/%.rc.o: res/%.rc
	$(<<) "   RC\t"$(@)
	@$(RC) -I src $(<) $(@)

obj/%.rc.o: rcs/%.rc
	$(<<) "   RC\t"$(@)
	@$(RC) -I src $(<) $(@)

obj/%.o: src/%.c
	$(<<) "   CC\t$(<)"
	@mkdir -p obj && $(CC) -masm=intel -c $(<) -o $(@)

all: $(OUTPUT)

obj/service-manager.o: src/error.h

$(OUTPUT): $(<<objects>>)
	$(<<) " LINK\t$(@)"
	@$(CC) $(^) -o $(@) -lshlwapi
	@test "$(DEBUG)" = "true" || $(<<:@%=%) "STRIP\t$(@)"
	@test "$(DEBUG)" = "true" || $(STRIP) $(@)

clean:
	@rm -fv $(<<objects>>) rcs/* $(<<mcs>>:res/%.mc=src/%.h) $(<<rcs>>:res/%.rc=src/%.h) $(OUTPUT)
	@rm -rf rcs obj

.PHONY: clean all
.INTERMEDIATE: $(<<mcs>>:res/%.mc=src/%.h) $(<<rcs>>:res/%.rc=src/%.h) $(<<mcs>>:res/%.mc=rcs/%.rc)
