import ModulePromise from "./build/QB";
let QB_MODULE = await ModulePromise();
enum Change {
	kChange_Rank,
	kChange_CardScore,
	kChange_AllyScore,
	kChange_EnemyScore,
	kChange_AnyFactionScore,
	kChange_Kill,
	kChange_RemoveFromBoard,
	kChange_Play,
	kChange_PlayFromHand,
	kChange_AddCardToHand,
	kChange_Count,
};
QB_MODULE.Change = Change;
QB_MODULE.BoardToString = function(board) {
	let sb = new QB_MODULE.SerializedBoard();
	sb.ReadFrom(board);
	return sb.base64;
}
QB_MODULE.StringToBoard = function(str: string) {
	let sb = new QB_MODULE.SerializedBoard();
	let board = new QB_MODULE.Board();
	sb.Set(str);
	sb.WriteTo(board);
	return board;
}
QB_MODULE.CardByIdx = function(idx: number) {
	return QB_MODULE.CardDb.prototype.GetByIdx(idx);
}
// QB_MODULE.BoardToData = function(board) {
// 	let size = board.GetSerializeIntCount();

// 	let data = new Array(size);
// 	// for( let ii = 0; ii < size; ++ii ) {
// 	// 	let byte = board.GetByte(ii);
// 	// 	console.log(byte);
// 	// 	bytes[ii] = byte;
// 	// }
// 	board.Serialize(data, false);
// 	return data;
// }
// QB_MODULE.BoardToJSON = function(board) {
// 	return JSON.stringify(QB_MODULE.BoardToData(board));
// }
// QB_MODULE.DataToBoard = function(data) {
// 	let board = new QB_MODULE.Board();
// 	let size = board.GetSerializeIntCount();
// 	// for( let ii = 0; ii < size; ++ii ) { 
// 	// 	board.SetByte( ii, data[ii] );
// 	// }
// 	board.Serialize(data, true);
// 	return board;
// }
// QB_MODULE.JSONToBoard = function(json) {
// 	return QB_MODULE.DataToBoard(JSON.parse(json));
// }

export default QB_MODULE;