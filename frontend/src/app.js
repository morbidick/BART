import { LitElement, html } from 'lit-element';
import * as bt from './bluetooth.js';

class MyApp extends LitElement {
	static get properties() {
		return {
			data: {
				type: Object,
			},
			battery: {
				type: Number,
			},
			error: {
				type: String,
			},
		}
	}

	constructor() {
		super();
		this.data = {};
		bt.onTemperature((data) => {
			this.data = data;
		})
		bt.onBattery((percentage) => {
			this.battery = percentage;
			console.log("got new battery state", percentage);
		})
	}

	render() {
		return html`
			${this.error ? html`<div class="error">${this.error}</div>` : ''}
			<div>Battery: ${this.battery}%</div>
			${this.data.connected ?
				html`<div>${this.data.temp0}°C ${this.data.temp1}°C</div>` :
				html`<div>Probe not connected</div>`
			}
			<button @click=${this.connect}>connect</button>
		`;
	}

	async connect() {
		try {
			this.clearError();
			await bt.connect();
		} catch({message}) {
			this.error = message;
		}
	}

	clearError() {
		this.error = false;
	}
}

customElements.define('my-app', MyApp);
