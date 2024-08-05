import Util from "./Util";
import Vec2 from "./Vec2";

export default class DOMSprite {
	div = document.createElement("div");
	private fitTextDirty = false;
	private textSize = 1;
	private x = 0;
	private y = 0;
	private nx = -1;
	private ny = -1;
	private marginX = 0;
	private marginY = 0;
	private shouldAnchorCenterY = false;
	private shouldAnchorCenterX = false;

	constructor(root: HTMLElement) {
		this.div.classList.add("domSprite");
		root.appendChild(this.div);
	}

	setBasePos( nx, ny, marginX?, marginY? ) {
		this.nx = nx;
		this.ny = ny;
		this.marginX = marginX ?? 0;
		this.marginY = marginY ?? 0;
		this.updatePositioning();
	}

	isVisible() {
		return this.div.offsetWidth > 0;
	}

	setVisible(visible: boolean) {
		Util.setVisible(this.div, visible);
		this.fitTextCentered();
	}

	setTextCentered(text: string, size: number, widthPct?: number) {
		if( widthPct ) {
			this.div.style.width = `${widthPct}%`;
		}
		this.textSize = size;
		this.div.style.fontSize = `${size}rem`;
		this.div.style.textAlign = "center";
		this.div.innerText = text;

		this.fitTextDirty = true;
		this.fitTextCentered();
	}

	fitTextCentered() {
		if( !this.isVisible() || !this.fitTextDirty ) { return; }

		for( let ii = 0; ii < 10; ++ii ) {
			this.div.style.fontSize = `${this.textSize}rem`;
			if( this.div.scrollWidth > this.div.offsetWidth ) {
				this.textSize *= 0.85;
			}
			else {
				break;
			}
		}
		this.fitTextDirty = false;
	}

	allowMultiline() {
		this.div.style.whiteSpace = "normal";
	}

	getOffsetX() { return this.x; }
	getOffsetY() { return this.y; }
	setOffset( x, y ) { this.x = x; this.y = y; this.updateTranslation(); }
	setOffsetX( x ) { this.x = x; this.updateTranslation(); }
	setOffsetY( y ) { this.y = y; this.updateTranslation(); }
	anchorCenterX( v?: boolean ) { this.shouldAnchorCenterX = v === undefined ? true : v; this.updateTranslation(); }
	anchorCenterY( v?: boolean ) { this.shouldAnchorCenterY = v == undefined ? true : v; this.updateTranslation() }
	getWidth() { return this.div.getBoundingClientRect().width; }
	getHeight() { return this.div.getBoundingClientRect().height; }
	getSize() { let rect = this.div.getBoundingClientRect(); return new Vec2(rect.width, rect.height); }
	getParentSize() { let rect = this.div.parentElement!.getBoundingClientRect(); return new Vec2(rect.width, rect.height); }

	private updatePositioning() {
		let margX = `${this.marginX}rem`;
		let margY = `${this.marginY}rem`;
		let auto = "auto";
		let cent = "50%";
		let top = this.ny > 0 ? margY : this.ny == 0 ? cent : auto;
		let right = this.nx > 0 ? margX : auto;
		let bot = this.ny < 0 ? margY : auto;
		let left = this.nx < 0 ? margX : this.nx == 0 ? cent : auto;
		this.div.style.inset = `${top} ${right} ${bot} ${left}`;

		this.updateTranslation();
	}

	private updateTranslation() {
		let transX = this.nx == 0 || this.shouldAnchorCenterX ? `calc(-50% + ${this.x * 0.1}rem)` : `${this.x * 0.1}rem`;
		let transY = this.ny == 0 || this.shouldAnchorCenterY ? `calc(-50% + ${this.y * 0.1}rem)` : `${this.y * 0.1}rem`;
		this.div.style.translate = `${transX} ${transY}`;
	}

	// applyTranslate() {
	// 	// this.div.style.translate = `calc(${pctX}% + ${x}px),calc(${anchorPctY}% + ${pctY}% + ${y}px)`;
	// }


	// // getParentRect() { return this.div.parentElement!.getBoundingClientRect(); }
	// // getParentPos() { let rect = this.getParentRect(); return new Vec2(rect.x, rect.y); }
	// // getParentCenter() { return this.getParentPos().add(this.getParentSize().mul(0.5)); }
	// // getRect() { return this.div.getBoundingClientRect(); }
	// // getPos() { let rect = this.div.getBoundingClientRect(); return new Vec2(rect.x, rect.y); }
	// setPos(...args) { let v = Vec2.smart(args); Util.setPos(this.div, v.x, v.y); }
	// setPosCenter(...args) { let v = Vec2.smart(args); Util.setPosCenter(this.div, v.x, v.y); }
	// translate(...args) { let v = Vec2.smart(args); Util.setPos
	// // setScale(...args) { let v = Vec2.smart(args); }
	// setVisible(visible: boolean) { Util.setVisible(this.div, visible); }
	// // floatLeft(margin: number) { let pos = this.getPos(); pos.x = margin; this.setPos(pos); }
	// // floatTop(margin: number) { let pos = this.getPos(); pos.y = margin; this.setPos(pos); }
	// // floatBottom(margin: number) { let pos = this.getPos(); pos.x = this.getParentSize().y - this.getSize().y - margin; this.setPos(pos); }
}