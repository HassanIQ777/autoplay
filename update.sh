#!/usr/bin/env bash

# WARNING:
# This hard resets any project in which you run this
# Use this to update any program and its submodules to their latest versions
# (this is NOT the same as submodules' own latest. bump pins separately when needed)

echo " ----- Started Updater ----- "
echo "Updating the program..."
if git fetch origin && git reset --hard origin/main && git submodule update --init --recursive; then
    echo "Successfully finished updating!"
else
    echo "Failed to update."
fi