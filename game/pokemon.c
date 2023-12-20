#include "g_local.h"

pokemon_t* FindPokemon(char* pokemonName)
{
	int		i;
	pokemon_t* pkmn;

	pkmn = pokemonTeam;
	for (i = 0; i < 2; i++, pkmn++)
	{
		if (!pkmn->pkmnName)
			continue;
		if (!Q_stricmp(pkmn->pkmnName, pokemonName))
			return pkmn;
	}

	return NULL;
}

pokemon_t pokemonTeam[] = {
	{
		300,
		300,
		200,
		200,
		5,
		0,
		20,
		"Charmander"
	}
	,
	{
		400,
		400,
		150,
		150,
		5,
		0,
		20,
		"Squirtle"
	}
	,
	{
		600,
		600,
		100,
		100,
		5,
		0,
		20,
		"Bulbasaur"
	}
	,
	{
		300,
		300,
		170,
		170,
		5,
		0,
		20,
		"Butterfree"
	}
	,
	{
		50,
		50,
		10,
		10,
		3,
		0,
		10,
		"Pikachu"
	}
	,
	{
		200,
		200,
		100,
		100,
		5,
		0,
		20,
		"Psyduck"
	}
	,
	{
		500,
		500,
		50,
		50,
		5,
		0,
		20,
		"Slakoth"
	}
	,
	{
		400,
		400,
		250,
		250,
		7,
		0,
		35,
		"Absol"
	}
	,
	{
		1000,
		1000,
		500,
		500,
		50,
		0,
		250,
		"Deoxys"
	}
	,
	{
		4000,
		4000,
		250,
		250,
		30,
		0,
		200,
		"Snorlax"
	}
};