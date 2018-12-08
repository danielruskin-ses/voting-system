CXX = g++
PROTOC=protoc
CPPFLAGS += -pthread -I../shared -I/usr/local/include -I/usr/include -std=c++11
LDFLAGS += -L/usr/local/lib -lcryptopp -lprotobuf -pthread -lgrpc++ -lsqlite3
SHARED_PATH=../shared
PROTOS_PATH=../protos
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH = /usr/local/bin/grpc_cpp_plugin

vpath %.proto $(PROTOS_PATH)
vpath %.shared.cc $(SHARED_PATH)

all: vote_server

vote_server: Crypto.shared.o shared.pb.o shared.grpc.pb.o vote_server.pb.o vote_server.grpc.pb.o vote_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h vote_server