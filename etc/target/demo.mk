demo_MIDDIR:=mid/demo
demo_OUTDIR:=out/demo

demo_CC:=gcc -c -MMD -O3 -Isrc
demo_LD:=gcc
demo_LDPOST:=

demo_CFILES:=$(filter src/demo/%.c,$(SRCFILES))
demo_OFILES:=$(patsubst src/demo/%.c,$(demo_MIDDIR)/%.o,$(demo_CFILES))
-include $(demo_OFILES:.o=.d)
$(demo_MIDDIR)/%.o:src/demo/%.c;$(PRECMD) $(demo_CC) -o$@ $<

demo_EXE:=$(demo_OUTDIR)/demo
$(demo_EXE):$(demo_OFILES) $(lib_STATIC);$(PRECMD) $(demo_LD) -o$@ $^ $(demo_LDPOST)
demo-all:$(demo_EXE)

demo-run:$(demo_EXE);$(demo_EXE) /dev/$(tiny_PORT) --baud_rate=9600
