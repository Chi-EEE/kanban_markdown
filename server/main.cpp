#include <iostream>
#include <yyjson.h>

int main() {
	std::string input;
	while (true)
	{
		std::cin >> input;
		if (input == "exit")
		{
			break;
		}
		std::cout << input << '\n';
	}

	return 0;
}