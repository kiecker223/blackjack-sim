#include "BjGame.h"
#include <vector>
#include <array>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <random>
#include <chrono>



/*
* I'm aware all the sidebet payouts vary from casino to casino
* but I'm going to configure these for the casinos I play at 
* to see if there's some sort of advantaged play here
*/

/*
* Callback for if player is going to play lucky ladies at a given count
*/
uint32_t LL(Card dealerUpcards, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

/*
* Callback for if player is going to play in between at a given count
*/
uint32_t InBetween(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

/*
* Callback for if player is going to play in between at a given count
*/
uint32_t TwentyOnePlus3(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

/*
* Callback for if player is going to play in between at a given count
*/
uint32_t MatchTheDealer(Card dealerUpcard, Card cardA, Card cardB, uint32_t inBet)
{
	return 0;
}

static const std::array<Card, 52> singleDeck = {
	Card(Suit::Diamond, Face::Ace), Card(Suit::Diamond, Face::King), Card(Suit::Diamond, Face::Queen), Card(Suit::Diamond, Face::Jack), Card(Suit::Diamond, Face::Ten), Card(Suit::Diamond, Face::Nine), Card(Suit::Diamond, Face::Eight), Card(Suit::Diamond, Face::Seven), Card(Suit::Diamond, Face::Six), Card(Suit::Diamond, Face::Five), Card(Suit::Diamond, Face::Four), Card(Suit::Diamond, Face::Three), Card(Suit::Diamond, Face::Two),
	Card(Suit::Heart, Face::Ace), Card(Suit::Heart, Face::King), Card(Suit::Heart, Face::Queen), Card(Suit::Heart, Face::Jack), Card(Suit::Heart, Face::Ten), Card(Suit::Heart, Face::Nine), Card(Suit::Heart, Face::Eight), Card(Suit::Heart, Face::Seven), Card(Suit::Heart, Face::Six), Card(Suit::Heart, Face::Five), Card(Suit::Heart, Face::Four), Card(Suit::Heart, Face::Three), Card(Suit::Heart, Face::Two),
	Card(Suit::Spades, Face::Ace), Card(Suit::Spades, Face::King), Card(Suit::Spades, Face::Queen), Card(Suit::Spades, Face::Jack), Card(Suit::Spades, Face::Ten), Card(Suit::Spades, Face::Nine), Card(Suit::Spades, Face::Eight), Card(Suit::Spades, Face::Seven), Card(Suit::Spades, Face::Six), Card(Suit::Spades, Face::Five), Card(Suit::Spades, Face::Four), Card(Suit::Spades, Face::Three), Card(Suit::Spades, Face::Two),
	Card(Suit::Clubs, Face::Ace), Card(Suit::Clubs, Face::King), Card(Suit::Clubs, Face::Queen), Card(Suit::Clubs, Face::Jack), Card(Suit::Clubs, Face::Ten), Card(Suit::Clubs, Face::Nine), Card(Suit::Clubs, Face::Eight), Card(Suit::Clubs, Face::Seven), Card(Suit::Clubs, Face::Six), Card(Suit::Clubs, Face::Five), Card(Suit::Clubs, Face::Four), Card(Suit::Clubs, Face::Three), Card(Suit::Clubs, Face::Two)
};


BjGame::BjGame(CounterSettings* cs, GameSettings* gs) :
	_gs(gs),
	_cs(cs),
	handPointer(0),
	rc(0.0f),
	wongCount(0.0f),
	wonging(false),
	deckPointer(0)
{
	playerSeat = _gs->playerSpot;
	numPlayers = _gs->numPlayers;
	players[playerSeat].bankRoll = _cs->bankroll;
	memcpy(ce, cs->entries.data(), cs->entries.size() * sizeof(CounterEntry));
	counterEntryCount = cs->entries.size();
	
	deck.resize(_gs->numDecks * 52);

	// Initialize the deck
	for (uint32_t i = 0; i < _gs->numDecks; i++)
	{
		std::copy(singleDeck.begin(), singleDeck.end(), deck.begin() + (i * 52));
	}
}



Stats BjGame::Run()
{
	Stats result = { };
	handPointer = 0;

	// Reset all the player hands
	for (int i = 0; i < 50; i++)
	{
		_ph[i].Reset();
	}

	// Reset the deck pointer
	deckPointer = 0;

	// Shuffle the deck
	// There might be a better way to do this, but std::shuffle should be fine
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(deck.begin(), deck.end(), std::default_random_engine(seed));

	// Decide where the cut card is
	// TODO: randomly add or subtract some arbitrary value to make the simulation
	// closer to life. Mathematically this should be fine
	int minCards = static_cast<int>(_gs->dealerCutCardApproxLocation * 52.0f);

	// While the cut card hasn't come out
	while (deck.size() > minCards)
	{
		// Get the true count
		float actual_tc = rc / EstimateDecksRemaining();

		// Round down to the nearest .5
		// TODO: This might not be necessary due to how tc
		// feeds into getting the bet
		float tc = (floor((actual_tc * 2.0f) + 0.5) / 2.0f);

		// Allocate our players' hands
		for (uint32_t i = 0; i < numPlayers; i++)
		{
			players[i].hands[0] = &_ph[handPointer];
			players[i].numHands = 1;
			handPointer++;
		}

		// If its time to wong, wong out till the end of the shoe
		if (_cs->wonging)
		{
			if (tc <= _cs->wongCount)
			{
				break;
			}
		}

		// Run the hand
		HandResults res = RunHand(tc);

		// If we ran out of bankroll
		if (res.ruined)
		{
			// Reset our bankroll
			result.ruinedCount += 1;
			GetUserPlayer().bankRoll = _cs->bankroll;
			std::cout << "Lost all bankroll" << std::endl;
			break;
		}

		// Collect stats
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


		// Reset the dealers hand
		_dh.Reset();

		// Reset the players' hands
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

		// Reset our hand pointer
		handPointer = 0;
	}

	// Reset our running count
	rc = 0.0f;

	return result;
}

bool HasBlackjack(Card cardA, Card cardB)
{
	// If neither card is an ace, blackjack isn't possible
	if (cardA.face != Face::Ace && cardB.face != Face::Ace)
	{
		return false;
	}

	// Collect the ace and the other card
	Card ace = cardA.face == Face::Ace ? cardA : cardB;
	Card other = cardA.face != Face::Ace ? cardA : cardB;

	// Check if its a 10
	switch (other.face)
	{
	case Face::Ten:
	case Face::Jack:
	case Face::Queen:
	case Face::King:
		// blackjack
		return true;
	}

	// No blackjack
	return false;
}

/*
* Based on the true count get our bet 
*/
int BjGame::GetBetAmount(float tc, int min, BetLocation bl)
{
	CounterEntry* selectedCE = nullptr;
	for (int i = 0; i < counterEntryCount; i++)
	{
		// If the true count is in between two different entries (plus some slack)
		// select that counter entry and assign our bet
		if (tc > ce[i].tc - 0.001f && tc < ce[i + 1].tc + 0.001f && ce[i].betLocation == bl && ce[i + 1].betLocation == bl)
		{
			selectedCE = &ce[i];
		}
	}
	
	// If we don't have an entry for this tc, then just bet the table min
	if (selectedCE == nullptr)
	{
		return min;
	}

	// Get the bet amount
	return static_cast<int>(static_cast<float>(min) * selectedCE->betAmount);
}

/*
* Get the player's action based on their hand and basic strategy
*/
PlayerAction BjGame::GetPlayerActionH17(DealerHand& dh, PlayerHand* ph, bool canSplit)
{
	// Check for aces
	bool hasAces = false;
	if (ph->numAces > 0)
	{
		hasAces = true;
	}

	// Check the various conditions that would affect
	// what we're doing
	bool canDouble = ph->cardPtr == 2;
	const bool starting = canDouble;
	bool canSurrender = ph->cardPtr == 2 && _gs->surrenderAvailable;
	bool playerSameFace = false;

	// If aces, split
	if (starting && ph->cards[0].face == Face::Ace && ph->cards[1].face == Face::Ace && canSplit)
	{
		return PlayerAction::Split;
	}

	Card nonAce;

	// If we have an ace get our non-ace card
	if (hasAces && starting && canSplit)
	{
		nonAce = ph->cards[0].face != Face::Ace ? ph->cards[0] : ph->cards[1];
	}
	if (starting)
	{
		if (ph->cards[0].face == ph->cards[1].face)
		{
			playerSameFace = true;
			// Cover specific case where we never split 5s
			if (ph->cards[0].face == Face::Five)
			{
				playerSameFace = false;
			}
		}
	}

	// Check which switch-case statement to fall under
	uint32_t code = (starting ? 1 : 0) + (hasAces ? 2 : 0) + ((playerSameFace && canSplit) ? 4 : 0);

	// Get the player's hand value
	uint8_t playerValue = ph->value;

	// If the dealer has busted
	// then __debugbreak because that's a bug and we should
	// never be here
	if (playerValue > 21)
	{
		__debugbreak();
		return PlayerAction::Stand;
	}

	switch (code)
	{
	case 1: // Starting, no aces and no splits, AKA: can double or surrender
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
	default: // No double, no surrender, no split
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
	// but evidently we do
	return PlayerAction::Stand;
}

PlayerAction BjGame::GetPlayerActionS17(DealerHand& dh, PlayerHand* ph, bool canSplit)
{
	return PlayerAction::Stand;
}

/*
* Check if the dealer draws
*/
bool BjGame::DealerDraws(DealerHand& dh)
{
	// Do we hit soft 17?
	if (_gs->hitSoft17)
	{
		// If the dealer has a 17 and they have more than one ace
		if (dh.value == 17 && dh.numAces > 0)
		{
			// Get just the soft total
			uint8_t cardVal = 0;
			for (int i = 0; i < dh.cardPtr; i++)
			{
				Card& c = dh.cards[i];
				if (c.face != Face::Ace)
					cardVal += c.GetValue();
			}

			// If its a soft 17
			if (cardVal + dh.numAces == 7)
			{
				// Hit
				return true;
			}
		}
		// Hit anyway
		if (dh.value < 17)
		{
			return true;
		}
		// Dealer stands
		return false;
	}
	else
	{
		// Pretty easy if we stand on all 17s
		if (dh.value >= 17)
		{
			return false;
		}
	}
	return true;
}

/*
* Pull a card from the deck
*/
Card BjGame::DrawCard()
{
	// Pull a card
	Card result = deck[deckPointer];
	deckPointer++;

	// Get the running count
	rc += result.GetRC();

	return result;
}

/*
* Return approximately how many decks are remaining 
* to closest .5 (how humans would about estimate it)
*/
float BjGame::EstimateDecksRemaining() const
{
	float decksRemaining = static_cast<float>(deck.size()) / 52.0f;
	return (floor((decksRemaining * 2.0f) + 0.5) / 2.0f);
}

/*
* Run a hand of blackjack
*/
HandResults BjGame::RunHand(float tc)
{
	HandResults result = {};

	// Get the player's bet
	int bet = GetBetAmount(tc, _gs->tableMinimum, BetLocation::MainGame);

	// If the bet is greater than our bankroll
	if (bet > players[_gs->playerSpot].bankRoll)
	{
		// We can't make the bet, quit out
		result.win = false;
		result.ruined = true;
		return result;
	}

	// Everyone bets the AP optimal bet
	for (int i = 0; i < numPlayers; i++)
	{
		players[i].hands[0]->betAmount = static_cast<uint16_t>(bet);
	}

	// Deal all the cards
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
				// If the player has blackjack
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
					// Loss
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
	// TODO: Remember the prior check for extra speed
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

	// Players play their hands, all according to 
	// basic strategy
	for (int i = 0; i < numPlayers; i++)
	{
		Player& p = players[i];
		uint32_t splitCount = 0;

		// What
		bool splitAces = false;
		if (p.hands[0] != nullptr)
		{
			splitAces = p.hands[0]->cards[0].face == Face::Ace && p.hands[0]->cards[1].face == Face::Ace;
		}

		// Foreach hand for a player
		for (int j = 0; j < p.numHands; j++)
		{
			// if the hand is null it likely busted
			if (p.hands[j] == nullptr)
			{
				continue;
			}

			// Setup our local vars
			bool canContinue = true;
			PlayerAction act = PlayerAction::None;
			bool playResult = false;

			// Play the hand
			do {
				// Get the player action
				act = GetPlayerActionH17(_dh, p.hands[j], splitCount < _gs->numResplits);
				switch (act)
				{
				case PlayerAction::Hit:
					// Hit, draw a card
					playResult = p.hands[j]->PullCard(DrawCard());
					break;
				case PlayerAction::Double:
					// Double, draw a card and dont allow replay
					playResult = p.hands[j]->PullCard(DrawCard());
					p.hands[j]->betAmount *= 2;
					canContinue = false;
					break;
				case PlayerAction::Stand:
					// Stand is stand
					break;
				case PlayerAction::Surrender:
					// Surrender, lose half our bet, and give up our hand
					p.bankRoll -= bet / 2;
					p.hands[j]->Reset();
					p.hands[j] = nullptr;
					break;
				case PlayerAction::Split:
					// Split, take our cards and create another hand from it
					// then put a bet on each one
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

					// Update the important stuff
					handPointer++;
					p.numHands++;
					splitCount++;

					// Check if we can resplit
					if (splitAces && !_gs->rehitAces)
					{
						canContinue = false;
					}

					break;
				}

				// If we busted, break
				if (playResult) break;
			} while ((act != PlayerAction::Stand && act != PlayerAction::Surrender && act != PlayerAction::Double) && canContinue);

			// If we busted clear out our hand
			// and take our money
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

	// Dealer plays
	// Check if they draw
	while (DealerDraws(_dh))
	{
		dealerBust = _dh.PullCard(DrawCard());

		// Check if the dealer busted after drawing
		if (dealerBust)
		{
			// If they did, everyone with a hand wins
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

