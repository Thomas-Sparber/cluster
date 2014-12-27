CXX=g++-4.7
CFLAGS=-c -g -Wall -Wextra -Weffc++ -std=c++11 -I./include -pedantic -Wdouble-promotion -Wuninitialized -fipa-pure-const -Wsuggest-attribute=const -Wsuggest-attribute=noreturn  -Wtrampolines -Wfloat-equal  -Wunsafe-loop-optimizations -Wc++11-compat -Wcast-qual -Wcast-align -Wzero-as-null-pointer-constant -Wconversion -Wlogical-op -Wredundant-decls -Winline -Wdisabled-optimization
LDFLAGS=-lpthread -L./ -lcluster -lipv4 -lipv6
SOURCES_CLUSTER= \
	src/client.cpp \
	src/p2p.cpp \
	src/server.cpp

SOURCES_IPV4= \
	src/ipv4/ipv4.cpp \
	src/ipv4/ipv4address.cpp \
	src/ipv4/ipv4communicationsocket.cpp \
	src/ipv4/ipv4listenersocket.cpp

SOURCES_IPV6= \
	src/ipv6/ipv6.cpp \
	src/ipv6/ipv6address.cpp \
	src/ipv6/ipv6communicationsocket.cpp \
	src/ipv6/ipv6listenersocket.cpp

SOURCES_MAIN= \
	src/main.cpp \

OBJECTS_CLUSTER=$(SOURCES_CLUSTER:src/%.cpp=bin/%.o)
OBJECTS_IPV4=$(SOURCES_IPV4:src/%.cpp=bin/%.o)
OBJECTS_IPV6=$(SOURCES_IPV6:src/%.cpp=bin/%.o)
OBJECTS_MAIN=$(SOURCES_MAIN:src/%.cpp=bin/%.o)
DEPS_CLUSTER=$(SOURCES_CLUSTER:src/%.cpp=bin/%.d)
DEPS_IPV4=$(SOURCES_IPV4:src/%.cpp=bin/%.d)
DEPS_IPV6=$(SOURCES_IPV6:src/%.cpp=bin/%.d)
DEPS_MAIN=$(SOURCES_MAIN:src/%.cpp=bin/%.d)

all: libcluster.a libipv4.a libipv6.a main 10_0_0_201 10_0_0_202

-include $(DEPS_CLUSTER)
-include $(DEPS_IPV4)
-include $(DEPS_IPV6)
-include $(DEPS_MAIN)

define compileecho
	@tput setaf 4
	@echo $1
	@tput sgr0
endef

define linkecho
      @tput setaf 6
      @echo $1
      @tput sgr0
endef

define cleanecho
      @tput setaf 5
      @echo $1
      @tput sgr0
endef

10_0_0_201: main
	$(call linkecho, "Building 10.0.0.201")
	@scp /home/cubie/cluster/main cubie@10.0.0.201:/home/cubie/main

10_0_0_202:
	$(call linkecho, "Building 10.0.0.202")
	@rsync -r --delete --size-only --exclude '*.o' --exclude '*.a' /home/cubie/cluster pi@10.0.0.202:/home/pi
	@ssh -t pi@10.0.0.202 /home/pi/buildcluster

main: $(OBJECTS_MAIN) libcluster.a libipv4.a libipv6.a
	$(call linkecho, "Linking" $@)
	@$(CXX) -o $@ $(OBJECTS_MAIN) $(LDFLAGS)

libcluster.a: $(OBJECTS_CLUSTER)
	$(call linkecho, "Linking" $@)
	@ar rs $@ $(OBJECTS_CLUSTER)

libipv4.a: $(OBJECTS_IPV4)
	$(call linkecho, "Linking" $@)
	@ar rs $@ $(OBJECTS_IPV4)

libipv6.a: $(OBJECTS_IPV6)
	$(call linkecho, "Linking" $@)
	@ar rs $@ $(OBJECTS_IPV6)

bin/%.o: src/%.cpp
	$(call compileecho, "Compiling" $<)
	@$(CXX) -MD $(CFLAGS) -o $@ $<

clean:
	$(call cleanecho, "Cleaning")
	@rm -rf bin/*.o bin/*.d bin/ipv4/*.o bin/ipv4/*.d bin/ipv6/*.o bin/ipv6/*.d libcluster.a libipv4.o libipv6.a main
