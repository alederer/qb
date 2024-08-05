import G from "./G";

export default function() {
	// let balloc = new BoardAllocator();

	// let allocs = new Array<number[]>();
	// for (let ii = 0; ii < 10; ++ii) {
	// 	allocs.push(balloc.alloc());
	// }
	// for (let ii = 0; ii < 9; ++ii) {
	// 	for (let jj = ii + 1; jj < 10; ++jj) {
	// 		console.assert(allocs[ii][1] != allocs[jj][1]);
	// 	}
	// }

	// let [pidx, bidx] = balloc.alloc();

	// let deck = new Array(G.MAX_DECK_SIZE);
	// deck.fill(CARDS.TEST_IDX);
	// let decks = [deck, deck];

	// balloc.setDecksAndInitBoard(pidx, bidx, decks);

	// console.assert(balloc.getHandSize(pidx, bidx, 0) == G.HAND_SIZE);

	// let resolveQueue = []
	// balloc.playFromHand(resolveQueue, pidx, bidx, 0, 1, 0, 0);
	// console.assert(balloc.getHandSize(pidx, bidx, 0) == G.HAND_SIZE - 1);
	// balloc.resolve(resolveQueue, pidx, bidx, true);

	// balloc.playFromHand(resolveQueue, pidx, bidx, 4, 1, 1, 0);
	// balloc.resolve(resolveQueue, pidx, bidx, true);

	// balloc.playFromHand(resolveQueue, pidx, bidx, 1, 1, 0, 0);
	// balloc.resolve(resolveQueue, pidx, bidx, true);

	// balloc.playFromHand(resolveQueue, pidx, bidx, 3, 1, 1, 0);
	// balloc.resolve(resolveQueue, pidx, bidx, true);

	// console.assert(balloc.getRankScore(pidx, bidx, 0) == 6 * 2 + 4 * 2);
	// console.assert(balloc.getRankScore(pidx, bidx, 1) == 6 * 2 + 4 * 3);
}