#include "BjGame.h"
#include <vector>
#include <array>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <random>
#include <chrono>


uint32_t LL(Card dealerUpcards, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

uint32_t InBetween(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

uint32_t TwentyOnePlus3(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

uint32_t MatchTheDealer(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}


BjGame::BjGame(CounterSettings* cs, GameSettings* gs) :
	_gs(gs),
	_cs(cs)
{
	playerSeat = _gs->playerSpot;
	numPlayers = _gs->numPlayers;
	players[playerSeat].bankRoll = _cs->bankroll;
	memcpy(ce, cs->entries.data(), cs->entries.size() * sizeof(CounterEntry));
	counterEntryCount = cs->entries.size();
}


static const std::array<Card, 52> singleDeck = {
	Card(Suit::Diamond, Face::Ace), Card(Suit::Diamond, Face::King), Card(Suit::Diamond, Face::Queen), Card(Suit::Diamond, Face::Jack), Card(Suit::Diamond, Face::Ten), Card(Suit::Diamond, Face::Nine), Card(Suit::Diamond, Face::Eight), Card(Suit::Diamond, Face::Seven), Card(Suit::Diamond, Face::Six), Card(Suit::Diamond, Face::Five), Card(Suit::Diamond, Face::Four), Card(Suit::Diamond, Face::Three), Card(Suit::Diamond, Face::Two),
	Card(Suit::Heart, Face::Ace), Card(Suit::Heart, Face::King), Card(Suit::Heart, Face::Queen), Card(Suit::Heart, Face::Jack), Card(Suit::Heart, Face::Ten), Card(Suit::Heart, Face::Nine), Card(Suit::Heart, Face::Eight), Card(Suit::Heart, Face::Seven), Card(Suit::Heart, Face::Six), Card(Suit::Heart, Face::Five), Card(Suit::Heart, Face::Four), Card(Suit::Heart, Face::Three), Card(Suit::Heart, Face::Two),
	Card(Suit::Spades, Face::Ace), Card(Suit::Spades, Face::King), Card(Suit::Spades, Face::Queen), Card(Suit::Spades, Face::Jack), Card(Suit::Spades, Face::Ten), Card(Suit::Spades, Face::Nine), Card(Suit::Spades, Face::Eight), Card(Suit::Spades, Face::Seven), Card(Suit::Spades, Face::Six), Card(Suit::Spades, Face::Five), Card(Suit::Spades, Face::Four), Card(Suit::Spades, Face::Three), Card(Suit::Spades, Face::Two),
	Card(Suit::Clubs, Face::Ace), Card(Suit::Clubs, Face::King), Card(Suit::Clubs, Face::Queen), Card(Suit::Clubs, Face::Jack), Card(Suit::Clubs, Face::Ten), Card(Suit::Clubs, Face::Nine), Card(Suit::Clubs, Face::Eight), Card(Suit::Clubs, Face::Seven), Card(Suit::Clubs, Face::Six), Card(Suit::Clubs, Face::Five), Card(Suit::Clubs, Face::Four), Card(Suit::Clubs, Face::Three), Card(Suit::Clubs, Face::Two)
};

Stats BjGame::Run()
{
	Stats result = { };
	handPointer = 0;
	deck.resize(_gs->numDecks * 52);

	for (int i = 0; i < 50; i++)
	{
		_ph[i].Reset();
	}

	for (uint32_t i = 0; i < _gs->numDecks; i++)
	{
		std::copy(singleDeck.begin(), singleDeck.end(), deck.begin() + (i * 52));
	}

	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(deck.begin(), deck.end(), std::default_random_engine(seed));

	int minCards = static_cast<int>(_gs->dealerCutCardApproxLocation * 52.0f);


	while (deck.size() > minCards)
	{
		float actual_tc = rc / EstimateDecksRemaining();
		float tc = (floor((actual_tc * 2.0f) + 0.5) / 2.0f);

		for (uint32_t i = 0; i < numPlayers; i++)
		{
			players[i].hands[0] = &_ph[handPointer];
			players[i].numHands = 1;
			handPointer++;
		}


		if (_cs->wonging)
		{
			if (tc <= _cs->wongCount)
			{
				break;
			}
		}

		HandResults res = RunHand(tc);

		if (res.ruined)
		{
			result.ruinedCount += 1;
			GetUserPlayer().bankRoll = _cs->bankroll;
			std::cout << "Lost all bankroll" << std::endl;
			break;
		}

		if (res.win)
		{
			result.handsWon += 1;
		}
		else if (!res.push)
		{
			result.handsLost += 1;
		}
		else if (res.push)
		{
			result.handsPushed += 1;
		}


		_dh.Reset();
		for (int i = 0; i < numPlayers; i++)
		{
			Player& p = players[i];
			for (int j = 0; j < p.numHands; j++)
			{
				if (p.hands[j] != nullptr)
				{
					p.hands[j]->Reset();
				}
			}
			p.numHands = 0;
		}
		handPointer = 0;
	}

	deck.clear();
	rc = 0.0f;
	return result;
}

bool HasBlackjack(Card cardA, Card cardB)
{
	if (cardA.face != Face::Ace && cardB.face != Face::Ace)
	{
		return false;
	}

	Card ace = cardA.face == Face::Ace ? cardA : cardB;
	Card other = cardA.face != Face::Ace ? cardA : cardB;

	switch (other.face)
	{
	case Face::Ten:
	case Face::Jack:
	case Face::Queen:
	case Face::King:
		return true;
	}
	return false;
}

int BjGame::GetBetAmount(float tc, int min, BetLocation bl)
{
	CounterEntry* selectedCE = nullptr;
	for (int i = 0; i < counterEntryCount; i++)
	{
		if (tc > ce[i].tc - 0.001f && tc < ce[i + 1].tc + 0.001f && ce[i].betLocation == bl && ce[i + 1].betLocation == bl)
		{
			selectedCE = &ce[i];
		}
	}
	if (selectedCE == nullptr)
	{
		return min;
	}
	return static_cast<int>(static_cast<float>(min) * selectedCE->betAmount);
}

PlayerAction BjGame::GetPlayerActionH17(DealerHand& dh, PlayerHand* ph, bool canSplit)
{
	bool hasAces = false;
	if (ph->numAces > 0)
	{
		hasAces = true;
	}

	bool canDouble = ph->cardPtr == 2;
	const bool starting = canDouble;
	bool canSurrender = ph->cardPtr == 2 && _gs->surrenderAvailable;
	bool playerSameFace = false;

	if (starting && ph->cards[0].face == Face::Ace && ph->cards[1].face == Face::Ace && canSplit)
	{
		return PlayerAction::Split;
	}

	Card nonAce;

	if (hasAces && starting && canSplit)
	{
		nonAce = ph->cards[0].face != Face::Ace ? ph->cards[0] : ph->cards[1];
	}
	if (starting)
	{
		if (ph->cards[0].face == ph->cards[1].face)
		{
			playerSameFace = true;
			if (ph->cards[0].face == Face::Five)
			{
				playerSameFace = false;
			}
		}
	}

	uint32_t code = (starting ? 1 : 0) + (hasAces ? 2 : 0) + ((playerSameFace && canSplit) ? 4 : 0);

	uint8_t playerValue = ph->value;

	if (playerValue > 21)
	{
		__debugbreak();
		return PlayerAction::Stand;
	}

	// Hard totals
	switch (code)
	{
	case 1: // Starting, no aces and no splits
	{
		switch (dh.cards[1].face)
		{
		case Face::Two: // dealer 2
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				return PlayerAction::Hit;
			case 9:
				return PlayerAction::Hit;
			case 10:
				return PlayerAction::Double;
			case 11:
				return PlayerAction::Double;
			case 12:
				return PlayerAction::Hit;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Three: // dealer 3
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				return PlayerAction::Hit;
			case 9:
			case 10:
			case 11:
				return PlayerAction::Double;
			case 12:
				return PlayerAction::Hit;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Four:
		case Face::Five:
		case Face::Six:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				return PlayerAction::Hit;
			case 9:
			case 10:
			case 11:
				return PlayerAction::Double;
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Seven:
		case Face::Eight:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				return PlayerAction::Hit;
			case 10:
			case 11:
				return PlayerAction::Double;
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Nine:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				return PlayerAction::Hit;
			case 10:
			case 11:
				return PlayerAction::Double;
			case 12:
			case 13:
			case 14:
			case 15:
				return PlayerAction::Hit;
			case 16:
				return PlayerAction::Surrender;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Ten:
		case Face::Jack:
		case Face::Queen:
		case Face::King:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				return PlayerAction::Hit;
			case 11:
				return PlayerAction::Double;
			case 12:
			case 13:
			case 14:
				return PlayerAction::Hit;
			case 15:
			case 16:
				return PlayerAction::Surrender;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Ace:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				return PlayerAction::Hit;
			case 11:
				return PlayerAction::Double;
			case 12:
			case 13:
			case 14:
				return PlayerAction::Hit;
			case 15:
			case 16:
			case 17:
				return PlayerAction::Surrender;
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
				break;
			}
		}
	} break;
	case 3: // Starting and has aces
	{
		switch (dh.cards[1].face)
		{
		case Face::Two: // dealer 2
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
				return PlayerAction::Hit;
			case 18:
				return PlayerAction::Double;
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Three: // dealer 3
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
				return PlayerAction::Double;
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Four:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
				return PlayerAction::Hit;
			case 15:
			case 16:
			case 17:
			case 18:
				return PlayerAction::Double;
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Five:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				return PlayerAction::Double;
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Six:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				return PlayerAction::Double;
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Seven:
		case Face::Eight:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
				return PlayerAction::Hit;
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Nine:
		case Face::Ten:
		case Face::Jack:
		case Face::Queen:
		case Face::King:
		case Face::Ace:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				return PlayerAction::Hit;
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
				break;
			}
		}
	} break;
	case 5: // Starting and splitable hand
	{
		switch (dh.cards[1].face)
		{
		case Face::Two: // dealer 2
		case Face::Three: // dealer 3
		case Face::Four:
			switch (playerValue)
			{
			case 4:
			case 6:
				return PlayerAction::Split;
			case 8:
				return PlayerAction::Hit;
			case 12:
			case 14:
			case 16:
			case 18:
				return PlayerAction::Split;
			}
			break;
		case Face::Five:
		case Face::Six:
			return PlayerAction::Split;
		case Face::Seven:
			switch (playerValue)
			{
			case 4:
			case 6:
				return PlayerAction::Split;
			case 8:
			case 12:
				return PlayerAction::Hit;
			case 14:
			case 16:
				return PlayerAction::Split;
			case 18:
				return PlayerAction::Stand;
			}
			break;
		case Face::Eight:
		case Face::Nine:
			switch (playerValue)
			{
			case 4:
			case 6:
			case 8:
			case 12:
			case 14:
				return PlayerAction::Hit;
			case 16:
			case 18:
				return PlayerAction::Split;
			}
			break;
		case Face::Ten:
		case Face::Jack:
		case Face::Queen:
		case Face::King:
			switch (playerValue)
			{
			case 4:
			case 6:
			case 8:
			case 12:
			case 14:
				return PlayerAction::Hit;
			case 16:
				return PlayerAction::Split;
			case 18:
				return PlayerAction::Stand;
			}
			break;
		case Face::Ace:
			switch (playerValue)
			{
			case 4:
			case 6:
			case 8:
			case 12:
			case 14:
				return PlayerAction::Hit;
			case 16:
				return PlayerAction::Surrender;
			case 18:
				return PlayerAction::Stand;
			}
			break;
		}
	} break;
	default:
	{
		switch (dh.cards[1].face)
		{
		case Face::Two:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				return PlayerAction::Hit;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Three: // dealer 3
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				return PlayerAction::Hit;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Four:
		case Face::Five:
		case Face::Six:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				return PlayerAction::Hit;
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Seven:
		case Face::Eight:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Nine:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Ten:
		case Face::Jack:
		case Face::Queen:
		case Face::King:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
			}
			break;
		case Face::Ace:
			switch (playerValue)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				return PlayerAction::Hit;
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				return PlayerAction::Stand;
				break;
			}
		}
	} break;
	}

	// We shouldn't get here
	return PlayerAction::Stand;
}

PlayerAction BjGame::GetPlayerActionS17(DealerHand& dh, PlayerHand* ph, bool canSplit)
{
	return PlayerAction::Stand;
}

bool BjGame::DealerDraws(DealerHand& dh)
{
	if (_gs->hitSoft17)
	{
		if (dh.value == 17 && dh.numAces > 0)
		{
			uint8_t cardVal = 0;
			for (int i = 0; i < dh.cardPtr; i++)
			{
				Card& c = dh.cards[i];
				if (c.face != Face::Ace)
					cardVal += c.GetValue();
			}
			if (cardVal + dh.numAces == 7)
			{
				return true;
			}
		}
		if (dh.value < 17)
		{
			return true;
		}
		return false;
	}
	else
	{
		if (dh.value >= 17)
		{
			return false;
		}
	}
	return true;
}

Card BjGame::DrawCard()
{
	Card result = deck.back();
	deck.pop_back();
	rc += result.GetRC();
	return result;
}

float BjGame::EstimateDecksRemaining() const
{
	float decksRemaining = static_cast<float>(deck.size()) / 52.0f;
	return (floor((decksRemaining * 2.0f) + 0.5) / 2.0f);
}

HandResults BjGame::RunHand(float tc)
{
	HandResults result = {};
	// Get the player's bet
	int bet = GetBetAmount(tc, _gs->tableMinimum, BetLocation::MainGame);

	if (bet > players[_gs->playerSpot].bankRoll)
	{
		result.win = false;
		result.ruined = true;
		return result;
	}

	for (int i = 0; i < numPlayers; i++)
	{
		players[i].hands[0]->betAmount = static_cast<uint16_t>(bet);
	}

	// First step deal all the cards
	for (int i = 0; i < 2; i++)
	{
		_dh.PullCard(DrawCard());

		for (int j = 0; j < numPlayers; j++)
		{
			for (int k = 0; k < players[j].numHands; k++)
			{
				players[j].hands[k]->PullCard(DrawCard());
			}
		}
	}

	// Check dealer blackjack
	if (HasBlackjack(_dh.cards[0], _dh.cards[1]))
	{
		for (int i = 0; i < numPlayers; i++)
		{
			Player& p = players[i];
			for (int j = 0; j < p.numHands; j++)
			{
				if (HasBlackjack(p.hands[j]->cards[0], p.hands[j]->cards[1]))
				{
					// push
					if (i == playerSeat)
					{
						result.push = true;
						result.win = false;
						result.moneyChange = 0;
					}
				}
				else
				{
					if (i == playerSeat)
					{
						result.win = false;
						result.moneyChange -= bet;
					}
					p.bankRoll -= bet;
				}
			}
		}
		return result;
	}

	// Check player blackjack
	for (int i = 0; i < numPlayers; i++)
	{
		Player& p = players[i];
		for (int j = 0; j < p.numHands; j++)
		{
			PlayerHand* h = p.hands[j];
			if (HasBlackjack(h->cards[0], h->cards[1]))
			{
				p.bankRoll += static_cast<int>(static_cast<float>(bet) * 1.5f);
				h->Reset();
				p.hands[j] = nullptr;
			}
		}
	}

	// Players have played their hands;
	for (int i = 0; i < numPlayers; i++)
	{
		Player& p = players[i];
		uint32_t splitCount = 0;

		bool splitAces = false;
		if (p.hands[0] != nullptr)
		{
			splitAces = p.hands[0]->cards[0].face == Face::Ace && p.hands[0]->cards[1].face == Face::Ace;
		}

		for (int j = 0; j < p.numHands; j++)
		{
			if (p.hands[j] == nullptr)
			{
				continue;
			}

			bool canContinue = true;

			PlayerAction act = PlayerAction::None;
			bool playResult = false;
			do {
				act = GetPlayerActionH17(_dh, p.hands[j], splitCount < _gs->numResplits);
				switch (act)
				{
				case PlayerAction::Hit:
					playResult = p.hands[j]->PullCard(DrawCard());
					break;
				case PlayerAction::Double:
					playResult = p.hands[j]->PullCard(DrawCard());
					p.hands[j]->betAmount *= 2;
					break;
				case PlayerAction::Stand:
					break;
				case PlayerAction::Surrender:
					p.bankRoll -= bet / 2;
					p.hands[j]->Reset();
					p.hands[j] = nullptr;
					break;
				case PlayerAction::Split:
					Card newHandStart = p.hands[j]->cards[1];
					p.hands[j]->cards[1] = Card();
					p.hands[j]->cardPtr--;
					p.hands[j]->CalculateValue();
					
					p.hands[p.numHands] = &_ph[handPointer];
					p.hands[p.numHands]->Reset();
					p.hands[p.numHands]->PullCard(newHandStart);
					p.hands[p.numHands]->PullCard(DrawCard());
					p.hands[p.numHands]->betAmount = bet;
					p.hands[j]->PullCard(DrawCard());

					handPointer++;
					p.numHands++;
					splitCount++;

					if (splitAces && !_gs->rehitAces)
					{
						canContinue = false;
					}

					break;
				}

				if (playResult) break;
			} while ((act != PlayerAction::Stand && act != PlayerAction::Surrender && act != PlayerAction::Double) && canContinue);
			if (playResult)
			{
				p.hands[j]->busted = true;
				p.hands[j]->Reset();
				p.hands[j] = nullptr;
				p.bankRoll -= bet;
				continue;
			}
		}
	}

	bool dealerBust = false;

	while (DealerDraws(_dh))
	{
		dealerBust = _dh.PullCard(DrawCard());

		if (dealerBust)
		{
			for (int i = 0; i < numPlayers; i++)
			{
				Player& p = players[i];
				for (int j = 0; j < p.numHands; j++)
				{
					if (p.hands[j] == nullptr)
					{
						continue;
					}
					else
					{
						p.bankRoll += p.hands[j]->betAmount;
					}
				}
			}

			return result;
		}
	}

	// Dealer didn't bust
	for (int i = 0; i < numPlayers; i++)
	{
		Player& p = players[i];
		for (int j = 0; j < p.numHands; j++)
		{
			if (p.hands[j] == nullptr)
			{
				continue;
			}
			if (_dh.value > p.hands[j]->value)
			{
				// player loses
				p.bankRoll -= p.hands[j]->betAmount;
			}
			else if (_dh.value == p.hands[j]->value)
			{
				// push
				result.push = true;
			}
			else if (_dh.value < p.hands[j]->value)
			{
				// win
				p.bankRoll += p.hands[j]->betAmount;
			}
		}
	}
	return result;
}

