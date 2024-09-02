lib_MIDDIR:=mid/lib
lib_OUTDIR:=out/lib

lib_CC:=gcc -c -MMD -O3 -Isrc
lib_AR:=ar

lib_CFILES:=$(filter src/lib/%.c,$(SRCFILES))
lib_OFILES:=$(patsubst src/lib/%.c,$(lib_MIDDIR)/%.o,$(lib_CFILES))
-include $(lib_OFILES:.o=.d)
$(lib_MIDDIR)/%.o:src/lib/%.c;$(PRECMD) $(lib_CC) -o$@ $<

lib_STATIC:=$(lib_OUTDIR)/libgamemon.a
$(lib_STATIC):$(lib_OFILES);$(PRECMD) $(lib_AR) rc $@ $^
lib-all:$(lib_STATIC)

lib_HEADER:=$(lib_OUTDIR)/gamemon.h
$(lib_HEADER):src/lib/gamemon.h;$(PRECMD) cp $< $@
lib-all:$(lib_HEADER)
