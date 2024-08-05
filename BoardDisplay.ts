import G from "./G";
import HandDisplay from "./HandDisplay";
import Util from "./Util";
import HTMLElementListenerSet from "./HTMLElementListenerSet";
import QB from "./QBWrap";
import DOMSprite from "./DOMSprite";
import Vec2 from "./Vec2";

// type BoardGen = number;
// class BoardAnimCo {
// 	co: 
// 	*update(dt);
// }

type BoardAnimFun = (dt: number, time: number) => boolean;

class BoardAnim {
	time = 0.0;
	barrier = false;
	fun: BoardAnimFun|undefined;

	BoardAnim(fun?: BoardAnimFun) {
		this.fun = fun;
	}

	update(dt: number) {
		let result = this.fun ? this.fun(dt, this.time) : true;
		this.time += dt;
		return result;
	}

	static Barrier() {
		let anim = new BoardAnim();
		anim.barrier = true;
		return anim;
	}
}

class BoardCellDisplay {
	x: number;
	y: number;
	root;
	pawns = new Array<DOMSprite>();
	name: DOMSprite;
	score: DOMSprite;
	bonus0: DOMSprite;
	bonus1: DOMSprite;
	// rank = document.createElement("p");
	// name = document.createElement("p");
	// score = document.createElement("p");
	listenerSet = new HTMLElementListenerSet();
	rankUpAnimT = 0.0;

	constructor(root, x, y) {
		this.x = x;
		this.y = y;
		this.root = root;

		this.root.style["position"] = "relative";
		this.root.style["text-align"] = "center";

		this.name = new DOMSprite(root);
		this.name.div.style.fontSize = "1.0em";
		this.name.div.style.width = "90%";
		this.name.setBasePos(0, 0);

		this.score = new DOMSprite(root);
		this.score.div.style.fontSize = "1.5rem";
		this.score.setBasePos(1, 1, .5, .5 );

		this.bonus0 = new DOMSprite(root);
		this.bonus0.div.style.fontSize = "1.5rem";
		this.bonus0.div.style.color = G.getCssColorFg(0);
		this.bonus0.setBasePos(0, -1, 0, .5 );
		this.bonus1 = new DOMSprite(root);
		this.bonus1.div.style.fontSize = "1.5rem";
		this.bonus1.div.style.color = G.getCssColorFg(1);
		this.bonus1.setBasePos(0, -1, 0, .5 );

		for (let ii = 0; ii < 3; ++ii ) {
			this.pawns[ii] = new DOMSprite(this.root);
			this.pawns[ii].setBasePos(0, 0);
			this.pawns[ii].div.innerText = G.PAWN;
			this.pawns[ii].div.style.fontSize = "2em";
		}
	}

	setListeners(listeners) {
		this.listenerSet.update(this.root, listeners);
	}

	updateFrom(board, board0, x, y) {
		let space = board.GetSpace( y * G.BOARD_WIDTH + x );
		let space0 = board0.GetSpace( y * G.BOARD_WIDTH + x );
		this.root.style.backgroundColor = G.getCssColorBg(space.faction);
		let center = this.pawns[0].getParentSize().mul(0.5);

		if(space.card || space0.card) {
			let card = space.card != 0 ? space.card : space0.card;
			for( let ii = 0; ii < G.MAX_RANK; ++ii ) {
				this.pawns[ii].setVisible(false);
			}

			let name = QB.CardByIdx(card).name;;
			if( this.name.div.innerText != name ) {
				// this.name.div.innerText = name;
				this.name.setTextCentered(name, 1.5, 95);
			}
			
			this.name.setVisible(true);
			this.name.setBasePos(0, 0);
			this.name.div.style.color = G.getDeltaColor( space.card - space0.card );

			let isNew = space.card > space0.card;

			// let parentSize = this.score.getParentSize();
			// let scoreSize = this.score.getSize();
			this.score.div.innerText = space.GetTotalScore();
			this.score.setVisible(true);
			// this.score.div.style.color = G.getDeltaColor( isNew ? space.GetAddScore() : space0.card ? space.GetTotalScore() - space0.GetTotalScore() : 0 );
			this.score.div.style.color = G.getDeltaColor( space.GetAddScore() );
		}
		else {
			this.name.setVisible(false);
			this.score.setVisible(false);

			let width = this.pawns[0].getSize().x;
			for( let ii = 0; ii < G.MAX_RANK; ++ii ) {
				let x = -width * (space.rank - 1) * 0.5;
				x += width * ii;
				this.pawns[ii].div.style.color = G.getCssColorFg(space.faction);
				this.pawns[ii].setOffsetX(x);
				this.pawns[ii].setVisible(ii < space.rank );
			}
		}

		let bonus0 = space.GetFactionAddPoints(0);
		let bonus1 = space.GetFactionAddPoints(1);
		this.bonus0.div.innerText = bonus0 > 0 ? `+${bonus0}` : bonus0 < 0 ? `${bonus0}` : "";
		this.bonus1.div.innerText = bonus1 > 0 ? `+${bonus1}` : bonus1 < 0 ? `${bonus1}` : "";
		if( bonus0 && bonus1 ) {
			this.bonus0.setOffsetX(-3);
			this.bonus1.setOffsetX(3);
		}
		else {
			this.bonus0.setOffsetX(0);
			this.bonus1.setOffsetX(0);
		}
	}
}

export default class BoardDisplay {
	board;
	previewBoard;

	scoreCells : HTMLDivElement[][];
	cells : BoardCellDisplay[][];
	handDisplay: HandDisplay;
	previewCell: BoardCellDisplay|null;
	interactible = false;
	onPlayEvent: (cardIdx: number, x: number, y: number) => void;
	curPlayer = 0;

	animQueue = new Array<BoardAnim>();

	constructor(root) {
		this.scoreCells = Array(2).fill(0).map(() =>
			Array(G.BOARD_HEIGHT).fill(null));
		this.cells = Array(G.BOARD_WIDTH).fill(0).map(() =>
			Array(G.BOARD_HEIGHT).fill(null));

		// let test = document.createElement("div");
		root.className = "board";

		for (let ii = 0; ii < 2; ++ii) {
			for (let jj = 0; jj < G.BOARD_HEIGHT; jj++) {
				let cell = document.createElement("div");
				cell.className = "boardCell";
				let x = (G.BOARD_WIDTH + 1) * ii + 1;
				cell.style.gridColumn = `${x} / ${x + 1}`;
				cell.style.gridRow = `${jj + 1} / ${jj + 2}`;
				root.appendChild(cell);
				this.scoreCells[ii][jj] = cell;
			}
		}

		for (let ii = 0; ii < G.BOARD_WIDTH; ++ii) {
			for (let jj = 0; jj < G.BOARD_HEIGHT; jj++) {
				let cellDiv = document.createElement("div");
				cellDiv.className = "boardCell";
				let x = ii + 2;
				cellDiv.style.gridColumn = `${x} / ${x + 1}`;
				cellDiv.style.gridRow = `${jj + 1} / ${jj + 2}`;
				root.appendChild(cellDiv);
				let cell = new BoardCellDisplay(cellDiv, ii, jj)
				this.cells[ii][jj] = cell;

				cell.setListeners({
					mouseover: (ev: Event) => this.setPreview(cell),
					mouseout: (ev: Event) => this.setPreview(null),
					click: (ev: Event) => this.onClick(cell),
				});
			}
		}
	}

	onClick(cell: BoardCellDisplay) {
		if (!this.interactible) { return; }

		let selectedIdx = this.handDisplay.getSelectedIdx();
		if (selectedIdx < 0) { return; }
		if (this.onPlayEvent) {
			let card = this.handDisplay.getCard(selectedIdx);
			if (this.board.CanPlay(this.curPlayer, selectedIdx, cell.y * G.BOARD_WIDTH + cell.x )) {
				this.onPlayEvent(selectedIdx, cell.x, cell.y);
			}
		}
	}

	setPreview(cell: BoardCellDisplay|null) {
		if (!this.interactible) { return; }

		if (this.previewCell != cell ) {
			this.previewBoard = null;
			this.previewCell = null;
		}

		let selectedIdx = this.handDisplay.getSelectedIdx();

		if (cell && selectedIdx >= 0) {
			this.previewCell = cell;
			this.previewBoard = new QB.Board();
			this.board.CloneInto( this.previewBoard );
			this.previewBoard.silent = true;
			let ix = cell.y * G.BOARD_WIDTH + cell.x;
			if (this.board.CanPlay(this.curPlayer, selectedIdx, ix )) {
				let resolveQueue = new QB.ResolveQueue();
				this.previewBoard.QueuePlay(this.curPlayer, selectedIdx, ix, resolveQueue );
				this.previewBoard.Apply(resolveQueue, true);
			}

			this.updateDisplay(this.previewBoard);
		}
		else
		{
			this.updateDisplay(this.board);
		}
	}

	updateDisplay(board) {
		for (let ii = 0; ii < G.BOARD_WIDTH; ++ii) {
			for (let jj = 0; jj < G.BOARD_HEIGHT; ++jj) {
				let cell = this.cells[ii][jj];
				cell.updateFrom(board, this.board, ii, jj);
			}
		}

		for (let ii = 0; ii < 2; ++ii) {
			for (let jj = 0; jj < G.BOARD_HEIGHT; ++jj) {
				let cell = this.scoreCells[ii][jj];
				cell.innerText = `${board.GetRowScore( ii, jj )}`;
			}
		}
	}

	updateFrom(board) {
		this.board = board

		if (this.previewBoard) {
			this.updateDisplay(this.previewBoard);
		}
		else {
			this.updateDisplay(this.board);
		}
	}
}
