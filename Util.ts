let exports = {
	setVisible: function(element: HTMLElement, visible: boolean) {
		if( visible )
		{
			element.style.removeProperty("display");
		}
		else
		{
			element.style.setProperty("display", "none");
		}		
	},

	setPos: function(element: HTMLElement, x: number, y: number) {
		element.style.left = x + "px";
		element.style.top = y + "px";
	},

	setPosCenter: function(element: HTMLElement, x: number, y: number) {
		let rect = element.getBoundingClientRect();
		element.style.left = (x - rect.width * 0.5) + "px";
		element.style.top = (y! - rect.height * 0.5) + "px";
	},

	// translate: function(element: HTMLElement, x: number, y: number) {

	// }
};

export default {...exports};