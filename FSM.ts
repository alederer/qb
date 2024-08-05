import DefaultArray from "./DefaultArray";
import DefaultDict from "./DefaultDict";

enum Phase {
	Enter,
	Update,
	Exit
};

class StateEntry<State> {
	enter: (() => State|void)|undefined;
	update: ((dt: number) => State|void)|undefined;
	exit: ((next: State) => State|void)|undefined;
}

export default class FSM<State> {
	// @ts-ignore
	private state: State|undefined;
	private hasEntered = false;
	dt = 0.0;
	time = 0.0;
	states = DefaultDict<StateEntry<State>>(() => new StateEntry<State>());
	// states = DefaultArray(() => new StateEntry<State>());

	getState(): State|undefined {
		return this.state;
	}

	setState(state: State, forceReenter?: boolean) {
		if( state == this.state && this.hasEntered && !forceReenter ) { return; }

		let notInAnyState = !this.hasEntered;
		this.hasEntered = false;
		let next = state;

		while( !this.hasEntered ) { 
			if( !notInAnyState ) {
				console.log( `ExitState  ${this.state}` );
				let exitNext = this.stateExit( next );
				if( exitNext !== undefined ) {
					next = exitNext;
				}
			}

			notInAnyState = false;
			this.time = 0.0;
			this.state = next;

			console.log( `EnterState  ${next}` );
			let enterNext = this.stateEnter();
			if( enterNext !== undefined ) {
				next = enterNext;
			}
			else {
				this.hasEntered = true;
			}
		}
	}
	
	update(dt: number) {
		this.dt = dt;

		// transition if need be
		if( this.state !== undefined ) {
			this.setState(this.state, false);
		}

		let updateNext = this.stateUpdate(dt);
		if( updateNext !== undefined ) {
			this.setState(updateNext, true);
		}

		this.time += dt;
	}

	private stateEnter() {
		// @ts-ignore
		let f = this.states[this.state!].enter;
		return f ? f() : undefined;
	}

	private stateUpdate(dt: number) {
		// @ts-ignore
		let f = this.states[this.state!].update;
		return f ? f(dt) : undefined;
	}

	private stateExit(next: State) {
		// @ts-ignore
		let f = this.states[this.state!].exit;
		return f ? f(next) : undefined;
	}
}

