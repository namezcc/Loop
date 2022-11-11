/*
Navicat MySQL Data Transfer

Source Server         : local
Source Server Version : 50173
Source Host           : localhost:3306
Source Database       : master

Target Server Type    : MYSQL
Target Server Version : 50173
File Encoding         : 65001

Date: 2022-10-31 09:52:15
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `admin`
-- ----------------------------
DROP TABLE IF EXISTS `admin`;
CREATE TABLE `admin` (
  `user` varchar(64) NOT NULL,
  `pass` varchar(64) NOT NULL,
  `level` int(11) NOT NULL,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of admin
-- ----------------------------

-- ----------------------------
-- Table structure for `console`
-- ----------------------------
DROP TABLE IF EXISTS `console`;
CREATE TABLE `console` (
  `id` int(11) NOT NULL,
  `ip` varchar(64) NOT NULL COMMENT 'server ip',
  `port` int(11) NOT NULL COMMENT 'server port',
  `name` varchar(64) NOT NULL COMMENT 'server name',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of console
-- ----------------------------

-- ----------------------------
-- Table structure for `machine`
-- ----------------------------
DROP TABLE IF EXISTS `machine`;
CREATE TABLE `machine` (
  `id` int(11) NOT NULL,
  `ip` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of machine
-- ----------------------------
INSERT INTO `machine` VALUES ('1', '10.41.2.57');
INSERT INTO `machine` VALUES ('2', '192.168.154.143');

-- ----------------------------
-- Table structure for `server`
-- ----------------------------
DROP TABLE IF EXISTS `server`;
CREATE TABLE `server` (
  `type` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `machine` int(11) NOT NULL,
  `port` int(11) NOT NULL,
  `mysql` varchar(128) NOT NULL DEFAULT '',
  `redis` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`type`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of server
-- ----------------------------
INSERT INTO `server` VALUES ('2', '1', '1', '22001', '', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('2', '2', '2', '22002', '', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('2', '3', '1', '22003', '', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('3', '1', '1', '23001', '10.41.2.57|3306|lp_game|root|123456', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('3', '2', '2', '23002', '10.41.2.57|3306|lp_game|root|123456', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('7', '1', '1', '27001', '', '');
INSERT INTO `server` VALUES ('8', '1', '1', '28001', '', '');
INSERT INTO `server` VALUES ('8', '2', '1', '28002', '', '');
INSERT INTO `server` VALUES ('8', '3', '2', '28003', '', '');
INSERT INTO `server` VALUES ('8', '4', '2', '28004', '', '');
INSERT INTO `server` VALUES ('9', '1', '1', '29001', '', '10.41.2.57|6379|');
INSERT INTO `server` VALUES ('10', '1', '1', '30001', '10.41.2.57|3306|lp_player_num|root|123456', '');
INSERT INTO `server` VALUES ('15', '1', '1', '35001', '', '');
INSERT INTO `server` VALUES ('15', '2', '2', '35002', '', '');
INSERT INTO `server` VALUES ('17', '1', '1', '37001', '10.41.2.57|3306|lp_account|root|123456', '');
INSERT INTO `server` VALUES ('17', '2', '2', '37002', '10.41.2.57|3306|lp_account|root|123456', '');
INSERT INTO `server` VALUES ('18', '1', '1', '38001', '', '');
INSERT INTO `server` VALUES ('18', '2', '2', '38002', '', '');
