
#include <chrono>
#include <iostream>
#include <thread>

#include <onion/Logger.hpp>

static void Speak()
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << "I count '" << i << "'" << std::endl;

		if (i == 2 || i == 8)
		{
			std::cerr << "One fake error occured." << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	onion::Logger logger("./logs.txt");

	std::cout << " ---- DEMO LOGGER ----" << std::endl;

	std::cout << "This is a 'std::cout'" << std::endl;
	std::cerr << "This is a 'std::cerr'" << std::endl;

	std::jthread t1(Speak);

	std::this_thread::sleep_for(std::chrono::milliseconds(450));

	std::jthread t2(Speak);

	return 0;
}
