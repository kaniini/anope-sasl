ANOPE_SRC := ../anope
SRCS = m_sasl.cpp
OBJS = ${SRCS:.cpp=.so}

.SUFFIXES: .so
.PHONY: clean

.cpp.so:
	${CXX} -fPIC -DPIC -shared -I$(ANOPE_SRC)/include -I$(ANOPE_SRC)/build/include -I$(ANOPE_SRC)/modules/pseudoclients $< -o $@

build: all
all: $(OBJS)

clean:
	rm -f $(OBJS)
