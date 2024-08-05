
let PROFILES = new Map<string, number>();
let NOW_COUNT = 0;
let APPROX_NOW_COST = 0;
let ENABLED = true;

if (APPROX_NOW_COST == 0.0) {
	init();
}

export function init() {
	const ITERS = 10000;
	let start = performance.now();
	for (let ii = 0; ii < ITERS; ++ii) {
		performance.now();
	}
	APPROX_NOW_COST = (performance.now() - start) / ITERS;
}

export function setEnabled(enabled) {
	ENABLED = enabled;
}

export function startCapture() {
	if (!ENABLED) { return; }
	NOW_COUNT++;
	return performance.now();
}

export function endCapture(start, name) {
	if (!ENABLED) { return; }
	NOW_COUNT++;
	let elapsed = performance.now() - start;
	if (PROFILES.has(name)) {
		PROFILES.set(name, PROFILES.get(name)! + elapsed);
	}
	else {
		PROFILES.set(name, elapsed);
	}
}

export function tallyPerfNowOverhead(name) {
	PROFILES.set(name, NOW_COUNT * APPROX_NOW_COST);
}

export function tallyRemainder(totalName, remainderName) {
	let sum = 0.0;
	let total = 0.0;
	for (let [kk, vv] of PROFILES.entries()) {
		if (kk == totalName) {
			total = vv;
		}
		else {
			sum += vv;
		}
	}
	PROFILES.set(remainderName, total - sum);
}

export function logAndReset() {
	console.log(new Map(PROFILES.entries()));
	PROFILES.clear();
	NOW_COUNT
}