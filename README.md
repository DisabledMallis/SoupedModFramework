# SoupedModFramework
An open source modding framework for Bloons TD Battles 2


# Structure
## Common
Shared code between all SMF targets.

## Launcher
This is the launcher for B2SMF. It will load the game with SMF injected.

## Loader
This is the SMF mod loader. Once inside of BTDB2, it will hook functions necessary for applying mods.

## ModFS
An api for interacting with `.smf` files. These files are the mods a user wants to load. It is a JSON based archive format, meaning all data is JSON serialized.

## SMFApi
SMFApi is a JavaScript library for interacting and modifying the game's assets at runtime. It is helpful for modders since it provides a way to easily modify content, as well as distribute it without breaking copyright law.

## Proxy
The proxy dll responsible for bootstrapping required loaders.

## SMF-Pages
Web pages related to SMF. This includes pages for the launcher ui.

## SoupSDK
Code for interacting with the game. Provides classes, signatures, and other useful information.