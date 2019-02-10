-- Dumping structure for view bees.Last6Months
-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `Last6Months`;
CREATE VIEW `Last6Months` AS select avg(`data_received`.`Temperature1`) AS `AvgTemp1`,avg(`data_received`.`Humidity1`) AS `AvgHumidity1`,avg(`data_received`.`Temperature2`) AS `AvgTemp2`,((`data_received`.`ReceivedAt` + interval -(minute(`data_received`.`ReceivedAt`)) minute) + interval -(second(`data_received`.`ReceivedAt`)) second) AS `Time` from `data_received` where (`data_received`.`ReceivedAt` > (now() - interval 6 month)) group by month(`data_received`.`ReceivedAt`);
