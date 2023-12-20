/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"


char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		sprintf(st, "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


void command_rocket_jump(edict_t* self)
{
	vec3_t dir = {0,0,-1};
	if (!self) {
		gi.dprintf(" % s: % i : no identity provided", __FILE__, __LINE__);
		return;
	}
	if (self -> health <= 0) {
		gi.dprintf(" % s: % i : already dead", __FILE__, __LINE__);
		return;
	}
	fire_rocket(self, self->s.origin, dir, 50, 8000, 120, 120);
}

void Cmd_SelectPokemon_f(edict_t* ent)
{
	char* pokemon;
	int			index;
	gitem_t* grenade;
	char* s;

	s = "grenades";
	grenade = FindItem(s);
	if (!grenade)
	{
		gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!grenade->use)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(grenade);
	if (!ent->client->pers.inventory[index])
	{
		ent->client->pers.inventory[index]++;
	}

	grenade->use(ent, grenade);

	pokemon = gi.args();
	if (!Q_stricmp(pokemon, "charmander")) {
		if (ent->client->grenadeType == GRENADE_CHARMANDER)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_CHARMANDER;
			selectedPokemon = &pokemonTeam[GRENADE_CHARMANDER];
		}
	}
	else if (!Q_stricmp(pokemon, "squirtle")) {
		if (ent->client->grenadeType == GRENADE_SQUIRTLE)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_SQUIRTLE;
			selectedPokemon = &pokemonTeam[GRENADE_SQUIRTLE];
		}
	}
	else if (!Q_stricmp(pokemon, "bulbasaur")) {
		if (ent->client->grenadeType == GRENADE_BULBASAUR)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_BULBASAUR;
			selectedPokemon = &pokemonTeam[GRENADE_BULBASAUR];
		}
	}
	else if (!Q_stricmp(pokemon, "butterfree")) {
		if (ent->client->grenadeType == GRENADE_BUTTERFREE)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_BUTTERFREE;
			selectedPokemon = &pokemonTeam[GRENADE_BUTTERFREE];
		}
	}
	else if (!Q_stricmp(pokemon, "pikachu")) {
		if (ent->client->grenadeType == GRENADE_PIKACHU)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_PIKACHU;
			selectedPokemon = &pokemonTeam[GRENADE_PIKACHU];
		}
	}
	else if (!Q_stricmp(pokemon, "psyduck")) {
		if (ent->client->grenadeType == GRENADE_PSYDUCK)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_PSYDUCK;
			selectedPokemon = &pokemonTeam[GRENADE_PSYDUCK];
		}
	}
	else if (!Q_stricmp(pokemon, "slakoth")) {
		if (ent->client->grenadeType == GRENADE_SLAKOTH)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_SLAKOTH;
			selectedPokemon = &pokemonTeam[GRENADE_SLAKOTH];
		}
	}
	else if (!Q_stricmp(pokemon, "absol")) {
		if (ent->client->grenadeType == GRENADE_ABSOL)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_ABSOL;
			selectedPokemon = &pokemonTeam[GRENADE_ABSOL];
		}
	}
	else if (!Q_stricmp(pokemon, "deoxys")) {
		if (ent->client->grenadeType == GRENADE_DEOXYS)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_DEOXYS;
			selectedPokemon = &pokemonTeam[GRENADE_DEOXYS];
		}
	}
	else if (!Q_stricmp(pokemon, "snorlax")) {
		if (ent->client->grenadeType == GRENADE_SNORLAX)
		{
			gi.cprintf(ent, PRINT_HIGH, "%s already selected.\n", pokemon);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "%s selected.\n", pokemon);
			ent->client->grenadeType = GRENADE_SNORLAX;
			selectedPokemon = &pokemonTeam[GRENADE_SNORLAX];
		}
	}
	else {
		gi.cprintf(ent, PRINT_HIGH, "%s does not exist.\n", pokemon);
	}
}

void Cmd_RetreatPokemon_f(edict_t* ent)
{
	if (currentPokemon)
	{
		gi.cprintf(ent, PRINT_HIGH, "%s retreated. Health: % i \n", currentPokemon->pokemon->pkmnName, currentPokemon->health);
		currentPokemon->pokemon->health = currentPokemon->health;
		currentPokemon->health = -500;
		currentPokemon->die(currentPokemon,ent,ent,100,currentPokemon->pos1);
		//gi.dprintf(" health:  % i \n", currentPokemon->health);
		currentPokemon = NULL;
		isSpawned = false;
	}
}

void Cmd_SpawnEnemy_f(edict_t* ent)
{
	edict_t* monster;

	monster = G_Spawn();
	SP_monster_chick(monster);
	monster->s.origin[0] = ent->s.origin[0];
	monster->s.origin[1] = ent->s.origin[1] + 100;
	monster->s.origin[2] = ent->s.origin[2] + 50;
	//VectorCopy(ent->s.origin, monster->s.origin);

	monster->s.angles[1] = ent->s.angles[1];
	monster->s.frame = 0;
	monster->monsterinfo.nextframe = 0;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_WIDOWSPLASH);
	gi.WritePosition(ent->s.origin);
	gi.multicast(ent->s.origin, MULTICAST_PHS);
	pkmnEnemy = monster;
}

void Cmd_SwitchPokemonAttack_f(edict_t* ent)
{
	if (currentPokemon)
	{
		currentPokemon->isAttack1 = !currentPokemon->isAttack1;
		if (currentPokemon->isAttack1) {
			gi.cprintf(ent, PRINT_HIGH, "%s uses attack 1.\n", currentPokemon->pokemon->pkmnName);
		}
		else {
			gi.cprintf(ent, PRINT_HIGH, "%s uses attack 2.\n", currentPokemon->pokemon->pkmnName);
		}
		
	}
}

void Cmd_RandomPokemonAttack_f(edict_t* ent)
{
	if (currentPokemon)
	{
		currentPokemon->isRandomAttack = !currentPokemon->isRandomAttack;
		if (currentPokemon->isRandomAttack) {
			gi.cprintf(ent, PRINT_HIGH, "%s switched to random attacks.\n", currentPokemon->pokemon->pkmnName);
		}
		else {
			gi.cprintf(ent, PRINT_HIGH, "%s switched to non random attacks.\n", currentPokemon->pokemon->pkmnName);
			if (currentPokemon->isAttack1) {
				gi.cprintf(ent, PRINT_HIGH, "%s uses attack 1.\n", currentPokemon->pokemon->pkmnName);
			}
			else {
				gi.cprintf(ent, PRINT_HIGH, "%s uses attack 2.\n", currentPokemon->pokemon->pkmnName);
			}
		}
	}
}

void Cmd_UsePokemonItem_f(edict_t* ent)
{
	char* item;
	char* pkmnName;

	if (selectedPokemon)
	{
		pkmnName = selectedPokemon->pkmnName;
		item = gi.args();
		if (!Q_stricmp(item, "potion")) {
			if (selectedPokemon->health <= 0) {
				gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is dead.\n", item, pkmnName, pkmnName);
				return;
			}
			else if (selectedPokemon->health >= selectedPokemon->max_health) {
				gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is already at full health.\n", item, pkmnName, pkmnName);
				return;
			}
			else if (selectedPokemon->health + 100 > selectedPokemon->max_health) {
				gi.cprintf(ent, PRINT_HIGH, "%s healed %s %ihp.\n", item, pkmnName, selectedPokemon->max_health - selectedPokemon->health);
				selectedPokemon->health = selectedPokemon->max_health;
			}
			else {
				gi.cprintf(ent, PRINT_HIGH, "%s healed %s %ihp.\n", item, pkmnName, 100);
				selectedPokemon->health += 100;
			}
			if (currentPokemon) {
				if (!Q_stricmp(pkmnName, currentPokemon->pokemon->pkmnName)) {
					currentPokemon->pokemon->health = selectedPokemon->health;
					currentPokemon->health = selectedPokemon->health;
				}
			}
		}
		else if (!Q_stricmp(item, "revive")) {
			if (selectedPokemon->health > 0) {
				gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is not dead.\n", item, pkmnName, pkmnName);
				return;
			}
			else {
				gi.cprintf(ent, PRINT_HIGH, "%s used on %s. %s is alive with %ihp.\n", item, pkmnName, pkmnName, selectedPokemon->max_health/2);
				selectedPokemon->health = selectedPokemon->max_health / 2;
			}
		}
		else if (!Q_stricmp(item, "x-attack")) {
			if (selectedPokemon->pkmnAttack > selectedPokemon->max_pkmnAttack) {
				gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because the attack is already doubled.\n", item, pkmnName);
			}
			else {
				gi.cprintf(ent, PRINT_HIGH, "%s used on %s. Attack will be doubled until the next enemy is killed.\n", item, pkmnName);
				selectedPokemon->pkmnAttack = selectedPokemon->pkmnAttack * 2;
			}
		}
		else if (!Q_stricmp(item, "rarecandy")) {
			selectedPokemon->pkmnXp = 0;
			selectedPokemon->pkmnLevel++;
			selectedPokemon->max_health += 5;
			selectedPokemon->health += 5;
			selectedPokemon->max_pkmnXp += 3;
			selectedPokemon->max_pkmnAttack += 5;
			selectedPokemon->pkmnAttack += 5;
			gi.cprintf(ent, PRINT_HIGH, "%s used on %s. %s reached level %i!\n", item, pkmnName, pkmnName, selectedPokemon->pkmnLevel);
			if (currentPokemon) {
				if (!Q_stricmp(pkmnName, currentPokemon->pokemon->pkmnName)) {
					currentPokemon->pokemon->health = selectedPokemon->health;
					currentPokemon->health = selectedPokemon->health;
				}
			}
		}
		else if (!Q_stricmp(item, "protect")) {
			if (selectedPokemon->isInvincible) {
				gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is already invincible.\n", item, pkmnName, pkmnName);
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "%s used on %s. %s will be invincible until the next enemy is killed.\n", item, pkmnName, pkmnName);
				selectedPokemon->isInvincible = true;
			}
		}
	}
}

//void UsePotion(edict_t* ent, char* item, char* pkmnName)
//{
//	if(selectedPokemon->health <= 0) {
//		gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is dead.\n", item, pkmnName, pkmnName);
//		return;
//	}
//	else if (selectedPokemon->health >= selectedPokemon->max_health) {
//		gi.cprintf(ent, PRINT_HIGH, "%s can not be used on %s because %s is already at full health.\n", item, pkmnName, pkmnName);
//		return;
//	}
//	else if (selectedPokemon->health + 100 > selectedPokemon->max_health) {
//		gi.cprintf(ent, PRINT_HIGH, "%s healed %s %i hp.\n", item, pkmnName, selectedPokemon->max_health - selectedPokemon->health);
//		selectedPokemon->health = selectedPokemon->max_health;
//	}
//	else {
//		gi.cprintf(ent, PRINT_HIGH, "%s healed %s %i hp.\n", item, pkmnName, 100);
//		selectedPokemon->health += 100;
//	}
//	if (currentPokemon) {
//		if (!Q_stricmp(pkmnName, currentPokemon->pokemon->pkmnName)) {
//			currentPokemon->pokemon->health = selectedPokemon->health;
//		}
//	}
//}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp(cmd, "rocket_jump") == 0)
	{
		command_rocket_jump(ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp(cmd, "use") == 0)
		Cmd_Use_f(ent);
	else if (Q_stricmp(cmd, "drop") == 0)
		Cmd_Drop_f(ent);
	else if (Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if (Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if (Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if (Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if (Q_stricmp(cmd, "inven") == 0)
		Cmd_Inven_f(ent);
	else if (Q_stricmp(cmd, "invnext") == 0)
		SelectNextItem(ent, -1);
	else if (Q_stricmp(cmd, "invprev") == 0)
		SelectPrevItem(ent, -1);
	else if (Q_stricmp(cmd, "invnextw") == 0)
		SelectNextItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invprevw") == 0)
		SelectPrevItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invnextp") == 0)
		SelectNextItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invprevp") == 0)
		SelectPrevItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invuse") == 0)
		Cmd_InvUse_f(ent);
	else if (Q_stricmp(cmd, "invdrop") == 0)
		Cmd_InvDrop_f(ent);
	else if (Q_stricmp(cmd, "weapprev") == 0)
		Cmd_WeapPrev_f(ent);
	else if (Q_stricmp(cmd, "weapnext") == 0)
		Cmd_WeapNext_f(ent);
	else if (Q_stricmp(cmd, "weaplast") == 0)
		Cmd_WeapLast_f(ent);
	else if (Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if (Q_stricmp(cmd, "putaway") == 0)
		Cmd_PutAway_f(ent);
	else if (Q_stricmp(cmd, "wave") == 0)
		Cmd_Wave_f(ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else if (Q_stricmp(cmd, "pokemon") == 0)
		Cmd_SelectPokemon_f(ent);
	else if (Q_stricmp(cmd, "modhelp") == 0)
		Cmd_ModHelp_f(ent);
	else if (Q_stricmp(cmd, "retreat") == 0)
		Cmd_RetreatPokemon_f(ent);
	else if (Q_stricmp(cmd, "enemy") == 0)
		Cmd_SpawnEnemy_f(ent);
	else if (Q_stricmp(cmd, "random_attack") == 0)
		Cmd_RandomPokemonAttack_f(ent);
	else if (Q_stricmp(cmd, "switch_attack") == 0)
		Cmd_SwitchPokemonAttack_f(ent);
	else if (Q_stricmp(cmd, "pokemon_item") == 0)
		Cmd_UsePokemonItem_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
