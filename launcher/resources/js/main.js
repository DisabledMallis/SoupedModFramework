// This is just a sample app. You can structure your Neutralinojs app code as you wish.
// This example app is written with vanilla JavaScript and HTML.
// Feel free to use any frontend framework you like :)
// See more details: https://neutralino.js.org/docs/how-to/use-a-frontend-library

function onWindowClose() {
	Neutralino.app.exit();
}

Neutralino.init();

var fs = Neutralino.filesystem;
var os = Neutralino.os;

async function loadSettings() {
	try {
		return JSON.parse(await fs.readFile("config.json"));
	} catch(ex) {
		console.log("Read error: "+ex.message);
		return {
			debug: false
		};
	}
}
function saveSettings(data) {
	var dStr = JSON.stringify(data);
	fs.writeFile("config.json", dStr);
}

Neutralino.events.on("windowClose", onWindowClose);

window.nativeLaunch = async function() {
	try {
		await fs.moveFile("./proxies/Winhttp.dll", "./Winhttp.dll");
		await os.execCommand("btdb2_game.exe");
	} catch(ex) {
		os.showMessageBox("Launch Error", ex.message, 'OK', 'ERROR');
		return false;
	}
}

window.areWeNative = function() {
	return true;
}

window.nativeSetOption = async function(settingId, value) {
	console.log("Saving "+settingId+" as "+value);
	var data = await loadSettings();
	data[settingId] = value;
	saveSettings(data);
}
window.nativeGetOption = async function(settingId) {
	console.log("Reading "+settingId);
	var data = await loadSettings();
	return data[settingId];
}