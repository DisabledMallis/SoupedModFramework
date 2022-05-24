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

const csvRegex = new RegExp(`\\s*(")?(.*?)\\1\\s*(?:,|$)`, "gs");
const match = line => {
    var matches = new Array();
    const matchResult = line.match(csvRegex);
    for (let index in matchResult) {
        const item = matchResult[index];
        matches[index] = item.match(csvRegex); 
    }
    
    matches = matches.map(m => m[0].endsWith(',') ? m[0].slice(0, -1) : m[0]);
    matches.pop();
    return matches;
}

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