#!/usr/bin/env bash

echo -e "\033[1;36mJWCCommandSpawn ANSI Sniffer Demo\033[0m"
echo "This script prints ANSI effects, asks for input, then continues showing a multi character glyph."
echo

echo -e "Normal text, then \033[31mred\033[0m, \033[32mgreen\033[0m, and \033[1mbold\033[0m."
echo

printf "Enter your name: "
read name

echo -e "\033[33m😃Hello, $name!\033[0m"
echo "Now demonstrating delayed output:"

for i in 1 2 3 4 5; do
    echo -e "tick \033[35m$i\033[0m"
    sleep 1
done

echo
echo -e "\033[1;32mDone.\033[0m"