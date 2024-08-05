export default function DefaultArray<Value>(f: () => Value): Array<Value> {
	return new Proxy(new Array<Value>(), {
		get(target, p, receiver) {
			// @ts-ignore
			let pInt = parseInt(p);
			while( pInt >= target.length ) {
				target.push(f());
			}
			return target[pInt];
		},
	});
};
