Hearthplay
==========

Monte Carlo Tree Search for Hearthstone.

## Results

5000 trials of 16 games, a new random deck every 10 trials.

| Matchup | Player One Wins | Player Two Wins | Draws |
| ------------- | ------------- | ------------- | ------------- |
| Random vs Random | 1807 | 2011 | 1182 |
| Random vs CheatingMCTS | 5 | 4992 | 3 |
| Random vs DetMCTS | 13 | 4987 | 0 |
| Random vs SO-IS-MCTS | 10 | 4990 | 0 |
| CheatingMCTS vs Random | 4983 | 16 | 1 |
| CheatingMCTS vs CheatingMCTS | 3006 | 1994 | 0 |
| CheatingMCTS vs DetMCTS | 4082 | 918 | 0 |
| CheatingMCTS vs SO-IS-MCTS | 3412 | 1588 | 0 |
| DetMCTS vs Random | 4983 | 15 | 2 |
| DetMCTS vs CheatingMCTS | 1673 | 3327 | 0 |
| DetMCTS vs DetMCTS | 2985 | 2015 | 0 |
| DetMCTS vs SO-IS-MCTS | 1996 | 3004 | 0 |
| SO-IS-MCTS vs Random | 4991 | 8 | 1 |
| SO-IS-MCTS vs CheatingMCTS | 2648 | 2352 | 0 |
| SO-IS-MCTS vs DetMCTS | 3785 | 1215 | 0 |
| SO-IS-MCTS vs SO-IS-MCTS | 3042 | 1958 | 0 |

## Implemented Cards

* The Coin
* Wisp
* Argent Squire
* Goldshire Footman
* Murloc Raider
* Shieldbearer
* Stonetusk Boar
* Ancient Watcher
* Bloodfen Raptor
* Bluegill Warrior
* Frostwolf Grunt
* River Crocolisk
* Ironfur Grizzly
* Magma Rager
* Scarlet Crusader
* Silverback Patriarch
* Wolfrider
* Chillwind Yeti
* Mogu'shan Warden
* Oasis Snapjaw
* Sen'jin Shieldmasta
* Silvermoon Guardian
* Stormwind Knight
* Booty Bay Bodyguard
* Fen Creeper
* Argent Commander
* Boulderfist Ogre
* Lord of the Arena
* Reckless Rocketeer
* Sunwalker
* Core Hound
* War Golem
