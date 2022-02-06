
# Splendor AI

In this project I try to create neural network 
playing Splendor capable of defeating me in a game.

### How to train

Change variables and hyperparameters in GeneticAlgorithm.cpp
If you want to change amount of tokens on board (from 7, 5 default) do this in SplendorGame.h in constructor at top
If you don't want to play game after change name of 'train' function to 'main' compile and run.
Otherwise change the 'training' variable in interface.cpp and compile and run interface.cpp

### How to run and interact with interface

If you want to train network for yourself change 'training' variable in interface.cpp check 'how to train' section to train correctly

To change number of players and add more / remove bots from game change variables 'player_count', 'ai_player_count'. 
After this fill 'sitting' vector with the order of players you want (0 for bot, 1 for human).

Compile interface.cpp and run it

Commands:
 - 'b' [where from] [place] Buy a card
     - [where from] 'b' [tier(1-3)] [position(1-4)] Buy card in position 'position' from tier 'tier'
     - [where_from] 'r' [position(1-3)] Buy card previously reserved in position 'position'
     
 - 'r' [tier(1-3)] [position(1-5)] Reserve a card from a board (5 for the hidden card on top of the deck)
 
 - 't' [tokens] Take 'tokens' from the board
     - [tokens] are in format xxxxx where x is any digit (will accept only legal takes) eg. 02000, 11010

eg. 
b b 2 3 (buys 3rd card from 2nd tier from board).
t 01101 (takes 0 green, 1 white, 1 blue, 0 black, 1 red).
r 1 4 (reserves 4th card from 1st tier).
