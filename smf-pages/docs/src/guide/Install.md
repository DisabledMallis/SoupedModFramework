# Installing SMF

SMF has a very simple installation process. Before beginning, please ensure that you have the **Steam** version of BTDB2, as well as you are using a **64-bit** version of Windows. Linux support is planned, but not priority right now.

## Downloading SMF

The first step in installing SMF is to actually download it. You can find downloads [here](https://github.com/DisabledMallis/SoupedModFramework/releases). Download the `SoupedModFramework.zip` file, NOT the source. If you wish to build from source, see [Building SMF](./build).

If there are no Releases, download an experimental version. (See below)

### Downloading experimental SMF versions

Alternatively, you can download experimental SMF builds from GitHub Actions Artifacts [here](https://github.com/DisabledMallis/SoupedModFramework/actions). Please note that these builds will likely have even more problems than releases. To download one, select the `Build SMF` workflow,

![](./img/workflows.png)

and then choose the newest (top most) working build (the one with a green check mark). Scroll to the bottom and download the `SoupedModFramework` artifact.

![](./img/artifacts.png)

## Preparing the game

Before we can install SMF, we need to find our game's files. Inside of Steam, navigate to your Steam Library. Right click on Bloons TD Battles 2 and select "properties".

![](./img/properties.png)

Next, select the "Local Files" tab and then "Browse".

![](./img/browse.png)

## Extracting

To extract the `SoupedModFramework.zip` file, use [7Zip](https://www.7-zip.org/). Open the file using 7zip, and drag the contents into the game's folder.

## Launching SMF

SMF comes with a `Launcher.exe` file. To launch SMF, open the launcher and select 'Launch'. The launcher will now load the game with SMF injected. NOTE: You will not be logged in to BTDB2 in game, DO NOT log in to your account (Using mods will flag your account), instead create a brand new BTDB2 account for using mods.
