-- Dumping structure for view bees.Logs
-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `Logs`;
CREATE VIEW `Logs` AS select `dr`.`ID` AS `ID`,`dr`.`ReceivedAt` AS `ReceivedAt`,`h`.`Name` AS `Name`,`dr`.`Temperature1` AS `Temperature1`,`dr`.`Humidity1` AS `Humidity1`,`dr`.`Temperature2` AS `Temperature2` from (`data_received` `dr` join `hives` `h` on((`h`.`ID` = `dr`.`Source`)));
