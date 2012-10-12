ANOPE_SRC := ../anope

build: all
all: m_sasl.so

m_sasl.so:
	${CXX} -fPIC -DPIC -shared -o m_sasl.so m_sasl.cpp -I$(ANOPE_SRC)/include -I$(ANOPE_SRC)/build/include -I$(ANOPE_SRC)/modules/pseudoclients

