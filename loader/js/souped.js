/*
 * Souped.js - The JavaScript library for making BTDB2 mods.
 * 
 * For documentation, please visit: https://souped.dev
 * Join the community! Discord: https://discord.gg/nPcBPyHP4c
 * 
 */

//By default, SMF specified console.print instead of console.log (because in C++ 'log' refers to the mathematical function)
console.log = console.print;
console.log("Welcome to souped.js!");

//Patcher helper functions
function createPatcher(callback, filename) {
    patchers.registerPatcher((name, data) => {
        if (name.includes(filename)) {
            var dataObj = JSON.parse(data);
            return callback(dataObj);
        }
        return { successful: false, data: undefined };
    });
}

function dartPatch(data) {
    console.error("Hello dart mokey!");
    return { successful: false, data: data };
}
createPatcher(dartPatch, "dart_monkey")