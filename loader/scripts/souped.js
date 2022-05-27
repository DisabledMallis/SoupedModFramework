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
souped.registerAssetCsvPatcher = function (bundleName, fileName, callback) {
    souped.registerPatcher(bundleName, fileName, (bundleName, fileName, fileContent) => {
        const lines = fileContent.split('\n');

        const dataObj = lines.map(line => {
            const kvp = line.split(',');
            return { [kvp[1]]: kvp[0] };
        });
        dataObj.pop();
        
        var { successful, data } = callback(bundleName, fileName, dataObj);
        if (successful) {
            const patchedStr = data.map(kvp => `${Object.values(kvp)[0]},${Object.keys(kvp)[0]}`).join('\n');
            return { successful: successful, data: patchedStr };
        }
        
        return { successful: false, data: fileContent };
    });
}

souped.registerDataCsvPatcher = function (bundleName, fileName, callback) {
    souped.registerPatcher(bundleName, fileName, (bundleName, fileName, fileContent) => {
        const lines = fileContent.split('\n');
        const headers = match(lines.shift());

        const dataObj = lines.map(line => {
            return match(line).reduce((acc, cur, i) => {
                if (headers[i] !== null) {
		    const val = cur.length <= 0 ? null : Number(cur) || cur;
                    return { ...acc, [headers[i]]: val };
                }
            }, {});
        });

        var { successful, data } = callback(bundleName, fileName, dataObj);
        if (successful) {
            const keys = [...new Set(data.map(e => Object.keys(e)).flat())];
            const patchedStr = `${keys.join(',')}\n` + data.map(e => Object.values(e).join(',')).join('\n');
            return { successful: successful, data: patchedStr };
        }

        return { successful: false, data: fileContent };
    });
}

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

souped.registerFileOverride = function (bundleName, fileName, modifyable, pathInMod, callback) {
    souped.registerOverride(bundleName, fileName, modifyable, (bundleName, fileName, fileContent, isPost) => {
		try {
			var overrideContent = souped.mfs.readFile(pathInMod);
			return { successful: true, data: overrideContent };
		} catch(ex) {
			console.warn(ex.message);
		}
		return { successful: false, data: fileContent };
	});
}
