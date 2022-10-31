/*
Navicat MySQL Data Transfer

Source Server         : local
Source Server Version : 50173
Source Host           : localhost:3306
Source Database       : lp_account

Target Server Type    : MYSQL
Target Server Version : 50173
File Encoding         : 65001

Date: 2022-10-31 09:58:28
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `account`
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `platform_uid` varchar(64) NOT NULL DEFAULT '',
  `hash_index` int(11) DEFAULT NULL,
  `game_uid` bigint(20) DEFAULT NULL,
  `dbindex` int(11) DEFAULT NULL,
  PRIMARY KEY (`platform_uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account
-- ----------------------------
