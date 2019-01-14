const serviceUuid = 0x181A;
const charUuid = 0x2A1F;

let onDataCallback;

function dataViewToValues({ buffer }) {
    let connected = new DataView(buffer, 0, 1).getInt8() === 0x01;
    let temp0 = null;
    let temp1 = null;

    if (connected) {
        temp0 = new DataView(buffer, 1, 2).getInt16() / 10;
        temp1 = new DataView(buffer, 3, 2).getInt16() / 10;
    }

    return {connected, temp0, temp1};
}

export async function connect() {
	console.log('Requesting Bluetooth Device...');
	const device = await navigator.bluetooth.requestDevice({
		filters: [{services: [serviceUuid]}],
	});

	console.log('Connecting to GATT Server...');
	const server = await device.gatt.connect();

	console.log('Getting Service...');
	const service = await server.getPrimaryService(serviceUuid);

	console.log('Getting Characteristic...');
	const characteristic = await service.getCharacteristic(charUuid);

	await characteristic.startNotifications();
	characteristic.addEventListener('characteristicvaluechanged',(e) => {
		if(typeof onDataCallback == 'function') {
			onDataCallback(dataViewToValues(e.target.value));
		}
	});

	return characteristic;
}

export function onData(callback) {
	onDataCallback = callback;
}

