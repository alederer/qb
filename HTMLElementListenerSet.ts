export default class HTMLElementListenerSet {
	listeners = new Map<string, EventListener>();
	target: HTMLElement|null = null;
	update(target: HTMLElement|null, listeners: Object) {
		// remove all from old target
		if (this.target && this.target != target) {
			for (const kk in this.listeners) {
				this.target.removeEventListener(kk, this.listeners[kk]);
			}
			this.listeners.clear();
		}

		this.target = target;
		if (this.target) {
			// remove old/different listeners
			for (const kk in this.listeners) {
				if (listeners[kk] != this.listeners[kk]) {
					this.target.removeEventListener(kk, this.listeners[kk]);
					delete this.listeners[kk];
				}
			}

			// add new listeners
			for (const kk in listeners) {
				if (listeners[kk] != this.listeners[kk]) {
					this.target.addEventListener(kk, listeners[kk]);
					this.listeners[kk] = listeners[kk];
				}
			}
		}
	}
}