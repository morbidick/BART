import { LitElement, html } from 'lit-element';
import * as bt from './bluetooth.js';

class MyApp extends LitElement {
	static get properties() {
		return {
			data: {
				type: Object,
			}
		}
	}

	constructor() {
		super();
		this.data = {};
		bt.onData((data) => {
			this.data = data;
		})
	}

	render() {
		return html`
			<div>${this.data.temp0} ${this.data.temp1}</div>
			<button @click=${this.demo}>connect</button>
		`;
	}

	async demo() {
		try {
			await bt.connect();
		} catch(error) {
			console.log('Argh! ' + error);
		}
	}
}

customElements.define('my-app', MyApp);
