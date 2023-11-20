#pragma once

#include "Simulator.h"
#include <functional>


struct CounterSettings;
struct GameSettings;


struct Player
{
	PlayerHand* hands[8];
	int numHands;
	int bankRoll;
};

struct HandResults
{
	int moneyChange;
	bool win;
	bool push;
	bool ruined;
};

enum class PlayerAction : uint32_t
{
	None,
	Hit,
	Split,
	Double,
	Surrender,
	Stand
};

class BjGame
{
public:

	BjGame() = delete;

	BjGame(CounterSettings* cs, GameSettings* gs);

	Stats Run();

	inline Player& GetUserPlayer()
	{
		return players[_gs->playerSpot];
	}

private:

	HandResults RunHand(float tc);

	int GetBetAmount(float tc, int min, BetLocation bl);

	PlayerAction GetPlayerActionH17(DealerHand& dh, PlayerHand* ph, bool canSplit);
	PlayerAction GetPlayerActionS17(DealerHand& dh, PlayerHand* ph, bool canSplit);

	bool DealerDraws(DealerHand& dh);

	Card DrawCard();

	float EstimateDecksRemaining() const;

	GameSettings* _gs;
	CounterSettings* _cs;

	DealerHand _dh;
	PlayerHand _ph[50];
	uint32_t handPointer;

	float rc;

	Player players[6];

	std::vector<Card> deck;

	uint32_t numPlayers;
	uint32_t playerSeat;
	
	CounterEntry ce[32];
	int counterEntryCount;
	bool wonging;
	float wongCount;
};

