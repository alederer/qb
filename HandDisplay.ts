import CardDisplay from "./CardDisplay";
import G from "./G";
import Util from "./Util";
import QB from "./QBWrap";

export default class HandDisplay {
	faction;
	root = document.createElement("div");
	handDiv = document.createElement("div");
	deckDiv = document.createElement("div");
	cardDisplays: CardDisplay[] = [];
	passButton = document.createElement("button");
	onPass;
	interactible = true;
	multiSelect = false;
	handSize = 1;

	mouseX = 0;
	mouseY = 0;
	hover = false;

	constructor(faction, root: HTMLElement) {
		this.faction = faction;
		this.passButton.innerText = "pass";
		this.passButton.onclick = () => this.onPass();
		this.handDiv.append(this.passButton);

		for (let ii = 0; ii < G.MAX_DECK_SIZE; ++ii) {
			let display = new CardDisplay(this.handDiv, this);
			this.cardDisplays.push(display);
		}

		this.root.className = "playerHandDisplay";
		this.handDiv.className = "playerHand";
		this.deckDiv.className = "playerDeck";
		this.root.appendChild(this.handDiv);
		this.root.appendChild(this.deckDiv);
		this.root.onmouseover = () => this.hover = true;
		this.root.onmouseout = () => this.hover = false;
		this.root.onmousemove = ev => {
			this.mouseX = ev.x - this.root.offsetLeft;
			this.mouseY = ev.y - this.root.offsetTop;
		}

		root.appendChild(this.root);
	}

	select(card: CardDisplay|null) {
		let idx = this.getSelectedIdx();

		if (!this.multiSelect && idx >= 0) {
			let cur = this.cardDisplays[idx];
			if (cur != card) {
				cur.setSelected(false);
			}
		}

		if (card && !card.selected) {
			card.setSelected(true);
		}
		else if (card) {
			card.setSelected(false);
		}
	}

	getCard(idx) {
		console.assert(idx >= 0);
		console.assert(idx < this.cardDisplays.length);
		console.assert(!!this.cardDisplays[idx]);
		return this.cardDisplays[idx].card;
	}

	getHoveredIdx() {
		return this.cardDisplays.findIndex(card => card.hovered);
	}

	getSelectedIdx() {
		return this.cardDisplays.findIndex(card => card.selected);
	}

	getSelectedMask() {
		let mask = 0;
		for( let ii = 0; ii < this.cardDisplays.length; ++ii )
		{
			if( this.cardDisplays[ii].enabled && this.cardDisplays[ii].selected ) {
				mask |= 1 << ii;
			}
		}
		return mask;
	}

	updateFrom(board, playerIdx) {
		let deckAndHand = board.GetDeckAndHand(playerIdx);
		this.handSize = deckAndHand.handSize;
		let w = (this.handDiv.offsetWidth - 100) / deckAndHand.handSize;
		let selected = this.getSelectedIdx();

		let pos = new Array(this.handSize).fill(0);

		for (let ii = 0; ii < G.MAX_DECK_SIZE; ++ii) {
			let display = this.cardDisplays[ii];
			let highlight = display.hovered || display.selected;
			display.div.style.zIndex = display.hovered ? "3" : display.selected ? "1" : "0";
			if (ii < deckAndHand.handSize) {
				display.setEnabled(true);
				// display.setOffset( ii * w - (highlight ? 10 : 0), highlight ? -10 : 0 );
				// let x = display.getOffsetX();
				// x += 0.8 * (this.getIdealPos(ii) - x);
				// display.setOffsetX(x);
				let idx = deckAndHand.GetCard(ii);
				console.assert(idx > 0);
				display.updateFrom(QB.CardDb.prototype.GetByIdx(idx));
			}
			else {
				display.setEnabled(false);
			}
		}

		this.updatePosAndHover(board);

		this.deckDiv.innerHTML = board.GetDeckAndHand( playerIdx ).deckSize;
	}

	updatePosAndHover(board) {
		let cardWidth = this.cardDisplays[0].div.offsetWidth;
		let total = this.handDiv.offsetWidth - 2 * cardWidth;
		let last = this.handSize - 1;
		let baseWidth = total / last;
		let minWidth = 10;
		let poss = new Array(this.handSize).fill(null);
		let sel = this.getSelectedIdx();
		let hov = Math.min( last, Math.floor(this.mouseX / baseWidth) );
		if( !this.hover ) { hov = -1; }

		let focus = hov >= 0 ? hov : sel;
		// let frac = 0;
		// if( this.hover ) {
		// 	sel = this.mouseX / baseWidth;
		// 	// frac = Math.floor(sel) - sel;g
		// 	sel = Math.floor(sel);
		// 	console.log(sel);
		// }

		/*
		(1) move sel if necessary to a spot far enough from the ends that it
			can be free on its left side
		(2) squeeze everything else 
		*/

		if(baseWidth < 0.9 * cardWidth) {
			for( let ii = 0; ii < this.handSize; ++ii ) {
				poss[ii] = ii * baseWidth;
			}

			for( let ii = 0; ii <= last; ++ii ) {
				if( ii == focus ) { continue; }

				let iStart = 0;
				let iEnd = last;
				let start = 0;
				let end = total;
				if( ii > focus && focus >= 0 ) { 
					iStart = focus + 1;
					start = poss[focus] + cardWidth + 10;
					end = Math.max(end, start + (last - focus) * minWidth);
				}

				let tt = iEnd == iStart ? 0.0 : (ii - iStart) / (iEnd - iStart);;
				poss[ii] = start + (end - start) * tt;
			}
		}
		else {
			for( let ii = 0; ii < this.handSize; ++ii ) {
				poss[ii] = ii * (cardWidth + 5);
			}
		}

		for (let ii = 0; ii < G.MAX_DECK_SIZE; ++ii) {
			let display = this.cardDisplays[ii];
			if( ii >= this.handSize ) { display.setVisible(false); continue; }
			display.setVisible(true);

			let y = 0;
			if( ii == sel ) {
				display.div.style.outline = `4px solid ${G.getCssColorFg(this.faction)}`;
				y = -15;
			}
			else if( ii == hov ) {
				display.div.style.outline = `2px solid ${G.getCssColorFg(this.faction)}`;
				y = -5;
			}
			else {
				display.div.style.outline = `1px solid black`;
			}

			let x0 = display.getOffsetX();
			let x = x0 + 0.5 * (poss[ii] - x0);
			let y0 = display.getOffsetY();
			y = y0 + 0.5 * (y - y0);
			// if(ii == focus) { x = poss[ii]; }
			display.setOffset(x, y);
			if(false && ii == focus) {
				display.div.style.zIndex = `${1000}`;
			}
			else {
				display.div.style.zIndex = `${100 + ii}`;
			}
		}
	}

	setInteractible(interactible) {
		for (let card of this.cardDisplays) {
			card.setSelected(false);
			card.setHovered(false);
		}

		this.interactible = interactible;
		this.passButton.disabled = !interactible;
	}
}