# Snake_APO

## Snake

## Description
Snake game for MZAPO device. Game is intended for two players (local multiplayer) where each player controls one snake by rotating knob on MZAPO device. Game has two modes - Skirmish where players goal kill other players snake by making it crash and Hunt where the main goal of this mode is to collect certain number of apples faster then your opponent.


## Installation
The first thing you need to do to start the game on the MZ_APO machine is to turn on the machine and connect the MZ_APO to the network where your computer is located. Then, you need to access the MZ_APO system via gtkterm (login: root, password: mzAPO35) and add your SSH key. Once the IP address appears on the MZ_APO screen in binary format, record it in the makefile. Now, in the project console, you can type 'make all' to compile the project and 'make run' to launch it on the MZ_APO machine.

## Authors and License
Designed by Lezhnev Evgenii and Jáchym Žák, MIT license, 2024
