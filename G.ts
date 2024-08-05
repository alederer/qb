export default {
	BOARD_WIDTH : 5,
	BOARD_HEIGHT : 3,
	BOARD_CELLS : 15,
	MAX_RANK : 3,
	MAX_DECK_SIZE: 15,
	HAND_SIZE: 5,
	EFFECT_DISPLAY_GRID_WIDTH : 5,
	EFFECT_DISPLAY_GRID_HEIGHT : 5,
	EFFECT_DISPLAY_WIDTH : 70,
	EFFECT_DISPLAY_HEIGHT : 70,
	CELL_WIDTH : 150,
	CELL_HEIGHT : 150,
	CARD_WIDTH : 120,
	CARD_HEIGHT : 200,
	PAWN : "\u265F",
	// FACTION_PLAYER : Symbol("FACTION_PLAYER"),
	// FACTION_NEUTRAL : Symbol("FACTION_NEUTRAL"),
	// FACTION_ENEMY : Symbol("FACTION_ENEMY"),

	getCssColorBg(faction) {
		switch (faction) {
			case 0: return "#bef";
			case 1: return "#dcf";
		}
		return "black";
	},

	getCssColorFg(faction) {
		switch (faction) {
			case 0: return "#0060fc";
			case 1: return "#c000dc";
		}
		return "black";
	},

	getDeltaColor(sign) {
		if( sign > 0 ) {
			return "#0b0";
		}
		else if( sign < 0 ) {
			return "#f00";
		}
		else {
			return "#000";
		}
	}
};