-- MySQL dump 10.11
--
-- Host: localhost    Database: e_mush
-- ------------------------------------------------------
-- Server version	5.0.51a-24-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `honor_log`
--

DROP TABLE IF EXISTS `honor_log`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `honor_log` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `victim` mediumint(9) NOT NULL,
  `delta` int(11) NOT NULL,
  `balance` int(11) NOT NULL,
  `description` text NOT NULL,
  `parent_honor_log_id` bigint(20) default NULL,
  `dt` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `victim` (`victim`),
  KEY `parent_honor_log_id` (`parent_honor_log_id`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `honor_log`
--

LOCK TABLES `honor_log` WRITE;
/*!40000 ALTER TABLE `honor_log` DISABLE KEYS */;
INSERT INTO `honor_log` VALUES (1,3,45,45,'pooped a3948tr078sdfaoiu2390rtudflnavgni8509840283923ur8wudfisjdf8j239r5th82',NULL,'2009-06-10 16:43:32'),(2,34,35,35,'gained honor for being osum!',NULL,'2009-06-15 15:43:52'),(3,34,-35,0,'farted in public',NULL,'2009-06-15 15:43:52');
/*!40000 ALTER TABLE `honor_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'e_mush'
--
DELIMITER ;;
DELIMITER ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-07-13 18:33:00
