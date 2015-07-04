DATE=`date +"%Y-%m-%d"`
ARCH=`uname -a | rev | cut -d ' ' -f 2 | rev`

CC=g++
ROOT_DIR=$(HOME)/devel

GMOCK_DIR=$(ROOT_DIR)/gmock
GTEST_DIR=$(GMOCK_DIR)/gtest

GMOCK_INCLUDE_DIR=-isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
GMOCK_LIB=-lgmock
GMOCK_FLAGS=-pthread


INCLUDE_DIR=-I$(ROOT_DIR)

LIBRARY_DIR=-L$(ROOT_DIR)/libs
LIBRARIES=-lz -lcrypto -lssl

PROFILING_FLAGS=-pg 
OBJECT_FLAGS=-c -Wall -std=c++11 -O3 -ggdb
EXECUTABLE_FLAGS=-Wall -std=c++11 -O3 -ggdb -pthread
TEST_EXE_FLAGS=-Wall -std=c++11 -O3 -ggdb -pthread

SOURCES=NarshaMain.cpp HttpServer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=HttpClient

UNIT_TEST_OFF=_UNIT_TEST false
UNIT_TEST_ON=_UNIT_TEST true

DEBUG_OFF=_DEBUG false
DEBUG_ON=_DEBUG true

UNITTEST= \
	rm -f $(1)-Test-$(DATE)-$(ARCH).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m module Unit Test..."; \
	echo "  Enabling UnitTest by modifying the UNIT TEST LINE."; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_OFF}/${DEBUG_ON}/g" $(1).hpp; \
	$(CC) $(TEST_EXE_FLAGS) $(INCLUDE_DIR) $(GMOCK_INCLUDE_DIR) $(LIBRARY_DIR) $(1).cpp $(2) $(GMOCK_LIB) $(LIBRARIES) -o "$(1)-Test-$(DATE)-$(ARCH).exe" ; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	read -p "Press 'N' key to stop and other keys to continue... " -n 1 REPLY; \
	echo -e "\n\e[1;34m=============== RUNNING TEST ===============\e[0m"; \
	gdb ./$(1)-Test-$(DATE)-$(ARCH).exe; \
	echo -e "\e[1;31m=============== END OF TEST ===============\e[0m\n"; \

GMOCK_TEST= \
	sed -i.bak "s/${UNIT_TEST_OFF}/${UNIT_TEST_ON}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_OFF}/${DEBUG_ON}/g" $(1).hpp; \
	$(CC) $(EXECUTABLE_FLAGS) $(GMOCK_FLAGS) $(INCLUDE_DIR) $(GMOCK_INCLUDE_DIR) $(LIBRARY_DIR) $(1).cpp $(2) $(LIBRARIES) $(GMOCK_LIB) -o "$(1)-Test-$(DATE)-$(ARCH).exe"; \
	sed -i.bak "s/${UNIT_TEST_ON}/${UNIT_TEST_OFF}/g" $(1).cpp; \
	sed -i.bak "s/${DEBUG_ON}/${DEBUG_OFF}/g" $(1).hpp; \
	./$(1)-Test-$(DATE)-$(ARCH).exe

COMPILE= \
	rm -f $(1)-$(DATE)-$(ARCH).exe; \
	echo -e "\n\e[1;32m=============== START COMPILING ===============\e[0m"; \
	echo -e "Compiling \e[1;36m$(1)\e[0m..."; \
	echo -e "\e[1;34m=============== COMPILER MESSAGE ===============\e[0m"; \
	$(CC) $(EXECUTABLE_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $(3).cpp $(2) $(LIBRARIES) -o $(1)-$(DATE)-$(ARCH).exe ; \
	echo -e "\e[1;33m=============== COMPILER MESSAGE END ===============\e[0m"; \
	echo -e "\nDONE: \e[1;33m$@\e[0m."

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(EXECUTABLE_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $(OBJECTS) $(LIBRARIES) -o $@

TestInvitations: $(ROOT_DIR)/liolib/Util.o
	rm -rf ./testdata/invitations/*
	@$(call UNITTEST,Invitations,$^,Invitations)

TestLives: $(ROOT_DIR)/liolib/Util.o
	@$(call UNITTEST,Lives,$^,Lives)

TestSessions: $(ROOT_DIR)/liolib/Util.o
	@$(call UNITTEST,$@,$^,Sessions)

TestTable: Table.o $(ROOT_DIR)/liolib/Util.o
	@$(call UNITTEST,$@,$^,TestTable)



HttpServer: Logger.o $(ROOT_DIR)/liolib/AsyncIo.o $(ROOT_DIR)/liolib/Inotify.o $(ROOT_DIR)/liolib/Util.o $(ROOT_DIR)/liolib/AsyncSockets.o $(ROOT_DIR)/liolib/Socket.o $(ROOT_DIR)/liolib/http/Http.o
	rm -f HttpServer-*.exe
	@$(call COMPILE,$@,$^,HttpServer)

Worker: ILioData.o Invitations.o Lives.o Sessions.o HttpRequest.o HttpResponse.o HttpRequestPool.o HttpResponsePool.o HttpConnection.o AppCore.o $(ROOT_DIR)/liolib/Gzip.o $(ROOT_DIR)/liolib/MemoryPool.o $(ROOT_DIR)/liolib/AsyncIo.o $(ROOT_DIR)/liolib/Inotify.o $(ROOT_DIR)/liolib/Util.o $(ROOT_DIR)/liolib/Logger.o $(ROOT_DIR)/liolib/AsyncSocket.o $(ROOT_DIR)/liolib/Socket.o $(ROOT_DIR)/liolib/http/Http.o
	@$(call COMPILE,$@,$^,Worker)
	mv Worker-*.exe ./workers/HttpWorker.exe

.cpp.o:
	@echo -e "Compiling \e[1;33m$<\e[0m for dependency..."
	@$(CC) $(OBJECT_FLAGS) $(INCLUDE_DIR) $(LIBRARY_DIR) $< $(LIBRARIES) -o $@

clean:
	@rm -rf *.o *.exe *.bak
	@echo "Object files and Executables are removed!"
