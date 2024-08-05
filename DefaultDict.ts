export default function DefaultDict<Value>(f: () => Value) {
	return new Proxy({}, {
		get(target, p, receiver) {
			if (!(p in target)) {
				target[p] = f();
			}
			return target[p];
		},
	});
};