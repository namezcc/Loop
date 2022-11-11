/*
Navicat MySQL Data Transfer

Source Server         : local
Source Server Version : 50173
Source Host           : localhost:3306
Source Database       : lp_game

Target Server Type    : MYSQL
Target Server Version : 50173
File Encoding         : 65001

Date: 2022-10-31 09:58:46
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `lp_player`
-- ----------------------------
DROP TABLE IF EXISTS `lp_player`;
CREATE TABLE `lp_player` (
  `pid` bigint(20) NOT NULL DEFAULT '0',
  `uid` bigint(20) NOT NULL DEFAULT '0',
  `name` varchar(32) NOT NULL DEFAULT '',
  `level` int(11) NOT NULL DEFAULT '1',
  `gold` int(11) NOT NULL DEFAULT '0',
  UNIQUE KEY `pid` (`pid`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of lp_player
-- ----------------------------

-- ----------------------------
-- Table structure for `lp_player_relation`
-- ----------------------------
DROP TABLE IF EXISTS `lp_player_relation`;
CREATE TABLE `lp_player_relation` (
  `pid` bigint(20) NOT NULL,
  `rpid` bigint(20) NOT NULL,
  `name` varchar(20) NOT NULL DEFAULT '',
  `level` int(11) NOT NULL DEFAULT '0',
  `time` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  PRIMARY KEY (`pid`,`rpid`),
  KEY `pid` (`pid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of lp_player_relation
-- ----------------------------

-- ----------------------------
-- Procedure structure for `test_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `test_data`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `test_data`()
BEGIN
	#Routine body goes here...
DECLARE i INT;
SET i=1;
WHILE i < 100000 DO
	INSERT INTO lp_test(`f1`) VALUES(0);
SET i=i+1;
END WHILE;

END
;;
DELIMITER ;
