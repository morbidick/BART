const tempServiceUuid = 0x181A;
const tempCharUuid = 0x2A1F;

const battServiceUuid = 0x180F;
const battCharUuid = 0x2A19;

let onTempCallback;
let onBattCallback;

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
	if (!navigator.bluetooth)
		throw new Error(`Browser doesn't support web Bluetooth, try the latest Chrome build`);

	console.log('Requesting Bluetooth Device...');
	const device = await navigator.bluetooth.requestDevice({
		filters: [{services: [tempServiceUuid, battServiceUuid]}],
	});

	console.log('Connecting to GATT Server...');
	const server = await device.gatt.connect();

	console.log('Getting Temperature Service...');
	const service = await server.getPrimaryService(tempServiceUuid);

	console.log('Getting Temperature Characteristic...');
	const characteristic = await service.getCharacteristic(tempCharUuid);

	console.log('Subscribe Temperature Characteristic...');
	await characteristic.startNotifications();
	characteristic.addEventListener('characteristicvaluechanged',(e) => {
		if(typeof onTempCallback == 'function') {
			onTempCallback(dataViewToValues(e.target.value));
		}
	});

	console.log('Getting Battery Service...');
	const battService = await server.getPrimaryService(battServiceUuid);

	console.log('Getting Battery Characteristic...');
	const battCharacteristic = await battService.getCharacteristic(battCharUuid);

	console.log('Subscribe Battery Characteristic...');
	await battCharacteristic.startNotifications();
	battCharacteristic.addEventListener('characteristicvaluechanged',(e) => {
		if(typeof onBattCallback == 'function') {
			onBattCallback(e.target.value);
		}
	});
}

export function onTemperature(callback) {
	onTempCallback = callback;
}

export function onBattery(callback) {
	onBattCallback = callback;
}
