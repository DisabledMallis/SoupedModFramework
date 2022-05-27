---
sidebar: auto
---

# Developing with SMF

## About SMF and how it works

Part of SMF's goal is to expose an API to create mods for Bloons TD Battles 2 (which I will refer to as 'BTDB2'). Unlike Bloons TD6 (BTD6), BTDB2 is not made with Unity. Since Unity uses C#, modding tools like MelonLoader can be used on BTD6 to create mods. MelonLoader is powerful and gives you a library you can link your own C# code against to create mods. BTDB2, on the other hand, was not created with Unity, but instead a proprietary engine created by Ninja Kiwi. This engine is believed to be a heavily modified version of the BTD5/BTDB1 engines. This engine is written in C++ and not C#, so unfortunately we cannot decompile the game to make mods. (Even if we could decompile the game, it wouldn't produce anything meaningful to mod creators, since function & type names would have been lost in the process) This does not mean making mods is impossible! Looking back on BTD5's modding scene, there is something called a 'jet' file. These 'jet' files hold the contents for the game's assets. In BTDB2, we have 'jet' files inside of the game's files, but no way to edit them. Unlike BTD5, we cannot just extract the jet files with 7zip, (Technically we actually can, but there's no reason to as you'll find out) However, we _can_ modify their contents inside of the game's memory. This is done in the form of 'patchers', which you will learn all about in this guide.

## Creating your first mod

SMF mods are very simple, they are just uncompressed tar/zip files with a 'meta.json' file inside. While developing a mod, you will not be required to package it into the tar/zip archive to test it.

### Getting started (+ meta.json)

To get started, create a new folder inside of the 'mods' folder. (If there is no 'mods' folder, simply create one) You can name this folder anything you want. Inside of the folder, create a new 'meta.json' file. It is important that the meta.json file has the information SMF needs. Take a look at the meta.json example file below:

```json
{
    "authors": [
        "DisabledMallis",
        "B2C",
        "NeverGlow"
    ],
    "description": "This is an example mod that does very cool things!",
    "modid": "example",
    "name": "Example Mod",
    "scripts": [
        "exampleScript.js"
    ],
    "version": "1.0.0"
}
```

So, what do these fields mean?

-   `authors` - A list of who created the mod
-   `description` - A description of what the mod does
-   `modid` - The mod's ID, should not contain: Capital letters, Spaces, Numbers
-   `name` - The name of the mod
-   `scripts` - A list of every script file this mod needs to load
-   `version` - The version of the mod

It is important that the file includes a `name`, `modid`, `version` and `scripts` field. You shouldn't leave out the other fields, but it wont be very problematic if you do.

### Scripting part 1

Now that we have our meta.json created, lets create a script for our mod. Inside of the meta.json, we specified that the mod has a script called `exampleScript.js`. So, inside of the mod folder we create a new `exampleScript.js` file. Scripts are created using [JavaScript](https://developer.mozilla.org/en-US/docs/Web/JavaScript) (JS), which is fairly easy to learn. Since the JS code is not running in a browser, but inside of the game itself, most browser front-end APIs are missing (such as [`window._`](https://developer.mozilla.org/en-US/docs/Web/API/Window) and [`document._`](https://developer.mozilla.org/en-US/docs/Web/API/Document)) but we don't need those to create mods anyway.

Inside of our script, we may want to create something called a 'patcher'. A patcher just modifies the game's assets when theyre loaded. You can register a patcher with the `souped.registerPatcher` function. However, there is an easier version of the function called `souped.registerJsonPatcher` which makes modifying the game's json a little bit easier.

To register a patcher, simply add this into your mod's JS file:

```js
souped.registerJsonPatcher("simulation", "tack_shooter.tower_blueprint", myPatcher);
```

Now you may be wondering what `myPatcher` is. It's important to create the actual patcher itself _before_ that line in order for it to be registered. Your code may look like the following:

```js
function myPatcher(bundleName, fileName, data) {
    return { successful: false, data: data };
};

souped.registerJsonPatcher("simulation", "tack_shooter.tower_blueprint", myPatcher);
```

The next missing piece of this puzzle is the `"tack_shooter.tower_blueprint"` string. This parameter of the function is a 'selector' for which files this patcher should patch. If you were to write `".tower_blueprint"`, that would mean every single file with `".tower_blueprint"` in its file name would be patched by this patcher, effectively giving you a way to modify every single tower in the game with a single patcher.
The `"simulation"` is the name of the bundle (.jet file) containing the `"tack_shooter.tower_blueprint"` file. If you want to search *every* bundle, you can put `"*"` instead.

But what is a "tower_blueprint" file?

### Analyzing the dump

When you launch BTDB2 with SMF, it will create a 'dump' folder. Inside of this folder holds all of the assets that your game has loaded while SMF is loaded. The folders inside are named in the format of `<name>_<hash>.jet` so they may be confusing to read. Either way, the one with the name "simulation" contains the files for in-match assets. We can find the "tack_shooter.tower_blueprint" file inside of the "game_data/game_project/assets/towers/tack_shooter/data" folder. You can open this file in [VS Code](https://code.visualstudio.com/). This is the recommended code editor to use for making SMF mods. When you open the file, click the 'Plain Text' button in the bottom right. This will give you the option to change the file type inside of VS Code, change it to "JSON". This will allow you to format the document by pressing `ctrl`+`shift`+`p`, and then typing "Format Document". This will make the JSON a lot easier to read.

### Scripting part 2

Now that we have a "tower_blueprint" file we want to edit, we can modify it in our script. Lets say we want to change the 'range' variable of the JSON

tack_shooter.tower_blueprint example:

```json
{
    ...
    "variables": [
        {
            "key": "range",
            "variable": {
                "type": "Float",
                "value": 23.0
            }
        },
        ...
    ]
}
```

Our script:

```js
function myPatcher(bundleName, fileName, data) {
    //Get the variables
    var tackVars = data["variables"];
    //Loop through the variables to find the 'range' variable
    for(var i = 0; i < tackVars.length; i++) {
        //Get the current variable
        var currentVar = tackVars[i];
        //Check if it is the range variable
        if(currentVar["key"] == "range") {
            //Get the variable object
            var rangeVar = currentVar["variable"];
            //Set its value
            rangeVar["value"] = 99;
            //Store the variable back into the current var
            currentVar["variable"] = rangeVar;
        }
        //Store the current var back into tackVars
        tackVars[i] = currentVar;
    }
    //Store tackVars back into the data
    data["variables"] = tackVars;

    //Return that it was successful, as well as the modified data
    return { successful: true, data: data };
};

souped.registerJsonPatcher("simulation", "tack_shooter.tower_blueprint", myPatcher);
```

### Sharing your mod

Now that you have created a mod, you may want to share it with others. You will need [7-zip](https://www.7-zip.org/) to do this part of the tutorial. The best way to do this is to distribute them as ".smf" files. To create a .smf file for your mod, select all of the files inside of your mod folder and then right-click → 7-zip → Add to archive... Select 'tar' for the "Archive format". Rename the file to replace '.tar' with '.smf' (this tells SMF that the file _is_ a tar file containing a valid 'meta.json' file). You now have a shareable mod file!
