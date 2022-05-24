# SoupedModFramework
An open source modding framework for Bloons TD Battles 2

Please leave a ⭐️ <3

# Getting Started
SMF is simple to install, follow the [installation guide](https://souped.dev/guide/Install.html) for instructions!

# Usage
For mod creation, check out the [development guide](https://souped.dev/develop/).

# Community
Looking for mods, or just to talk with other members in the SMF community?
Join the SMF [Discord Server](https://discord.gg/nPcBPyHP4c)!

http://bloonsmods.com

# Structure
## Common
Shared code between all SMF targets.

## Launcher
This is the launcher for SMF. It will load the game with SMF injected.

## Loader
This is the SMF mod loader. Once inside of BTDB2, it will hook functions necessary for applying mods.

## ModExplore
A small program to view the contents of mod archives. For modification & mod development, please see the Mod creation guide.

## ModFS
An api for interacting with `.smf` files. These files are the mods a user wants to load. It is a Zip based archive format with additional metadata in the form of a zip entry named 'meta.json' (Read the mod making guide for details)

## Proxy
The proxy dll responsible for bootstrapping required loaders.

## SMF-Pages
Web pages related to SMF. This includes pages for the launcher ui.

## SoupSDK
Code for interacting with the game. Provides classes, signatures, and other useful information.
