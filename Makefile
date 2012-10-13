ANOPE_SRC := ../anope
ANOPE_BIN := $(HOME)/services
SRCS = m_sasl.cpp
OBJS = ${SRCS:.cpp=.so}
CXXFLAGS = ${CFLAGS} -Wall -std=gnu++03 -pedantic

.SUFFIXES: .so
.PHONY: clean

.cpp.so:
	${CXX} ${CXXFLAGS} -fPIC -DPIC -shared -I$(ANOPE_SRC)/include -I$(ANOPE_SRC)/build/include -I$(ANOPE_SRC)/modules/pseudoclients $< -o $@

build: all
all: $(OBJS)

clean:
	rm -f $(OBJS)

install:
	@for i in $(OBJS); do \
		echo "Installed $$i."; \
		install -m755 $$i $(ANOPE_BIN)/lib/modules; \
	done

uninstall:
	@for i in $(OBJS); do \
		rm -f $(ANOPE_BIN)/lib/modules/$$i; \
		echo "Deleted $$i."; \
	done
