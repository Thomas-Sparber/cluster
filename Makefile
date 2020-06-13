CXX=g++
CFLAGS=-c -g -Wall -Wextra -Weffc++ -std=c++11 -I./include -pedantic -Wdouble-promotion -Wuninitialized -fipa-pure-const -Wtrampolines -Wfloat-equal  -Wunsafe-loop-optimizations -Wc++11-compat -Wcast-qual -Wcast-align -Wzero-as-null-pointer-constant -Wconversion -Wlogical-op -Wredundant-decls -Wshadow -Wint-to-pointer-cast
LDFLAGS=-lpthread -L./ -lcluster
SOURCES_CLUSTER= \
	src/client.cpp \
	src/clustermutex.cpp \
	src/clusterobject.cpp \
	src/clusterobjectdistributed.cpp \
	src/clusterobjectserialized.cpp \
	src/clusterspeedtest.cpp \
	src/database/database.cpp \
	src/database/datavalue.cpp \
	src/database/sqlquery.cpp \
	src/database/sqlquery_createtable.cpp \
	src/database/sqlquery_insertinto.cpp \
	src/database/sqlquery_select.cpp \
	src/database/sqlresult.cpp \
	src/database/table.cpp \
	src/p2p.cpp \
	src/server.cpp \
	src/ipv4/ipv4.cpp \
	src/ipv4/ipv4address.cpp \
	src/ipv4/ipv4communicationsocket.cpp \
	src/ipv4/ipv4listenersocket.cpp \
	src/ipv6/ipv6.cpp \
	src/ipv6/ipv6address.cpp \
	src/ipv6/ipv6communicationsocket.cpp \
	src/ipv6/ipv6listenersocket.cpp

SOURCES_MAIN= \
	src/main.cpp \

OBJECTS_CLUSTER=$(SOURCES_CLUSTER:src/%.cpp=bin/%.o)
OBJECTS_MAIN=$(SOURCES_MAIN:src/%.cpp=bin/%.o)
DEPS_CLUSTER=$(SOURCES_CLUSTER:src/%.cpp=bin/%.d)
DEPS_MAIN=$(SOURCES_MAIN:src/%.cpp=bin/%.d)

ifeq ($(OS),Windows_NT)
	LDFLAGS+=-lWs2_32
endif

all: libcluster.a main

-include $(DEPS_CLUSTER)
-include $(DEPS_MAIN)

define compileecho
	@echo $1
endef

define linkecho
      @echo $1
endef

define cleanecho
      @echo $1
endef

documentation:
	doxygen doc/doxygen.conf
	cd doc/latex && make

main: $(OBJECTS_MAIN) libcluster.a
	$(call linkecho, "Linking" $@)
	@$(CXX) -o $@ $(OBJECTS_MAIN) $(LDFLAGS)

libcluster.a: $(OBJECTS_CLUSTER)
	$(call linkecho, "Linking" $@)
	@ar rs $@ $(OBJECTS_CLUSTER)

bin/%.o: src/%.cpp
	$(call compileecho, "Compiling" $<)
	@$(CXX) -MD $(CFLAGS) -o $@ $<

clean:
	$(call cleanecho, "Cleaning")
	@rm -rf bin/*.o bin/*.d bin/ipv4/*.o bin/ipv4/*.d bin/ipv6/*.o bin/ipv6/*.d bin/database/*.o bin/database/*.d libcluster.a main
