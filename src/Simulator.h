#pragma once
#include <vector>
#include "Card.h"


struct GameSettings
{
	// Table rules
	uint32_t numDecks;
	bool hitSoft17;
	uint32_t numResplits;
	bool rehitAces;
	bool doubleAfterSplit;
	bool surrenderAvailable;
	float dealerCutCardApproxLocation;
	uint32_t tableMinimum;
	uint32_t tableMaximum;

	// Player info
	uint32_t numPlayers;
	uint32_t playerSpot;

	// Computing info
	uint32_t numGames;
	bool simSingleSession;
};


enum class BetLocation : uint32_t
{
	MainGame = 1,
	LL = 2,
	InBetween = 3,
	TwentyOnePlus3 = 4,
	MatchTheDealer = 5
};

struct CounterEntry
{
	float tc;
	float betAmount;
	BetLocation betLocation;
};

struct CounterSettings
{
	std::vector<CounterEntry> entries;
	bool wonging;
	float wongCount;

	uint32_t bankroll;
	uint32_t upperLimit;
	int32_t timeLimit;
};

struct Stats
{
	int32_t moneyWon;
	uint32_t handsWon;
	uint32_t handsLost;
	uint32_t handsPushed;
	uint32_t ruinedCount;
};


class Simulator
{
public:

	Simulator() = delete;
	Simulator(GameSettings* gs, CounterSettings* cs);

	void RunSimulation();

private:

	GameSettings* _gs;
	CounterSettings* _cs;

};

