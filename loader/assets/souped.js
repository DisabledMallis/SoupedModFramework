/*
 * Souped.js - The JavaScript library for making BTDB2 mods.
 *
 * Don't edit this file! This file contains important functions that mods
 * may rely on! If you change this file, a lot of mods may break!
 * 
 * For documentation, please visit: https://souped.dev
 * Join the community! Discord: https://discord.gg/nPcBPyHP4c
 * 
 */

console.log("Welcome to souped.js!");

//Patcher helper functions
souped.registerJsonPatcher = function(callback, filename) {
    souped.registerPatcher((name, data) => {
        var dataObj = JSON.parse(data);
        var { successful, data } = callback(dataObj);
        if (successful) {
            var patchedStr = JSON.stringify(data);
            return { successful: successful, data: patchedStr };
        }
        return { successful: false, data: data };
    }, filename);
}

function dartPatch(data) {
    console.log("Dart type: " + data["type"]);
    var towerVars = data["variables"];
    for (var i = 0; i < towerVars.length; i++) {
        var currentVar = towerVars[i];
        if (currentVar["key"] == "range") {
            currentVar["variable"]["value"] = 99;
        }
        if (currentVar["key"] == "proj_radius") {
            currentVar["variable"]["value"] = 99;
        }
        if (currentVar["key"] == "reload_time") {
            currentVar["variable"]["value"] = 0.01;
        }
        towerVars[i] = currentVar;
    }

    data["variables"] = towerVars;
    var towerNodes = data["nodes"];
    for (var i = 0; i < towerNodes.length; i++) {
        var currentNode = towerNodes[i];
        if (currentNode["id"] == 0) {
            currentNode["cost"] = 0;
            window.notif("DartMonkey patched!");
            console.warn("DartMonkey patch successfully!");
        }
        towerNodes[i] = currentNode;
    }
    data["nodes"] = towerNodes;
    return { successful: true, data: data };
}

souped.registerJsonPatcher(dartPatch, "dart_monkey.tower_blueprint");