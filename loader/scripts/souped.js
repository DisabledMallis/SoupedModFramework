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
    const matches = [...line.matchAll(csvRegex)].map(m => m[2]);
    matches.pop();
    return matches;
}

//Patcher helper functions
souped.registerCsvPatcher = function (callback, filename) {
    souped.registerPatcher((name, data) => {
        const lines = data.split("\n");
        const headers = match(lines.shift());

        const dataObj = lines.map(line => {
            return match(line).reduce((acc, cur, i) => {
                const val = cur.length <= 0 ? null : Number(cur) || cur;
                const key = headers[i] ?? `extra_${i}`;
                return { ...acc, [key]: val };
            }, {});
        });

        var { successful, data } = callback(dataObj);
        if (successful) {
            const keys = [...new Set(data.map(e => Object.keys(e)).flat())];
            const patchedStr = `${keys.join(',')}\n` + data.map(e => Object.values(e).join(',')).join('\n');
            return { successful: successful, data: patchedStr };
        }

        return { successful: false, data: data };
    }, filename);
}

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
