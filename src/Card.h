#pragma once
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include "Common.h"



enum class Suit : uint8_t 
{
	None = 0,
	Diamond = 1,
	Heart = 2,
	Spades = 3,
	Clubs = 4
};

enum class Face : uint8_t 
{
	None = 1,
	Two = 2,
	Three = 3,
	Four = 4,
	Five = 5,
	Six = 6,
	Seven = 7,
	Eight = 8,
	Nine = 9,
	Ten = 10,
	Jack = 11,
	Queen = 12,
	King = 13,
	Ace = 14,
};

struct Card
{
	Card() : 
		v(0)
	{ }

	// Constructors
	Card(uint8_t val)
	{
		v = val;
	}

	Card(Suit s, Face f) :
		suit(s),
		face(f)
	{
	}

	Card(Card& other) :
		v(other.v)
	{
	}


	// operators
	inline Card& operator = (uint8_t val)
	{
		v = val;
		return *this;
	}

	inline Card& operator = (const Card& o)
	{
		v = o.v;
		return *this;
	}

	inline operator uint8_t&()
	{
		return v;
	}

	/*
	* Check if we have all valid stuff
	*/
	inline bool IsValid() const
	{
		return static_cast<uint8_t>(suit) != 0 && static_cast<uint8_t>(suit) < 5 &&
			static_cast<uint8_t>(face) != 0 && static_cast<uint8_t>(face) < 15;
	}

	/*
	* Get the actual value of the card
	*/
	uint8_t GetValue() const;

	/*
	* 
	*/
	float GetRC() const;

	union {
		struct {
			Suit suit : 4;
			Face face : 4;
		};
		uint8_t v;
	};
};


static_assert(sizeof(Card) == 1, "sizeof(Card) isn't returning 1");

struct HandBase
{
	union {
		struct {
			Card cards[25];
			bool busted : 8;
			uint8_t value;
			uint8_t cardPtr;
			uint8_t numAces;
			uint8_t playerId;
			uint16_t betAmount;
		};
		__m128i v[2]; 
	};

	HandBase()
	{
		Reset();
	}

	inline void Reset()
	{
		v[0] = _mm_setzero_si128();
		v[1] = _mm_setzero_si128();
	}

	void CalculateValue();

	bool PullCard(uint8_t card);
};

static_assert(sizeof(HandBase) == 32, "sizeof(HandBase) isn't returning 32");

typedef HandBase PlayerHand;
typedef HandBase DealerHand;

