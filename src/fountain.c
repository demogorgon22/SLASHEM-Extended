/*	SCCS Id: @(#)fountain.c	3.4	2003/03/23	*/
/*	Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

#include "hack.h"

STATIC_DCL void dowatersnakes(void);
STATIC_DCL void dowaterdemon(void);
STATIC_DCL void dowaternymph(void);
STATIC_PTR void gush(int,int,void *);
STATIC_DCL void dofindgem(void);

void
floating_above(what)
const char *what;
{
    You("are floating high above the %s.", what);
}

STATIC_OVL void
dowatersnakes() /* Fountain of snakes! */
{
    register int num = rn1(5,2);
    struct monst *mtmp;

    if (!(mvitals[PM_RIVER_MOCCASIN].mvflags & G_GONE)) { /* changed to a non-concealing species --Amy */
	if (!Blind)
	    pline("An endless stream of %s pours forth!",
		  Hallucination ? makeplural(rndmonnam()) : "snakes");
	else
	    You_hear("%s hissing!", something);
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_RIVER_MOCCASIN],
			u.ux, u.uy, NO_MM_FLAGS)) && t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
    } else
	pline_The("fountain bubbles furiously for a moment, then calms.");
}

STATIC_OVL
void
dowaterdemon() /* Water demon */
{
    register struct monst *mtmp;
	register int wishchance = issoviet ? (60 + 5*level_difficulty()) : 97;
	/* In Soviet Russia, deep fountains may not give wishes. After all, digging too deep into the ground in such a cold country would endanger the groundwater supplies and is therefore strictly forbidden. --Amy */

    if(!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
	if((mtmp = makemon(&mons[PM_WATER_DEMON],u.ux,u.uy, NO_MM_FLAGS))) {
	    if (!Blind)
		You("unleash %s!", a_monnam(mtmp));
	    else
		You_feel("the presence of evil.");
/* ------------===========STEPHEN WHITE'S NEW CODE============------------ */
	/* Give those on low levels a (slightly) better chance of survival */
	/* 35% at level 1, 30% at level 2, 25% at level 3, etc... */            
	if (rnd(100) > wishchance) {
		pline("Grateful for %s release, %s grants you a boon!",
		      mhis(mtmp), mhe(mtmp));
		if (!rn2(4)) makewish();
		else othergreateffect();
		mongone(mtmp);
	    } else if (t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
	} else if (issoviet) {
		pline("Kha-kha-kha, tip bloka l'da ne khochet, chtoby vy, chtoby imet' krasivyy slavyanskiy zhenshchinu. Vmesto etogo, vy poluchayete vrazhdebnogo demona.");
	}
    } else
	pline_The("fountain bubbles furiously for a moment, then calms.");
}

STATIC_OVL void
dowaternymph() /* Water Nymph */
{
	register struct monst *mtmp;

	if(!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
	   (mtmp = makemon(&mons[PM_WATER_NYMPH],u.ux,u.uy, NO_MM_FLAGS))) {
		if (!Blind)
		   You("attract %s!", a_monnam(mtmp));
		else
		   You_hear("a seductive voice.");
		mtmp->msleeping = 0;
		if (t_at(mtmp->mx, mtmp->my))
		    (void) mintrap(mtmp);
	} else
		if (!Blind)
		   pline("A large bubble rises to the surface and pops.");
		else
		   You_hear("a loud pop.");
}

void
dogushforth(drinking) /* Gushing forth along LOS from (u.ux, u.uy) */
int drinking;
{
	int madepool = 0;

	do_clear_area(u.ux, u.uy, 7, gush, (void *)&madepool);
	if (!madepool) {
	    if (drinking)
		Your("thirst is quenched.");
	    else
		pline("Water sprays all over you.");
	}
}

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
void * poolcnt;
{
	register struct monst *mtmp;
	register struct trap *ttmp;

	if (((x+y)%2) || (x == u.ux && y == u.uy) ||
	    (rn2(1 + distmin(u.ux, u.uy, x, y)))  ||
	    (levl[x][y].typ != ROOM) ||
	    (sobj_at(BOULDER, x, y)) /*|| nexttodoor(x, y)*/)
		return;

	if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
		return;

	if (!((*(int *)poolcnt)++))
	    pline("Water gushes forth from the overflowing fountain!");

	/* Put a pool at x, y */
	levl[x][y].typ = POOL;
	/* No kelp! */
	del_engr_at(x, y);
	water_damage(level.objects[x][y], FALSE, TRUE);

	if ((mtmp = m_at(x, y)) != 0)
		(void) minliquid(mtmp);
	else
		newsym(x,y);
}

STATIC_OVL void
dofindgem() /* Find a gem in the sparkling waters. */
{
	if (!Blind) You("spot a gem in the sparkling waters!");
	else You_feel("a gem here!");
	(void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE-1),
			 u.ux, u.uy, TRUE, FALSE);
	SET_FOUNTAIN_LOOTED(u.ux,u.uy);
	newsym(u.ux, u.uy);
	exercise(A_WIS, TRUE);			/* a discovery! */
}

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
	if (IS_FOUNTAIN(levl[x][y].typ) &&
	    (!rn2(6) || FOUNTAIN_IS_WARNED(x,y))) {
		if(isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x,y)) {
			struct monst *mtmp;
			SET_FOUNTAIN_WARNED(x,y);
			/* Warn about future fountain use. */
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			    if (DEADMONSTER(mtmp)) continue;
			    if ((mtmp->data == &mons[PM_WATCHMAN] ||  mtmp->data == &mons[PM_WATCH_LIEUTENANT] ||
				mtmp->data == &mons[PM_WATCH_CAPTAIN] || mtmp->data == &mons[PM_WATCH_LEADER]) &&
			       couldsee(mtmp->mx, mtmp->my) &&
			       mtmp->mpeaceful) {
				pline("%s yells:", Amonnam(mtmp));
				verbalize("Hey, stop using that fountain!");
				break;
			    }
			}
			/* You can see or hear this effect */
			if(!mtmp) pline_The("flow reduces to a trickle.");
			return;
		}
#ifdef WIZARD
		if (isyou && wizard) {
			if (yn("Dry up fountain?") == 'n')
				return;
		}
#endif
		/* replace the fountain with ordinary floor */
		levl[x][y].typ = ROOM;
		levl[x][y].looted = 0;
		levl[x][y].blessedftn = 0;
		if (cansee(x,y)) pline_The("fountain dries up!");
		/* The location is seen if the hero/monster is invisible */
		/* or felt if the hero is blind.			 */
		newsym(x, y);
		level.flags.nfountains--;
		if(isyou && in_town(x, y))
		    (void) angry_guards(FALSE);
	}
}

void
drinkfountain()
{
	int nastytrapdur;
	int blackngdur;
	int pm;

	/* What happens when you drink from a fountain? */
	register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
	register int fate = rnd(30);

	if (Levitation) {
		floating_above("fountain");
		return;
	}

	if (uarmc && OBJ_DESCR(objects[uarmc->otyp]) && ( !strcmp(OBJ_DESCR(objects[uarmc->otyp]), "foundry cloak") || !strcmp(OBJ_DESCR(objects[uarmc->otyp]), "liteynyy plashch") || !strcmp(OBJ_DESCR(objects[uarmc->otyp]), "quyish plash") )) {
		u.uhunger += 100;
		pline("The water is very nutritious!");
	}

	if (mgkftn && u.uluck >= 0 && (!isfriday || !rn2(2)) && fate >= 10) {
		int i, ii, littleluck = (u.uluck < 4);

		pline("Wow!  This makes you feel great!");
		/* blessed restore ability */
		for (ii = 0; ii < A_MAX; ii++)
		    if (ABASE(ii) < AMAX(ii)) {
			ABASE(ii) = AMAX(ii);
			flags.botl = 1;
		    }
		/* gain ability, blessed if "natural" luck is high */
		i = rn2(A_MAX);		/* start at a random attribute */
		for (ii = 0; ii < A_MAX; ii++) {
		    if (adjattrib(i, 1, littleluck ? -1 : 0) && (littleluck || !rn2(2)) )
			break;
		    if (++i >= A_MAX) i = 0;
		}
		display_nhwindow(WIN_MESSAGE, FALSE);
		pline("A wisp of vapor escapes the fountain...");
		if (PlayerHearsSoundEffects) pline(issoviet ? "Vezet vam. No poprobovat' yeshche raz, i vy mgnovenno umirayut ot utopleniya." : "Dae-die-dae-dae-dae...");
		exercise(A_WIS, TRUE);
		levl[u.ux][u.uy].blessedftn = 0;
		return;
	}

	if (fate < 10) {
		pline_The("cool draught refreshes you.");
		u.uhunger += rnd(40); /* don't choke on water */
		newuhs(FALSE);
		if(mgkftn) return;
	} else {
	    switch (fate) {

		case 12:
			if (!Unchanging) { /* Serves you right for quaffing from fountains. --Amy */
				You_feel("a change coming over you.");
				polyself(FALSE);
			}
			break;

		case 13:
			pline("The water is bad!");
			badeffect();
			break;

		case 14:
			pline("Something comes out of the fountain!");

			int aggroamount = rnd(6);
			if (isfriday) aggroamount *= 2;
			u.aggravation = 1;
			reset_rndmonst(NON_PM);
			while (aggroamount) {

				makemon((struct permonst *)0, u.ux, u.uy, MM_ANGRY);
				aggroamount--;
				if (aggroamount < 0) aggroamount = 0;
			}
			u.aggravation = 0;

			break;

		case 15:
			pline("Ulch - the water was radioactive!");
			contaminate(rnd(10 + level_difficulty()), TRUE);
			break;

		case 16:
			pm = rn2(5) ? dprince(rn2((int)A_LAWFUL+2) - 1) : dlord(rn2((int)A_LAWFUL+2) - 1);
			if (pm && (pm != NON_PM)) {
				(void) makemon(&mons[pm], u.ux, u.uy, MM_ANGRY);
				pline("An angry demon climbs out of the fountain...");
			}
			break;

		case 17: /* evil patch idea by Amy - give that fountain quaffer fool a long-lasting nasty trap effect */
			pline("This tepid water is tasteless.");
			if (!rn2(isfriday ? 10 : 20)) {

				nastytrapdur = (Role_if(PM_GRADUATE) ? 36 : Role_if(PM_GEEK) ? 72 : 144);
				if (!nastytrapdur) nastytrapdur = 144; /* fail safe */
				blackngdur = (Role_if(PM_GRADUATE) ? 2000 : Role_if(PM_GEEK) ? 1000 : 500);
				if (!blackngdur ) blackngdur = 500; /* fail safe */

				if (!rn2(100)) pline("You have a bad feeling in your %s.",body_part(STOMACH) );

				randomnastytrapeffect(rnz(nastytrapdur * (monster_difficulty() + 1)), blackngdur - (monster_difficulty() * 3));

			}
			break;

		case 18: /* Experience (idea by Amy) */
			pluslvl(FALSE);
			break;

		case 19: /* Self-knowledge */

			You_feel("self-knowledgeable...");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0, 1);
			exercise(A_WIS, TRUE);
			pline_The("feeling subsides.");
			break;

		case 20: /* Foul water */

			pline_The("water is foul!  You gag and vomit.");
			morehungry(rn1(20, 11));
			vomit();
			break;

		case 21: /* Poisonous */

			pline_The("water is contaminated!");
			if (Poison_resistance) {
			   pline(
			      "Perhaps it is runoff from the nearby %s farm.",
				 fruitname(FALSE));
			   losehp(rnd(4),"unrefrigerated sip of juice",
				KILLED_BY_AN);
			   break;
			}
			losestr(rn1(4,3));
			losehp(rnd(10),"contaminated water", KILLED_BY);
			exercise(A_CON, FALSE);
			break;

		case 22: /* Fountain of snakes! */

			dowatersnakes();
			break;

		case 23: /* Water demon */
			if (uarmc && uarmc->oartifact == ART_JANA_S_ROULETTE_OF_LIFE && !rn2(10)) {
				pline("Booyah, luck is smiling on you!");
				if (!rn2(4)) makewish();
				else othergreateffect();
			} else dowaterdemon();
			break;

		case 24: /* Curse an item */ {
			register struct obj *obj;

			pline("This water's no good!");
			morehungry(rn1(20, 11));
			exercise(A_CON, FALSE);
			for(obj = invent; obj ; obj = obj->nobj)
				if (!rn2(5) && !stack_too_big(obj))	curse(obj);
			break;
			}

		case 25: /* See invisible */

			if (Blind) {
			    if (Invisible) {
				You_feel("transparent.");
			    } else {
			    	You_feel("very self-conscious.");
			    	pline("Then it passes.");
			    }
			} else {
			   You("see an image of someone stalking you.");
			   pline("But it disappears.");
			}
			HSee_invisible |= FROMOUTSIDE;
			newsym(u.ux,u.uy);
			exercise(A_WIS, TRUE);
			break;

		case 26: /* See Monsters */

			(void) monster_detect((struct obj *)0, 0);
			exercise(A_WIS, TRUE);
			break;

		case 27: /* Find a gem in the sparkling waters. */

			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}

		case 28: /* Water Nymph */

			dowaternymph();
			break;

		case 29: /* Scare */ {
			register struct monst *mtmp;

			pline("This water gives you bad breath!");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    if(!DEADMONSTER(mtmp))
				monflee(mtmp, rnd(10), FALSE, FALSE);
			}
			break;

		case 30: /* Gushing forth in this room */

			dogushforth(TRUE);

			/* evil patch idea by jonadab:
			   fountains have a small percentage chance of killing you outright, flavored as drowning */

			if (!Amphibious && !Swimming && !Breathless && !rn2(isfriday ? 10 : 20) && !(uarmf && OBJ_DESCR(objects[uarmf->otyp]) && ( !strcmp(OBJ_DESCR(objects[uarmf->otyp]), "fin boots") || !strcmp(OBJ_DESCR(objects[uarmf->otyp]), "plavnik sapogi") || !strcmp(OBJ_DESCR(objects[uarmf->otyp]), "kanatcik chizilmasin") ) ) ) {

				u.youaredead = 1;

				pline("You drown...");
				if (PlayerHearsSoundEffects) pline(issoviet ? "Pochemu ty slishkom glup, chtoby zhit' tak ili inache? Do svidaniya!" : "HUAAAAAAA-A-AAAAHHHHHH!");
				    killer_format = KILLED_BY_AN;
				    killer = "overflowing fountain";
				    done(DROWNING);

				u.youaredead = 0;


			}

			/* evil patch idea by jonadab: 
			   Drinking from a fountain can cause the fountain to overflow, turning the tile into a pool. */
			if (!rn2(isfriday ? 5 : 10)) {
				levl[u.ux][u.uy].typ = POOL;
				if (!Wwalking && !Flying && !Levitation) drown();
			}

			break;

		default:

			pline("This tepid water is tasteless.");

			u.uhunger += rnd(5); /* don't choke on water */
			newuhs(FALSE);

			break;
	    }
	}
	if (level.flags.lethe) { /* bad idea! */
		You_feel("the lethe waters running down your throat...");
		You_feel("dizzy!");
		forget(1 + rn2(5));
	}

	if (uarmc && uarmc->oartifact == ART_JANA_S_ROULETTE_OF_LIFE && !rn2(100)) {
		u.youaredead = 1;
		pline("Bad luck! You die.");
		killer_format = KILLED_BY;
		killer = "Jana's roulette";
		done(DIED);
		u.youaredead = 0;
	}

	dryup(u.ux, u.uy, TRUE);
}

void
dipfountain(obj)
register struct obj *obj;
{
	int pm;

	if (Levitation) {
		floating_above("fountain");
		return;
	}

	/* Don't grant Excalibur when there's more than one object.  */
	/* (quantity could be > 1 if merged daggers got polymorphed) */

	if (rn2(2) && !obj->oerodeproof && is_rustprone(obj) && !hard_to_destruct(obj) && (!obj->oartifact || !rn2(4)) && obj->oeroded == MAX_ERODE) {
		remove_worn_item(obj, FALSE);
		if (obj == uball) unpunish();
		useupall(obj);
		update_inventory();
		pline("The item rusted away completely!");
		return;
	}

	if (obj->otyp == LONG_SWORD && obj->quan == 1L
		/* it's supposed to be rare to get the thing if you're not a knight --Amy */
	    && u.ulevel > 4 && (!isfriday || !rn2(3)) && !rn2(Role_if(PM_KNIGHT) ? 8 : 50) && !obj->oartifact
	    && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {

		if (u.ualign.type != A_LAWFUL) {
			/* Ha!  Trying to cheat her. */
			pline("A freezing mist rises from the water and envelopes the sword.");
			pline_The("fountain disappears!");
			curse(obj);
			if (obj->spe > -6 && !rn2(3)) obj->spe--;
			obj->oerodeproof = FALSE;
			exercise(A_WIS, FALSE);
		} else {
			/* The lady of the lake acts! - Eric Backus */
			/* Be *REAL* nice */
	  pline("From the murky depths, a hand reaches up to bless the sword.");
			pline("As the hand retreats, the fountain disappears!");
			obj = oname(obj, artiname(ART_EXCALIBUR));
			discover_artifact(ART_EXCALIBUR);
			bless(obj);
			obj->oeroded = obj->oeroded2 = 0;
			obj->oerodeproof = TRUE;
			exercise(A_WIS, TRUE);
#ifdef LIVELOGFILE
			livelog_report_trophy("had Excalibur thrown to them by some watery tart");
#endif
		}
		update_inventory();
		levl[u.ux][u.uy].typ = ROOM;
		levl[u.ux][u.uy].looted = 0;
		newsym(u.ux, u.uy);
		level.flags.nfountains--;
		if(in_town(u.ux, u.uy))
		    (void) angry_guards(FALSE);
		return;
	} else if (get_wet(obj, FALSE) && !rn2(2))
		return;

	if (!obj) return; /* if the get_wet destroyed it --Amy */
	if (level.flags.lethe) { /* bad idea */
		pline("The sparkling waters wash over your %s...", doname(obj));
		lethe_damage(obj, TRUE, FALSE);
	}
	if (!obj) return; /* if the lethe_damage destroyed it --Amy */

	/* Acid and water don't mix */
	if (obj->otyp == POT_ACID) {
	    useup(obj);
	    return;
	}

	switch (rnd(30)) {

		case 8:
			pline("Something comes out of the fountain!");

			int aggroamount = rnd(6);
			if (isfriday) aggroamount *= 2;
			u.aggravation = 1;
			reset_rndmonst(NON_PM);
			while (aggroamount) {

				makemon((struct permonst *)0, u.ux, u.uy, MM_ANGRY);
				aggroamount--;
				if (aggroamount < 0) aggroamount = 0;
			}
			u.aggravation = 0;
			break;

		case 9:
		/* evil patch idea by rikersan: he wants demon lords to occasionally appear if you dip fountains.
		 * I made it so that if you're foolish enough to QUAFF from them instead, it happens much more often --Amy */
			if (!rn2(50)) {
				pm = rn2(5) ? dprince(rn2((int)A_LAWFUL+2) - 1) : dlord(rn2((int)A_LAWFUL+2) - 1);
				if (pm && (pm != NON_PM)) {
					(void) makemon(&mons[pm], u.ux, u.uy, MM_ANGRY);
					pline("An angry demon climbs out of the fountain...");
				}
			}
			break;

		case 10: /* Curse the item */
			if (!stack_too_big(obj)) curse(obj);
			break;
		case 11:
		case 12:
		case 13:
		case 14: /* Uncurse the item */
			if(obj->cursed) {
			    if (!Blind)
				pline_The("water glows for a moment.");
			    uncurse(obj, FALSE);
			} else {
			    pline("A feeling of loss comes over you.");
			}
			break;
		case 15:
		case 16: /* Water Demon */
			dowaterdemon();
			break;
		case 17:
		case 18: /* Water Nymph */
			dowaternymph();
			break;
		case 19:
		case 20: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 21:
		case 22:
		case 23: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 24:
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
			pline("A strange tingling runs up your %s.",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
			You_feel("a sudden chill.");
			break;
		case 28: /* Strange feeling */
			pline("An urge to take a bath overwhelms you.");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
			    You("lost some of your gold in the fountain!");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
			        You("lost some of your money in the fountain!");
				CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (FOUNTAIN_IS_LOOTED(u.ux,u.uy)) break;
		    SET_FOUNTAIN_LOOTED(u.ux,u.uy);
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
		pline("Far below you, you see coins glistening in the water.");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

void
diptoilet(obj)
register struct obj *obj;
{
	if (Levitation) {
	    floating_above("toilet");
	    return;
	}

	if (rn2(2) && !obj->oerodeproof && is_rustprone(obj) && !hard_to_destruct(obj) && (!obj->oartifact || !rn2(4)) && obj->oeroded == MAX_ERODE) {
		remove_worn_item(obj, FALSE);
		if (obj == uball) unpunish();
		useupall(obj);
		update_inventory();
		pline("The item rusted away completely!");
		return;
	}

	(void) get_wet(obj, FALSE);
	/* KMH -- acid and water don't mix */
	if (obj->otyp == POT_ACID && !stack_too_big(obj)) {
	    useup(obj);
	    return;
	}
	if(is_poisonable(obj) && !stack_too_big(obj)) {
	    if (flags.verbose)  You("cover it in filth.");
	    obj->opoisoned = TRUE;
	}
	if (obj->oclass == FOOD_CLASS && !stack_too_big(obj)) {
	    if (flags.verbose)  pline("My! It certainly looks tastier now...");
	    obj->orotten = TRUE;
	}
	if (flags.verbose)  pline("Yuck!");

	if (!rn2(25)) {
		pline("The toilet breaks!");
		level.flags.nsinks--;
		levl[u.ux][u.uy].typ = ROOM;
		newsym(u.ux,u.uy);
	}
}


void
breaksink(x,y)
int x, y;
{
    if(cansee(x,y) || (x == u.ux && y == u.uy))
	pline_The("pipes break!  Water spurts out!");
    level.flags.nsinks--;
    levl[x][y].doormask = 0;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
}

void
breaktoilet(x,y)
int x, y;
{
    register int num = rn1(5,2);
    struct monst *mtmp;
    pline("The toilet suddenly shatters!");
    level.flags.nsinks--;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
    if (!rn2(3)) {
      if (!(mvitals[PM_BABY_CROCODILE].mvflags & G_GONE)) {
	if (!Blind) {
	    if (!Hallucination) pline("Oh no! Crocodiles come out from the pipes!");
	    else pline("Oh no! Tons of poopies!");
	} else
	    You("hear something scuttling around you!");
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_BABY_CROCODILE],u.ux,u.uy, NO_MM_FLAGS)) &&
	       t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
      } else
	pline("The sewers seem strangely quiet.");
    }
}

void
drinksink()
{
	struct obj *otmp;
	struct monst *mtmp;

	if (Levitation) {
		floating_above("sink");
		return;
	}
	switch(rn2(25)) {
		case 0:
			You("take a sip of very cold water.");
			u.uhunger += rnd(20); /* don't choke on water */
			newuhs(FALSE);
			break;
		case 1:
			You("take a sip of very warm water.");
			u.uhunger += rnd(50); /* don't choke on water */
			newuhs(FALSE);
			break;
		case 2: You("take a sip of scalding hot water.");
			if (Fire_resistance) {
				pline("It seems quite tasty.");
				u.uhunger += rnd(50); /* don't choke on water */
				newuhs(FALSE);
			} else losehp(rnd(6), "sipping boiling water", KILLED_BY);

			u.uhunger += rnd(10); /* don't choke on water */
			newuhs(FALSE);

			break;
		case 3: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
				pline_The("sink seems quite dirty.");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
				if (mtmp) pline("Eek!  There's %s in the sink!",
					(Blind || !canspotmon(mtmp)) ?
					"something squirmy" :
					a_monnam(mtmp));
			}
			break;
		case 4: do {
				otmp = mkobj(POTION_CLASS,FALSE);
				if (otmp && otmp->otyp == POT_WATER) {
					obfree(otmp, (struct obj *)0);
					otmp = (struct obj *) 0;
				}
				if (otmp && otmp->otyp == GOLD_PIECE) {
					obfree(otmp, (struct obj *)0);
					otmp = (struct obj *) 0;
				}
			} while(!otmp);
			otmp->cursed = otmp->blessed = 0;
			pline("Some %s liquid flows from the faucet.",
			      Blind ? "odd" :
			      hcolor(OBJ_DESCR(objects[otmp->otyp])));
			otmp->dknown = !(Blind || Hallucination);
			otmp->fromsink = 1; /* kludge for docall() */
			/* dopotion() deallocs dummy potions */
			(void) dopotion(otmp);
			break;
		case 5: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
			    You("find a ring in the sink!");
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
			} else pline("Some dirty water backs up in the drain.");
			break;
		case 6: breaksink(u.ux,u.uy);
			break;
		case 7: pline_The("water moves as though of its own will!");
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
				pline("But it quiets down.");
			break;
		case 8: pline("Yuk, this water tastes awful.");
			more_experienced(1,0);
			newexplevel();
			break;
		case 9: pline("Gaggg... this tastes like sewage!  You vomit.");
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
			break;
		case 10:
			/* KMH, balance patch -- new intrinsic */
			pline("This water contains toxic wastes!");
			if (!Unchanging) {
				You("undergo a freakish metamorphosis!");
				polyself(FALSE);
			}
			break;
		/* more odd messages --JJB */
		case 11: You_hear("clanking from the pipes...");
			break;
		case 12: You_hear("snatches of song from among the sewers...");
			break;

		/* evil patch idea by jonadab - playing around with sinks can generate a green slime */
		case 13: if (mvitals[PM_GREEN_SLIME].mvflags & G_GONE)
				pline_The("sink seems extremely yucky...");
			else {
				mtmp = makemon(&mons[PM_GREEN_SLIME],
						u.ux, u.uy, NO_MM_FLAGS);
				if (mtmp) pline("Yuck!  There's %s in the sink!",
					(Blind || !canspotmon(mtmp)) ?
					"something slimy" :
					a_monnam(mtmp));
			}
			break;

            case 14: pline("Steam pours out of the faucet.");
                  if ((mvitals[PM_STEAM_VORTEX].mvflags & G_GONE)
                  || makemon(&mons[PM_STEAM_VORTEX],
                  u.ux, u.uy, NO_MM_FLAGS) || rnl(10) > 5)
                      pline("It shapes itself into a spiral!");
                  break;
            case 15: pline("A strong jet of %s water splashes all over you!",
                  rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot");
			water_damage(invent, FALSE, FALSE);
			if (level.flags.lethe) lethe_damage(invent, FALSE, FALSE);
                  break;
            case 16: pline("A strong jet of scalding hot water splashes all over you!");
                  if (Fire_resistance)
                       pline("It feels quite refreshing.");
                  else if (!Fire_resistance)
                       losehp(d(4,6), "jet of boiling water", KILLED_BY_AN);
			water_damage(invent, FALSE, FALSE);
			if (level.flags.lethe) lethe_damage(invent, FALSE, FALSE);
                  break;
            case 17: if (Hallucination)
                  pline_The("water flies out of the plughole and into the faucet!");
		case 19: if (Hallucination) {
		   pline("From the murky drain, a hand reaches up... --oops--");
				break;
			}
		default: You("take a sip of %s water.",
			rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot");
			u.uhunger += rnd(40); /* don't choke on water */
			newuhs(FALSE);
	}
}

void
drinktoilet()
{
	if (Levitation) {
		floating_above("toilet");
		return;
	}
	if ((youmonst.data->mlet == S_DOG) && (rn2(5)) ) {
		pline("The toilet water is quite refreshing!");
		u.uhunger += 10;
		return;
	}
	switch(rn2(9)) {
/*
		static NEARDATA struct obj *otmp;
 */
		case 0: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
				pline("The toilet seems quite dirty.");
			else {
				static NEARDATA struct monst *mtmp;

				mtmp = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy,
					NO_MM_FLAGS);
				pline("Eek!  There's %s in the toilet!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
			}
			break;
		case 1: breaktoilet(u.ux,u.uy);
			break;
		case 2: pline("Something begins to crawl out of the toilet!");
			if (mvitals[PM_BROWN_PUDDING].mvflags & G_GONE
			    || !makemon(&mons[PM_BROWN_PUDDING], u.ux, u.uy,
					NO_MM_FLAGS))
				pline("But it slithers back out of sight.");
			break;
		case 3:
		case 4: if (mvitals[PM_BABY_CROCODILE].mvflags & G_GONE)
				pline("The toilet smells fishy.");
			else {
				static NEARDATA struct monst *mtmp;

				mtmp = makemon(&mons[PM_BABY_CROCODILE], u.ux,
					 u.uy, NO_MM_FLAGS);
				pline("Egad!  There's %s in the toilet!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
			}
			break;
		default: pline("Gaggg... this tastes like sewage!  You vomit.");
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
	}
}

void
whetstone_fountain_effects(obj)
register struct obj *obj;
{
	if (Levitation) {
		floating_above("fountain");
		return;
	}

	switch (rnd(30)) {
		case 10: /* Curse the item */
			if (!stack_too_big(obj)) curse(obj);
			break;
		case 11:
		case 12:
		case 13:
		case 14: /* Uncurse the item */
			if(obj->cursed && !stack_too_big(obj)) {
			    if (!Blind)
				pline_The("water glows for a moment.");
			    uncurse(obj, FALSE);
			} else {
			    pline("A feeling of loss comes over you.");
			}
			break;
		case 15:
		case 16: /* Water Demon */
			dowaterdemon();
			break;
		case 17:
		case 18: /* Water Nymph */
			dowaternymph();
			break;
		case 19:
		case 20: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 21:
		case 22:
		case 23: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 24:
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
			pline("A strange tingling runs up your %s.",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
			You_feel("a sudden chill.");
			break;
		case 28: /* Strange feeling */
			pline("An urge to take a bath overwhelms you.");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
			    You("lost some of your gold in the fountain!");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
			        You("lost some of your money in the fountain!");
			        levl[u.ux][u.uy].looted &= ~F_LOOTED;
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (levl[u.ux][u.uy].looted) break;
		    levl[u.ux][u.uy].looted |= F_LOOTED;
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
		pline("Far below you, you see coins glistening in the water.");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

void
whetstone_toilet_effects(obj)
register struct obj *obj;
{
	if (Levitation) {
	    floating_above("toilet");
	    return;
	}
	if(is_poisonable(obj) && !stack_too_big(obj)) {
	    if (flags.verbose)  You("cover it in filth.");
	    obj->opoisoned = TRUE;
	}
	if (flags.verbose)  pline("Yuck!");
}

void
whetstone_sink_effects(obj)
register struct obj *obj;
{
	struct monst *mtmp;

	if (Levitation) {
		floating_above("sink");
		return;
	}
	switch(rn2(20)) {
		case 0: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
				pline_The("sink seems quite dirty.");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
				pline("Eek!  There's %s in the sink!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
			}
			break;
		case 1: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
			    You("find a ring in the sink!");
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
			} else pline("Some dirty water backs up in the drain.");
			break;
		case 2: breaksink(u.ux,u.uy);
			break;
		case 3: pline_The("water moves as though of its own will!");
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
				pline("But it quiets down.");
			break;
		case 4:
			pline("This water contains toxic wastes!");
			obj = poly_obj(obj, STRANGE_OBJECT);
			u.uconduct.polypiles++;
			break;
		case 5: You_hear("clanking from the pipes...");
			break;
		case 6: You_hear("snatches of song from among the sewers...");
			break;
		case 19: if (Hallucination) {
		   pline("From the murky drain, a hand reaches up... --oops--");
				break;
			}
		default:
			break;
	}
}

/*fountain.c*/
