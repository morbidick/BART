# BarT Feature list

BarT will be developed in KiCAD. Its hardware will be outlined in here.


- BarT will be battery optimized


Physical outline:

	- The PCB dimensions must lie within 30mm by 35mm
	- The PCB must be a 2 or 4 layer design with a feature size larger than 6/6mil 
	- The maximum height of the PCB including components must be below 8mm 

Power Supply:

	- The device is powered by a 350mAh Lithium Polymer battery inside the case
	- The device must not draw any quiescent power until activated
	- A DCDC converter shall be employed for low power operation
	- The device shall be chargeable via a USB micro interface


Temperature Measurement:

	- The device shall support two PT100/PT1000 sensors via 3.5mm audio jack
	- The temperature measurement accuracy shall be within a tolerance of 1K over a range from 20°C to 200°C
	- The temperature sensors shall be unpowered unless a measurement takes place
	- The default measurement interval shall be 30 seconds


User Interface:

	- Pushbutton for activation
	- One RGB multifunction LED and one red Alarm / charging LED
	- Once the pushbutton is clicked, BarT will stay active for 60 seconds and then turn off, unless a Bluetooth connection has been established
	- Once active, the multicolor LED will display the remaining charge for 5 seconds:
		- GREEN: 100% - 60%
		- YELLOW: 60% - 30%
		- RED: 30% - 10%
		- RED FLASHING: < 10%

