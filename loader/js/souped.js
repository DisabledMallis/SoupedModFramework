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

//By default, SMF specified console.print instead of console.log (because in C++ 'log' refers to the mathematical function)
console.log = console.print;
console.log("Welcome to souped.js!");

//Patcher helper functions
function createJsonPatcher(callback, filename) {
    patchers.registerPatcher((name, data) => {
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
    var towerNodes = data["nodes"];
    for (var i = 0; i < towerNodes.length; i++) {
        var currentNode = towerNodes[i];
        if (currentNode["id"] == 0) {
            var towerProps = currentNode["props"];
            data["nodes"][i]["cost"] = 100;
            console.warn("DartMonkey patch successfully!");
            return { successful: true, data: data };
        }
    }
    return { successful: false, data: data };
}

createJsonPatcher(dartPatch, "dart_monkey.tower_blueprint");