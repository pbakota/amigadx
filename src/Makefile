# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debuglib64
endif
export config

PROJECTS := AmigaDX

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

AmigaDX: 
	@echo "==== Building AmigaDX ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f AmigaDX.make

clean:
	@${MAKE} --no-print-directory -C . -f AmigaDX.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debuglib64"
	@echo "   releaselib64"
	@echo "   debuglib32"
	@echo "   releaselib32"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   AmigaDX"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
