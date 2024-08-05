import BoardDisplay from "./BoardDisplay";
import G from "./G";
import HandDisplay from "./HandDisplay";
import * as Profile from "./Profile";
import Util from "./Util";
import QB from "./QBWrap";
import MulliganPrompt from "./MulliganPrompt";
import FSM from "./FSM";

enum GameState {
	Init = "Init",
	Mulligan = "Mulligan",
	PlayerTurn = "PlayerTurn",
	AITurn = "AITurn",
	Resolve = "Resolve",
	GameOve = "GameOver"
}

let SKIP_MULLIGAN = true;

console.log(QB);
let mcts = new QB.MCTS();
let resolveQueue = new QB.ResolveQueue();

export default class Game {
	board;
	boardDisplay: BoardDisplay;
	playerHandDisplay: HandDisplay;
	enemyHandDisplay: HandDisplay;
	mulliganPrompt: MulliganPrompt;
	fsm = new FSM<GameState>();

	newBoard() {
		this.board = new QB.Board();

		let deck = new QB.JSDeck();
		deck.PushName( "Soldier", 2 );
		deck.PushName( "Cactuar", 2 );
		deck.PushName( "Spearhawk", 2 );
		deck.PushName( "Crystal Crab", 2 );
		deck.PushName( "Grenadier", 2 );
		deck.PushName( "Chocobo & Moogle", 1 );
		deck.PushName( "Riot Trooper", 4 );
		
		this.board.Init( deck, deck );

		if( SKIP_MULLIGAN ) {
			this.board.Mulligan(0, 0);
			this.board.Mulligan(1, 0);
		}
	}

	constructor(root: HTMLElement) {
		Profile.setEnabled(false);

		this.newBoard();

		let newButton = document.createElement("button");
		newButton.innerText = "new session";
		newButton.onclick = () => {
			// this.newBoard();
			window.localStorage.removeItem("board");
			window.location.reload();
		};
		root.appendChild(newButton);

		// window.localStorage.setItem("board", "AAAAAAAAAAQABAAAAAAAAAAAAIAAAAAAAAAAAEAAAAAAAAAAAAgAAAAAAAAAAAAADAAAAAAAAAAAAQwAAAAAAAAAAAQBCAAAAAAAAAAAGAAAAAAAAAAAAcAAAAAAAAAAAAACAAAAAAAAAIAAJUAAAAAAAAAAAogAAAAAAAAAAAwCAAAAAAAAAAAAMAAAAAAAAAAAA0wAGAAAAAAAAAgDDQAAFAABAIAAHAgAAMAAHAgBAEAAEAgBAQAAFAQAAQwCBAQBAQAAFAQAAQAACAwBAYAADAgAAIAAEAwBAQAAEoAAAAAAAAAALAA");
		if( window.localStorage.getItem("board") ) {
			console.log(window.localStorage.getItem("board"));
			this.board = QB.StringToBoard( window.localStorage.getItem("board") );
		}

		let enemyHandDiv = document.createElement("div");
		this.enemyHandDisplay = new HandDisplay(1, enemyHandDiv);
		this.enemyHandDisplay.setInteractible(false);
		Util.setVisible(this.enemyHandDisplay.passButton, false);
		root.appendChild(enemyHandDiv);

		let boardDiv = document.createElement("div");
		this.boardDisplay = new BoardDisplay(boardDiv);
		this.boardDisplay.board = this.board;
		root.appendChild(boardDiv);

		let mulliganDiv = document.createElement("div");
		this.mulliganPrompt = new MulliganPrompt(mulliganDiv);
		this.mulliganPrompt.onMulligan = () => this.onMulligan();
		this.mulliganPrompt.onMulliganPass = () => this.onMulliganPass();
		boardDiv.appendChild(mulliganDiv);

		let handDiv = document.createElement("div");
		this.playerHandDisplay = new HandDisplay(0, handDiv);
		this.playerHandDisplay.onPass = () => this.onPlayEvent(-1, 0, 0);
		root.appendChild(handDiv);

		this.boardDisplay.handDisplay = this.playerHandDisplay;
		this.boardDisplay.onPlayEvent = this.onPlayEvent.bind(this);
		this.boardDisplay.interactible = true;

		// this.testInit();
		Profile.logAndReset();

		// this.mulliganPrompt.root.style["display"] = "none";
		Util.setVisible(this.mulliganPrompt.root, false);
		this.setupFSM(this.fsm.states);

		this.fsm.setState(this.board.HasMulliganed(0) ? GameState.PlayerTurn : GameState.Mulligan);
	}

	setupFSM(states) {
		states[GameState.Mulligan].enter = () => {
			this.playerHandDisplay.multiSelect = true;
			this.playerHandDisplay.interactible = true;
			Util.setVisible(this.mulliganPrompt.root, true);
			this.playerHandDisplay.select(null);
		};
		states[GameState.Mulligan].exit = () => {
			this.playerHandDisplay.multiSelect = false;
			this.playerHandDisplay.interactible = false;
			this.playerHandDisplay.select(null);
			Util.setVisible(this.mulliganPrompt.root, false);
		};

		states[GameState.PlayerTurn].enter = () => {
			this.playerHandDisplay.interactible = true;
		}
		states[GameState.PlayerTurn].exit = () => {
			this.playerHandDisplay.interactible = false;
		}
	}

	onPlayEvent(handCardIdx: number, x: number, y: number) {
		if (handCardIdx >= 0) {
			this.playerHandDisplay.cardDisplays[handCardIdx].setSelected(false);
		}

		this.boardDisplay.setPreview(null);

		if (handCardIdx >= 0) {
			this.board.QueuePlay( this.board.turn, handCardIdx, y * G.BOARD_WIDTH + x, resolveQueue );
			this.board.log = new QB.BoardLog();
			this.board.Apply( resolveQueue, true );
			// this.board.DebugPrint();
			for( let ii = 0; ii < this.board.log.eventCount; ++ii ) {
				let ble = this.board.log.GetEvent(ii);
				if( ble.change == QB.kChange_Rank ) {

				}
			}
			this.board.log = null;
			this.playerHandDisplay.updateFrom(this.board, 0);
		}
		else {
			this.board.Pass( this.board.turn );
		}

		this.boardDisplay.updateFrom(this.board);

		this.boardDisplay.interactible = false;
		this.board.turn = 1 - this.board.turn;

		let play = mcts.SearchBestMove( this.board, 1.0 );
		console.log(play);

		if( play.isMulligan )
		{
			this.board.Mulligan( this.board.turn, play.handIdx );
		}
		else
		{
			this.board.QueuePlay( this.board.turn, play.handIdx, play.playIx, resolveQueue );
			this.board.log = new QB.BoardLog();
			this.board.Apply( resolveQueue, true );
			this.board.log = null;
		}
		this.board.Deal( this.board.turn, 1, true );

		this.board.turn = 1 - this.board.turn;
		this.board.Deal( this.board.turn, 1, true );
		this.boardDisplay.interactible = true;

		window.localStorage.setItem("board", QB.BoardToString(this.board));

		this.playerHandDisplay.updateFrom(this.board, 0);
		this.enemyHandDisplay.updateFrom(this.board, 1);
		this.boardDisplay.updateFrom(this.board);
	}

	onMulligan() {
		this.mulligan(this.playerHandDisplay.getSelectedMask());
	}

	onMulliganPass() {
		this.mulligan(0);
	}

	mulligan(mask) {
		this.board.Mulligan(this.board.turn, mask);
		this.playerHandDisplay.updateFrom(this.board, 0);
		this.board.turn = 1 - this.board.turn;

		let play = mcts.SearchBestMove( this.board, 1.0 );
		console.assert(play.isMulligan);
		this.board.Mulligan(this.board.turn, play.handIdx);
		this.enemyHandDisplay.updateFrom(this.board, 1);
		this.board.turn = 1 - this.board.turn;

		this.fsm.setState(GameState.PlayerTurn);
	}

	update(dt: number) {
		this.fsm.update(dt);
		this.enemyHandDisplay.updateFrom(this.board, 1);
		this.playerHandDisplay.updateFrom(this.board, 0);
		this.boardDisplay.updateFrom(this.board);
	}
}