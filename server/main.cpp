#include "server.hpp"

using namespace server;

int main() {
	KanbanServer server{};
	server.start();
	return 0;
}