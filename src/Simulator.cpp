#include "Simulator.h"
#include "BjGame.h"
#include <iostream>




Simulator::Simulator(GameSettings* gs, CounterSettings* cs) :
	_gs(gs),
	_cs(cs)
{
}

void Simulator::RunSimulation()
{
	BjGame g(_cs, _gs);
	int64_t moneyWon = 0;
	Stats s = { };
	Stats c = { };
	for (int i = 0; i < _gs->numGames; i++)
	{
		if ((i % 10000) == 0)
		{
			std::cout << "Games played: " << i << std::endl;
			std::cout << "Player bankroll: " << g.GetUserPlayer().bankRoll << std::endl;
		}
		s = g.Run();
		c.moneyWon += s.moneyWon;
		c.handsWon += s.handsWon;
		c.handsLost += s.handsLost;
		c.handsPushed += s.handsPushed;
		c.ruinedCount += s.ruinedCount;
	}
	std::cout << "Player bankroll: " << g.GetUserPlayer().bankRoll << std::endl;

	std::cout << "moneyWon: " << c.moneyWon << std::endl;
	std::cout << "handsWon: " << c.handsWon << std::endl;
	std::cout << "handsLost: " << c.handsLost << std::endl;
	std::cout << "handsPushed: " << c.handsPushed << std::endl;
	std::cout << "ruinedCount: " << c.ruinedCount << std::endl;

}
