# MasterAI-1.0-1vs1-Limit

## Introduction

MasterAI is an AI poker dedicated to suport n-play (single- or multi-agent) Texas Hold'em imperfect-informatin games. It has been achieving exceptionally good results by using its propretary algorithm. In September 2020, it defeated 14 top human poker professionals. 

## Technology

MasterAI is developed by two components: offline component and online component:
* An offline component solves random poker situations (public states along with probability vectors over private hands for both players) and uses them to train a neural network. After training, this neural network can accurately predict the value for each player of holding each possible hand at a given poker situation.

* An online component uses the continuous re-solving algorithm to dynamically select an action for MasterAI to play at each public state during the game. This algorithm solves a depth-limited lookahead using the neural network to estimate values at terminal states.

The MasterAI strategy approximates a Nash Equilibrium, with an approximation error that depends on the neural network error and the solution error of the solver used in continuous re-solving.

## Prerequisites

Running any of the MasterAI code requires [torch](http://torch.ch/).
Torch is only officially supported based on *Linux systems. [This page](https://github.com/torch/torch7/wiki/Windows)
contains suggestions for using torch on a Windows machine

If you would like to personally play against MasterAI, We provide AI models to challeng,[click here](https://master.deeptexas.ai/aigame/).

## Contact us

The Master team is constantly exploring the innovation of AI algorithm, and hoping that like-minded technical experts from all over the world can communicate and exchange here, or join us to make MasterAI bigger and stronger together. Please feel free to contact us at masterai918@gmail.com 
