-- Dumping structure for table bees.data_received
CREATE TABLE IF NOT EXISTS `data_received` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ReceivedAt` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `Source` smallint(5) unsigned DEFAULT NULL,
  `LQI` tinyint(3) unsigned DEFAULT NULL,
  `RSSI` tinyint(3) unsigned DEFAULT NULL,
  `Battery` tinyint(3) unsigned DEFAULT NULL,
  `Temperature1` double DEFAULT NULL,
  `Humidity1` double DEFAULT NULL,
  `Temperature2` double DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=27836 DEFAULT CHARSET=utf8mb4;
