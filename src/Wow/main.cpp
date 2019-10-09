#include <iostream>
#include <thread>
#include <Windows.h>

int health = 0;

void OnUpdate()
{	
	health++;
	printf("[OnUpdate] health = %i\n", health);

	if (health > 800000000)
		health = 0;
}

void Pulse()
{
	printf("[Pulse] Started\n");
	while (1)
	{
		Sleep(1000);
		OnUpdate();
	}
}

int main()
{	
	std::thread mainThread(Pulse);

	mainThread.join();
	return 0;
}




