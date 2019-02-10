-- Dumping structure for procedure bees.StoreData
CREATE PROCEDURE `StoreData`(
	IN `Source` SMALLINT UNSIGNED,
	IN `LQI` TINYINT UNSIGNED,
	IN `RSSI` TINYINT UNSIGNED,
	IN `Battery` TINYINT UNSIGNED,
	IN `Temperature1` DOUBLE,
	IN `Humidity1` DOUBLE,
	IN `Temperature2` DOUBLE
)
BEGIN

INSERT INTO data_received (Source, LQI, RSSI, Battery, Temperature1, Humidity1, Temperature2) 
VALUES(Source, LQI, RSSI, Battery, Temperature1, Humidity1, Temperature2);
END