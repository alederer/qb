import G from "./G";
import Util from "./Util";
import HTMLElementListenerSet from "./HTMLElementListenerSet";
import type HandDisplay from "./HandDisplay";
import QB from "./QBWrap";
import DOMSprite from "./DOMSprite";

class CardEffectDisplayCell {
	/** @type {HTMLElement} */
	root;
	affectsRank = false;
	affectsScore = false;
	constructor(root) {
		this.root = root;
		this.reset();
	}

	reset() {
		this.root.style["backgroundColor"] = "grey";
		this.root.style["outline"] = "0px";
		this.affectsRank = false;
		this.affectsScore = false;
	}

	updateCenter() {
		this.root.style["backgroundColor"] = "white";
	}
}

class CardEffectDisplay {
	root : HTMLElement;
	cells: CardEffectDisplayCell[][] = [];
	constructor(root) {
		this.root = root;

		// @ts-ignore
		this.cells = Array(G.EFFECT_DISPLAY_GRID_WIDTH).fill(0).map(() =>
			Array(G.EFFECT_DISPLAY_GRID_HEIGHT).fill(0).map(() => null));

		for (let jj = 0; jj < G.EFFECT_DISPLAY_GRID_HEIGHT; ++jj ) {
			let row = document.createElement("div");
			row.className = "cardEffectRow";
			for (let ii = 0; ii < G.EFFECT_DISPLAY_GRID_WIDTH; ++ii ) {
				let cell = document.createElement("div");
				cell.className = "cardEffectCell";
				row.appendChild(cell);

				this.cells[ii][jj] = new CardEffectDisplayCell(cell);
			}
			this.root.appendChild(row);
		}
	}

	updateFrom(card) {
		let ox = Math.floor(G.EFFECT_DISPLAY_GRID_WIDTH / 2);
		let oy = Math.floor(G.EFFECT_DISPLAY_GRID_HEIGHT / 2);
		for (let ii = 0; ii < this.cells.length; ++ii) {
			for (let jj = 0; jj < this.cells[ii].length; ++jj) {
				this.cells[ii][jj].reset();
			}
		}
		this.cells[ox][oy].updateCenter();
		// card.effects.forEach(effect => {
		// 	this.cells[effect.offsetX + ox][effect.offsetY + oy].updateCumulative(effect);
		// });
	}
}

export default class CardDisplay extends DOMSprite {
	enabled = false;
	selected = false;
	hovered = false;
	handDisplay: HandDisplay|undefined;
	card;
	rank: DOMSprite;
	score: DOMSprite;
	name: DOMSprite;
	effect: DOMSprite;
	desc: DOMSprite;
	// rank = document.createElement("div");
	// score = document.createElement("div");
	// name = document.createElement("div");
	// effect = document.createElement("div");
	// desc = document.createElement("div");
	effectDisplay: CardEffectDisplay;
	listeners = new HTMLElementListenerSet();

	constructor(root, hand?: HandDisplay) {//, listeners: Map<string, EventListener>) {
		super(root);

		this.div.classList.add("card");
		// this.div.style.borderRadius = ".5rem";
		// this.div.style.border = "1px solid black";
		// this.div.style.width = "7rem";
		// this.div.style.height = "15rem";
		this.handDisplay = hand;

		this.rank = new DOMSprite(this.div);
		this.rank.setBasePos(-1, 1, .25, .1);
		this.rank.div.style.fontSize = "2rem";
		this.score = new DOMSprite(this.div);
		this.score.setBasePos(-1, -1, .25, .1);
		this.score.div.style.fontSize = "2rem";
		this.effect = new DOMSprite(this.div);
		this.effect.setBasePos(0, 0);
		this.effect.div.classList.add("cardEffect");
		this.name = new DOMSprite(this.div);
		this.name.setBasePos(-1, 1, 0.25, 0.25);
		this.name.setOffsetY(30);
		this.name.anchorCenterY();
		this.desc = new DOMSprite(this.div);

		// this.rank.className = "cardRank";
		// this.score.className = "cardScore";
		// this.name.className = "cardName";
		// this.effect.className = "cardEffect";
		// this.desc.className = "cardDesc";

		this.rank.div.style["color"] = G.getCssColorFg(hand?.faction ?? 0);;
		this.effectDisplay = new CardEffectDisplay(this.effect.div);
		// this.div.appendChild(this.effectDispay);

		// this.div.appendChild(this.name)
		// this.div.appendChild(this.desc);

		this.div.addEventListener("mouseover", ev => {
			this.setHovered(true);
		});
		this.div.addEventListener("mouseout", ev => {
			this.setHovered(false);
		});
		this.div.addEventListener("click", ev => {
			if (this.handDisplay) {
				this.handDisplay.select(this);
			}
		});
	}

	setEnabled(enabled) {
		if (this.enabled && !enabled) {
			this.setHovered(false);
			this.setSelected(false);
		}
		this.enabled = enabled;
		this.setVisible(enabled);
	}

	setHovered(hovered) {
		if (!this.handDisplay?.interactible) { hovered = false; }
		this.hovered = hovered;
		// if (hovered) {
		// 	this.root.classList.add("cardHovered");
		// }
		// else {
		// 	this.root.classList.remove("car3dHovered");
		// }
	}

	setSelected(selected) {
		if (!this.handDisplay?.interactible) { selected = false; }
		this.selected = selected;
		// if (selected) {
		// 	this.root.classList.add("cardSelected");
		// }
		// else {
		// 	this.root.classList.remove("cardSelected");
		// }
	}

	updateFrom(card) {
		this.card = card;
		this.name.setTextCentered(card.name, 1.5, 90);
		this.name.div.style.textAlign = "left";
		this.desc.div.innerText = card.desc;
		this.rank.div.innerText = Array(card.rank).fill(G.PAWN).join("");
		this.score.div.innerText = card.score.toString();
		this.effect.setVisible(true);
		this.effectDisplay.updateFrom(card);
	}
}
