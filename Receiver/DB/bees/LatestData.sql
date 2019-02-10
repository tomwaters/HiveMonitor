-- Dumping structure for view bees.LatestData
-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `LatestData`;
CREATE VIEW `LatestData` AS select `d`.`ID` AS `ID`,`d`.`ReceivedAt` AS `ReceivedAt`,`h`.`Name` AS `ReceivedFrom`,`d`.`Temperature1` AS `Temperature1`,`d`.`Humidity1` AS `Humidity1`,`d`.`Temperature2` AS `Temperature2`,`d`.`Battery` AS `Battery` from (`data_received` `d` join `hives` `h` on((`h`.`ID` = `d`.`Source`))) order by `d`.`ReceivedAt` desc limit 1;
