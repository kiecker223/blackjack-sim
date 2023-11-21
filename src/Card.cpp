#include "Card.h"
#include "Common.h"


uint8_t Card::GetValue() const
{
	switch (face)
	{
	case Face::None:
		DEBUG_BREAK;
		break;
	case Face::Two:
		return 2;
	case Face::Three:
		return 3;
	case Face::Four:
		return 4;
	case Face::Five:
		return 5;
	case Face::Six:
		return 6;
	case Face::Seven:
		return 7;
	case Face::Eight:
		return 8;
	case Face::Nine:
		return 9;
	case Face::Ten:
	case Face::Jack:
	case Face::Queen:
	case Face::King:
		return 10;
	case Face::Ace:
		return 11;
	}
	return 0;
}

float Card::GetRC() const
{
	switch (face)
	{
	case Face::None:
		DEBUG_BREAK;
		return 0.0f;
	case Face::Two:
	case Face::Three:
	case Face::Four:
	case Face::Five:
	case Face::Six:
		return 1.0f;
	case Face::Ten:
	case Face::Jack:
	case Face::Queen:
	case Face::King:
	case Face::Ace:
		return -1.0f;
	default:
		return 0.0f;
	}
	return 0.0f;
}

void HandBase::CalculateValue()
{
	value = 0;
	numAces = 0;

	for (uint32_t i = 0; i < cardPtr; i++) 
	{
		Card& c = cards[i];
		switch (c.face)
		{
		case Face::None:
			DEBUG_BREAK;
			break;
		case Face::Two:
			value += 2;
			break;
		case Face::Three:
			value += 3;
			break;
		case Face::Four:
			value += 4;
			break;
		case Face::Five:
			value += 5;
			break;
		case Face::Six:
			value += 6;
			break;
		case Face::Seven:
			value += 7;
			break;
		case Face::Eight:
			value += 8;
			break;
		case Face::Nine:
			value += 9;
			break;
		case Face::Ten:
		case Face::Jack:
		case Face::Queen:
		case Face::King:
			value += 10;
			break;
		case Face::Ace:
			// TODO: Optimize this?
			numAces++;
			if (value + 11 <= 21)
			{
				value += 11;
			}
			else
			{
				value += 1;
			}
			break;
		}

		if (value > 21 && numAces > 1)
		{
			value -= 10;
		}
	}
}

bool HandBase::PullCard(uint8_t card)
{
	Card& c = cards[cardPtr] = card;
	uint8_t aces[29] = { 0 };
	uint8_t runningValue = 0;
	cardPtr++;

	switch (c.face)
	{
	case Face::None:
		DEBUG_BREAK;
		break;
	case Face::Two:
		value += 2;
		break;
	case Face::Three:
		value += 3;
		break;
	case Face::Four:
		value += 4;
		break;
	case Face::Five:
		value += 5;
		break;
	case Face::Six:
		value += 6;
		break;
	case Face::Seven:
		value += 7;
		break;
	case Face::Eight:
		value += 8;
		break;
	case Face::Nine:
		value += 9;
		break;
	case Face::Ten:
	case Face::Jack:
	case Face::Queen:
	case Face::King:
		value += 10;
		break;
	case Face::Ace:
		// TODO: Optimize this?
		numAces++;
		if (value + 11 <= 21)
		{
			value += 11;
		}
		else
		{
			value += 1;
		}
		break;
	}

	if (value <= 21)
	{
		return false;
	}
	else
	{
		if (value > 21 && numAces > 1)
		{
			if (value - 10 > 21)
			{
				return true;
			}
			value -= 10;
			return false;
		}
	}

	return true;
}