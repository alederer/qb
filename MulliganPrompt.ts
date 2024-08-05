export default class MulliganPrompt {
	root: HTMLDivElement;
	buttons: HTMLDivElement;

	onMulligan;
	onMulliganPass;

	constructor(root: HTMLDivElement) {
		this.root = root;
		root.classList.add("mulliganPromptWrapper");
		
		let div  = document.createElement("div");
		div.classList.add("mulliganPrompt");
		div.innerText = "Mulligan phase";
		this.root.appendChild(div);
		this.buttons = document.createElement("div");
		this.buttons.classList.add("mulliganPromptButtons");

		let mulligan = document.createElement("button");
		mulligan.innerText = "Mulligan Selected Cards";
		mulligan.onclick = () => this.onMulliganCb();
		let pass = document.createElement("button");
		pass.innerText = "No Mulligan";
		pass.onclick = () => this.onMulliganPassCb();
		this.buttons.appendChild(mulligan);
		this.buttons.appendChild(pass);
		div.appendChild(this.buttons);
	}

	onMulliganCb() {
		this.onMulligan();
	}

	onMulliganPassCb() {
		this.onMulliganPass()
	}
}