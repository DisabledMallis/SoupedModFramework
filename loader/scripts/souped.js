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
souped.registerJsonPatcher = function (callback, filename) {
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