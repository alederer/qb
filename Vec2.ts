export default class Vec2 {
	x = 0;
	y = 0;

	constructor(x?: number, y?: number) {
		if(x !== undefined) { this.x = x; }
		if(y !== undefined) { this.y = y; }
		else { this.y = this.x; }
	}

	set(x: number, y: number) { this.x = x; this.y = y; }
	setX(x: number) { this.x = x; }
	setY(y: number) { this.y = y; }

	static smart(args) {
		if( args[0] instanceof Vec2 ) { return args[0]; }
		if( typeof args[0] == "number" ) { return new Vec2(args[0], args[1] ?? args[0]); }
		return new Vec2(args[0][0], args[0][1]);
	}

	static X(x?: number) { return new Vec2( x ?? 0.0, 0.0 ); }
	static Y(y?: number) { return new Vec2( 0.0, y ?? 0.0 ); }
	c() { return new Vec2(this.x, this.y);  }
	copy() { return new Vec2(this.x, this.y); }
	neg() { return new Vec2(-this.x, -this.y); }

	add(...args) {
		let o = Vec2.smart(args);
		return new Vec2(this.x + o.x, this.y + o.y);
	}
	sub(...args) {
		let o = Vec2.smart(args);
		return new Vec2(this.x - o.x, this.y - o.y);
	}
	mul(...args) {
		let o = Vec2.smart(args);
		return new Vec2(this.x * o.x, this.y * o.y);
	}
	div(...args) {
		let o = Vec2.smart(args);
		return new Vec2(this.x / o.x, this.y / o.y);
	}
	xOnly() {
		return new Vec2(this.x, 0.0);
	}
	yOnly() {
		return new Vec2(0.0, this.y);
	}
}