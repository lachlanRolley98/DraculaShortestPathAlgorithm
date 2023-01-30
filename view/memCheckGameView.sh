#!/bin/sh
make
RED='\033[0;31m'
RESET='\033[0m'
echo -e "${RED}This script will run valgrind to find any memory leaks, errors, and unusual behavours \nThis will print a large output, press any key to contiue ${RESET}"
read -p ""
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose -s ./testGameView