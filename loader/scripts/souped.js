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
souped.registerJsonPatcher = function (bundleName, fileName, callback) {
    souped.registerPatcher(bundleName, fileName, (bundleName, fileName, fileContent) => {
        var dataObj = JSON.parse(fileContent);
        var { successful, data } = callback(bundleName, fileName, dataObj);
        if (successful) {
            var patchedStr = JSON.stringify(data);
            return { successful: successful, data: patchedStr };
        }
        return { successful: false, data: fileContent };
    });
}