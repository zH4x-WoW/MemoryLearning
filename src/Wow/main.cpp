#include <iostream>
#include <thread>
#include <Windows.h>

int health = 0;

int32_t OnUpdate(float a1)
{	
	health = health + 1;
	printf("[OnUpdate] health = %i\n", health);

	if (health > 800000000)
		health = 0;

	return 1;
}

void Pulse()
{
	printf("[Pulse] Started\n");
	while (1)
	{
		Sleep(1000);
		OnUpdate(1.0f);
	}
}

int main()
{	
	std::thread mainThread(Pulse);

	mainThread.join();
	return 0;
}




