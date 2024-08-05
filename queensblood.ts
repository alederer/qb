import G from './G';
import BoardDisplay from './BoardDisplay';
import CardDisplay from './CardDisplay';
import HandDisplay from './HandDisplay';
import QB from "./QBWrap";
import Game from './Game';
import Tests from './Tests';

Tests();

console.log("loaded");

function logD(...args) {
	console.log(...args);
}

if (!document.body) { document.body = document.createElement("body"); }

let DIV_ROOT = document.createElement("div");
document.body.appendChild(DIV_ROOT);

let GAME_DIV = document.createElement("div");
let GAME = new Game(document.body);
document.body.appendChild(GAME_DIV);

let prevTimeStamp;
function update(timeStamp: number) {
	if (prevTimeStamp === undefined) {
		prevTimeStamp = timeStamp;
	}

	const dt = timeStamp - prevTimeStamp;
	prevTimeStamp = timeStamp;

	GAME.update(timeStamp);
	requestAnimationFrame(update);
}

window["QB"] = QB;
requestAnimationFrame(update);

