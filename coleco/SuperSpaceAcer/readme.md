20091227

Super Space Acer
----------------

How to Play
-----------

1) Move joystick
2) shoot stuff
3) don't die

There are thee ships you can choose from on the main game select. Each ship
has a gameplay quirk to it. Unless otherwise mentioned, all ships move at
a set speed, support all powerups, have a shield that makes you invincible
while slowly counting down, and give you four lives. Getting hit without a
shield will destroy a life and set you back slightly from the boss.

- Cruiser: Slowest ship, but has a continuous power shield that can take
three hits before vanishing, and can be recharged. You get one life.

- Snowball: Standard ship upon which all others are based.

- Ladybird: Smaller ship by popular demand. Shield counts down faster but
may be used offensively.

There are also three difficulty modes. Higher difficulty modes offer more
enemies, less shields, and better endings. ;)

-------
Secrets
-------

The game has two secret ships and a third secret gameplay mode.

----------
Background
----------

This game I originally started in 1989 for the TI.. a couple years later I 
rewrote it in "c99", a small C dialect for the TI-99/4A. I released this 
commercially in 1991 and sold a single copy (my distributor claimed to sell 
a dozen more.)

Anyway, I found the source code on an old disk, and ported it across to the 
Coleco. All I have to do was write the support functions. I was hiccupped 
slightly by a buggy rand() function (my fault), and poorly documented code, 
but it came up pretty quickly, and in general I'm tickled to play the game 
again on the Coleco.

Many, many things have changed now. New graphics, included boss graphics
from the Tyrian project, better behaviours, better power ups, multiple
ships with multiple game modes, and more yet to come!

As such, please do not redistribute this ROM, please do not burn it to carts. 
It's not finished yet. But please feel free to share links and keep it for 
yourself, and I hope you like it!

-Mike Brent

Art Credits
-----------

- Title graphics originally by Brendan Wiese - http://starbow.com - converted by Tursi
- Boss graphics originally by Dan C, from the Tyrian project, released - http://lostgarden.com/2007/04/free-game-graphics-tyrian-ships-and.html - converted by Tursi
- Other graphics by Tursi

Music Credits
-------------

- music by Tadd Nuznov, used with explicit permission

---

27 Dec 2009 
- original release

28 Nov 2010
- Mines have blinking tips now
- boss explosions animate
- separate powerups for 3-way and pulse wave
- random number fixes
- code cleanup
- made bosses scroll smoothly horizontally
- made music player loop properly
- smoothed out framerate - improved performance
- improved 3-way to not 'lose' shots
- difficulty tweaks
- sound effect tweaks (boss explosion, ship warp)
- boss doesn't delete enemies anymore - they have to leave the screen
- boss approach warning onscreen
- score now visible onscreen

24 Jan 2011
- Powerups, I think? I didn't document this release.

30 Jun 2015
- Replace player, saucer, jet and helicopter sprites with new graphics
- add personality to saucer and helicopter enemies
- lots and lots of tuning
- source code ready for breaking up into banks - last 32k release

8 Aug 2015
- performance optimizations
- bugfix - invisible/popup powerups no longer occur
- bugfix - bosses no longer make powerups go away
- bugfix - dying on blimp doesn't corrupt the boss image
- bugfix - even a score of 0 will show on the title page after a game now
- bugfix - fixed sprite corruption on title page
- re-centered collision code
- improved sprite flicker
- added player ship select with three unique ships
- added three secret gameplay modes
- reduced number of explosion cells on the boss
- helicopters now turn around and fly off screen after shooting

28 Aug 2015
- bugfix - clean up sprites when leaving difficulty select
- bugfix - fix font characters
- bugfix - restore older bank switch mechanism
- bugfix - fix sprite for wide gun powerup
- bugfix - preserve score after win scroller
- bugfix - mines leave properly during BOSS APPROACHING
- bugfix - made boss hit flash last longer
- bugfix - don't draw lives indicator for cruiser or other single-life mode
- added easy/medium/hard mode endings (only one hard ending so far)
- scroll text (now 'medium' ending) is now a smooth scroll
- added proper lowercase to font
- re-did title page for new copyright year
- all enemies now have hitpoints, in particular mines, bombs and swirlies are harder to destroy
- tweaked weapon in one of the secret modes
- replaced boss draw code with explicit draw functions
- improved boss collision detection against player
- cruiser is now invulnerable during it's hit shimmy

19 Sept 2015
- fix ladybug shield behavior to recharge shield when hitting enemies
- fix random engine corruption during boss battle
- bosses with mine droppers launch increasingly difficult mines
- no longer recenters player before boss
- fix timing bug when boss is preparing to enter (should work on hw now)
- change startup to clear interrupt flags
- add scoring flag for cheat mode
- also remember scoremode across reboots
