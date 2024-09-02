all:
.SILENT:
.SECONDARY:
PRECMD=echo "  $@" ; mkdir -p $(@D) ;

ifeq ($(MAKECMDGOALS),clean)
clean:;rm -rf mid out
else

SRCFILES:=$(shell find src -type f)

-include etc/config.mk
etc/config.mk:|etc/config.mk.default;$(PRECMD) cp $< $@ ; echo "Please review $@ then 'make' again." ; exit 1

$(foreach T,$(TARGETS),$(eval include etc/target/$T.mk))
all:$(addsuffix -all,$(TARGETS))

endif
