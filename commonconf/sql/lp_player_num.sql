/*
Navicat MySQL Data Transfer

Source Server         : local
Source Server Version : 50173
Source Host           : localhost:3306
Source Database       : lp_player_num

Target Server Type    : MYSQL
Target Server Version : 50173
File Encoding         : 65001

Date: 2022-10-31 09:58:59
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `db_player_num`
-- ----------------------------
DROP TABLE IF EXISTS `db_player_num`;
CREATE TABLE `db_player_num` (
  `dbid` int(11) NOT NULL DEFAULT '0',
  `count` int(11) DEFAULT '0',
  PRIMARY KEY (`dbid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of db_player_num
-- ----------------------------
INSERT INTO `db_player_num` VALUES ('1', '2002');
