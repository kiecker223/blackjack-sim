#include <iostream>
#include "Simulator.h"




int main(int argc, char** argv)
{
	GameSettings g;

	g.numDecks = 8;
	g.hitSoft17 = true;
	g.numResplits = 3;
	g.rehitAces = false;
	g.doubleAfterSplit = true;
	g.surrenderAvailable = true;
	g.dealerCutCardApproxLocation = 2;
	g.tableMinimum = 25;
	g.tableMaximum = 2500;
	g.numPlayers = 1;
	g.playerSpot = 0;
	g.numGames = 1000000;
	g.simSingleSession = false;


	CounterSettings c;

	c.entries.push_back({ 0.0f, 1.0f, BetLocation::MainGame });
	c.entries.push_back({ 1.5f, 2.0f, BetLocation::MainGame });
	c.entries.push_back({ 2.5f, 3.0f, BetLocation::MainGame });
	c.entries.push_back({ 3.5f, 4.0f, BetLocation::MainGame });
	c.entries.push_back({ 4.5f, 5.0f, BetLocation::MainGame });
	c.entries.push_back({ 5.5f, 6.0f, BetLocation::MainGame });
	c.entries.push_back({ 6.5f, 7.0f, BetLocation::MainGame });
	c.entries.push_back({ 7.5f, 8.0f, BetLocation::MainGame });
	c.entries.push_back({ 8.5f, 9.0f, BetLocation::MainGame });
	c.entries.push_back({ 9.5f, 10.0f, BetLocation::MainGame });
	c.entries.push_back({ 10.5f, 11.0f, BetLocation::MainGame });
	c.entries.push_back({ 11.5f, 12.0f, BetLocation::MainGame });
	c.entries.push_back({ 12.5f, 13.0f, BetLocation::MainGame });

	c.wonging = true;
	c.wongCount = -1.0f;
	c.bankroll = 25000;

	Simulator s(&g, &c);
	s.RunSimulation();
}

